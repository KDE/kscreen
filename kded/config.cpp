/********************************************************************
Copyright 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
Copyright 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "config.h"
#include "kscreen_daemon_debug.h"
#include "generator.h"
#include "device.h"

#include <QStringList>
#include <QCryptographicHash>
#include <QFile>
#include <QStandardPaths>
#include <QRect>
#include <QStringBuilder>
#include <QJsonDocument>
#include <QDir>
#include <QLoggingCategory>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>

QString Config::s_fixedConfigFileName = QStringLiteral("fixed-config");
QString Config::s_dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % QStringLiteral("/kscreen/");

Config::Config(KScreen::ConfigPtr config)
    : m_data(config)
{
}

void Config::setDirPath(const QString &path)
{
    s_dirPath = path;
    if (!s_dirPath.endsWith(QLatin1Char('/'))) {
        s_dirPath += QLatin1Char('/');
    }
}

QString Config::filePath()
{
    if (!QDir().mkpath(s_dirPath)) {
        return QString();
    }
    return s_dirPath % id();
}

QString Config::id() const
{
    if (!m_data) {
        return QString();
    }
    return m_data->connectedOutputsHash();
}

bool Config::fileExists() const
{
    return (QFile::exists(s_dirPath % id()) || QFile::exists(s_dirPath % s_fixedConfigFileName));
}

std::unique_ptr<Config> Config::readFile()
{
    if (Device::self()->isLaptop() && !Device::self()->isLidClosed()) {
        // We may look for a config that has been set when the lid was closed, Bug: 353029
        const QString lidOpenedFilePath(filePath() % QStringLiteral("_lidOpened"));
        const QFile srcFile(lidOpenedFilePath);

        if (srcFile.exists()) {
            QFile::remove(filePath());
            if (QFile::copy(lidOpenedFilePath, filePath())) {
                QFile::remove(lidOpenedFilePath);
                qCDebug(KSCREEN_KDED) << "Restored lid opened config to" << id();
            }
        }
    }
    return readFile(id());
}

std::unique_ptr<Config> Config::readOpenLidFile()
{
    const QString openLidFilePath = filePath() % QStringLiteral("_lidOpened");
    auto config = readFile(openLidFilePath);
    QFile::remove(openLidFilePath);
    return config;
}

std::unique_ptr<Config> Config::readFile(const QString &fileName)
{
    if (!m_data) {
        return nullptr;
    }
    KScreen::ConfigPtr config = m_data->clone();

    QFile file;
    if (QFile::exists(s_dirPath % s_fixedConfigFileName)) {
        file.setFileName(s_dirPath % s_fixedConfigFileName);
        qCDebug(KSCREEN_KDED) << "found a fixed config, will use " << file.fileName();
    } else {
        file.setFileName(s_dirPath % fileName);
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(KSCREEN_KDED) << "failed to open file" << file.fileName();
        return nullptr;
    }

    KScreen::OutputList outputList = config->outputs();
    QJsonDocument parser;
    QVariantList outputs = parser.fromJson(file.readAll()).toVariant().toList();
    Q_FOREACH(KScreen::OutputPtr output, outputList) {
        if (!output->isConnected() && output->isEnabled()) {
            output->setEnabled(false);
        }
    }

    QSize screenSize;
    Q_FOREACH(const QVariant &info, outputs) {
        KScreen::OutputPtr output = findOutput(config, info.toMap());
        if (!output) {
            continue;
        }

        if (output->isEnabled()) {
            const QRect geom = output->geometry();
            if (geom.x() + geom.width() > screenSize.width()) {
                screenSize.setWidth(geom.x() + geom.width());
            }
            if (geom.y() + geom.height() > screenSize.height()) {
                screenSize.setHeight(geom.y() + geom.height());
            }
        }

        outputList.remove(output->id());
        outputList.insert(output->id(), output);
    }
    config->setOutputs(outputList);
    config->screen()->setCurrentSize(screenSize);

    if (!canBeApplied(config)) {
        return nullptr;
    }
    auto cfg = std::unique_ptr<Config>(new Config(config));
    cfg->setValidityFlags(m_validityFlags);
    return cfg;
}

bool Config::canBeApplied() const
{
    return canBeApplied(m_data);
}

bool Config::canBeApplied(KScreen::ConfigPtr config) const
{
#ifdef KDED_UNIT_TEST
    Q_UNUSED(config);
    return true;
#else
    return KScreen::Config::canBeApplied(config, m_validityFlags);
#endif
}

bool Config::writeFile()
{
    return writeFile(filePath());
}

bool Config::writeOpenLidFile()
{
    return writeFile(filePath() % QStringLiteral("_lidOpened"));
}

static QVariantMap metadata(const KScreen::OutputPtr &output)
{
    QVariantMap metadata;
    metadata[QStringLiteral("name")] = output->name();
    if (!output->edid() || !output->edid()->isValid()) {
        return metadata;
    }

    metadata[QStringLiteral("fullname")] = output->edid()->deviceId();
    return metadata;
}

bool Config::writeFile(const QString &filePath)
{
    if (!m_data) {
        return false;
    }
    const KScreen::OutputList outputs = m_data->outputs();

    QVariantList outputList;
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        QVariantMap info;

        info[QStringLiteral("id")] = output->hash();
        info[QStringLiteral("primary")] = output->isPrimary();
        info[QStringLiteral("enabled")] = output->isEnabled();
        info[QStringLiteral("rotation")] = output->rotation();
        info[QStringLiteral("scale")] = output->scale();

        QVariantMap pos;
        pos[QStringLiteral("x")] = output->pos().x();
        pos[QStringLiteral("y")] = output->pos().y();
        info[QStringLiteral("pos")] = pos;

        if (output->isEnabled()) {
            const KScreen::ModePtr mode = output->currentMode();
            if (!mode) {
                qWarning() << "CurrentMode is null" << output->name();
                return false;
            }

            QVariantMap modeInfo;
            modeInfo[QStringLiteral("refresh")] = mode->refreshRate();

            QVariantMap modeSize;
            modeSize[QStringLiteral("width")] = mode->size().width();
            modeSize[QStringLiteral("height")] = mode->size().height();
            modeInfo[QStringLiteral("size")] = modeSize;

            info[QStringLiteral("mode")] = modeInfo;
        }

        info[QStringLiteral("metadata")] = metadata(output);

        outputList.append(info);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(KSCREEN_KDED) << "Failed to open config file for writing! " << file.errorString();
        return false;
    }
    file.write(QJsonDocument::fromVariant(outputList).toJson());
    qCDebug(KSCREEN_KDED) << "Config saved on: " << file.fileName();

    return true;
}

KScreen::OutputPtr Config::findOutput(const KScreen::ConfigPtr &config, const QVariantMap& info)
{
    const KScreen::OutputList outputs = config->outputs();    // As individual outputs are indexed by a hash of their edid, which is not unique,
    // to be able to tell apart multiple identical outputs, these need special treatment
    QStringList duplicateIds;
    QStringList allIds;
    allIds.reserve(outputs.count());
    Q_FOREACH (const KScreen::OutputPtr &output, outputs) {
        const auto outputId = output->hash();
        if (allIds.contains(outputId) && !duplicateIds.contains(outputId)) {
            duplicateIds << outputId;
        }
        allIds << outputId;
    }
    allIds.clear();

    Q_FOREACH(KScreen::OutputPtr output, outputs) {
        if (!output->isConnected()) {
            continue;
        }
        const auto outputId = output->hash();
        if (outputId != info[QStringLiteral("id")].toString()) {
            continue;
        }

        // We may have identical outputs connected, these will have the same id in the config
        // in order to find the right one, also check the output's name (usually the connector)
        if (!output->name().isEmpty() && duplicateIds.contains(outputId)) {
            const auto metadata = info[QStringLiteral("metadata")].toMap();
            const auto outputName = metadata[QStringLiteral("name")].toString();
            if (output->name() != outputName) {
                continue;
            }
        }

        const QVariantMap posInfo = info[QStringLiteral("pos")].toMap();
        QPoint point(posInfo[QStringLiteral("x")].toInt(), posInfo[QStringLiteral("y")].toInt());
        output->setPos(point);
        output->setPrimary(info[QStringLiteral("primary")].toBool());
        output->setEnabled(info[QStringLiteral("enabled")].toBool());
        output->setRotation(static_cast<KScreen::Output::Rotation>(info[QStringLiteral("rotation")].toInt()));
        output->setScale(info.value(QStringLiteral("scale"), 1).toInt());

        const QVariantMap modeInfo = info[QStringLiteral("mode")].toMap();
        const QVariantMap modeSize = modeInfo[QStringLiteral("size")].toMap();
        const QSize size = QSize(modeSize[QStringLiteral("width")].toInt(), modeSize[QStringLiteral("height")].toInt());

        qCDebug(KSCREEN_KDED) << "Finding a mode for" << size << "@" << modeInfo[QStringLiteral("refresh")].toFloat();

        KScreen::ModeList modes = output->modes();
        KScreen::ModePtr matchingMode;
        Q_FOREACH(const KScreen::ModePtr &mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (!qFuzzyCompare(mode->refreshRate(), modeInfo[QStringLiteral("refresh")].toFloat())) {
                continue;
            }

            qCDebug(KSCREEN_KDED) << "\tFound: " << mode->id() << " " << mode->size() << "@" << mode->refreshRate();
            matchingMode = mode;
            break;
        }

        if (!matchingMode) {
            qCWarning(KSCREEN_KDED) << "\tFailed to find a matching mode - this means that our config is corrupted"
                                       "or a different device with the same serial number has been connected (very unlikely)."
                                       "Falling back to preferred modes.";
            matchingMode = output->preferredMode();

            if (!matchingMode) {
                qCWarning(KSCREEN_KDED) << "\tFailed to get a preferred mode, falling back to biggest mode.";
                matchingMode = Generator::biggestMode(modes);

                if (!matchingMode) {
                    qCWarning(KSCREEN_KDED) << "\tFailed to get biggest mode. Which means there are no modes. Turning off the screen.";
                    output->setEnabled(false);
                    return output;
                }
            }
        }

        output->setCurrentModeId(matchingMode->id());
        return output;
    }

    qCWarning(KSCREEN_KDED) << "\tFailed to find a matching output in the current config - this means that our config is corrupted"
                               "or a different device with the same serial number has been connected (very unlikely).";
    return KScreen::OutputPtr();
}

void Config::log()
{
    if (!m_data) {
        return;
    }
    const auto outputs = m_data->outputs();
    for (const auto o : outputs) {
        if (o->isConnected()) {
            qCDebug(KSCREEN_KDED) << o;
        }
    }
}
