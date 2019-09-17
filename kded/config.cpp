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
#include "output.h"
#include "../common/globals.h"
#include "../common/control.h"
#include "kscreen_daemon_debug.h"
#include "device.h"

#include <QFile>
#include <QStandardPaths>
#include <QRect>
#include <QJsonDocument>
#include <QDir>

#include <kscreen/config.h>
#include <kscreen/output.h>

QString Config::s_fixedConfigFileName = QStringLiteral("fixed-config");
QString Config::s_configsDirName = QStringLiteral("" /*"configs/"*/); // TODO: KDE6 - move these files into the subfolder

QString Config::configsDirPath()
{
    return Globals::dirPath() % s_configsDirName;
}

Config::Config(KScreen::ConfigPtr config)
    : m_data(config)
{
}

QString Config::filePath()
{
    if (!QDir().mkpath(configsDirPath())) {
        return QString();
    }
    return configsDirPath() % id();
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
    return (QFile::exists(configsDirPath() % id()) || QFile::exists(configsDirPath() % s_fixedConfigFileName));
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
    auto config = std::unique_ptr<Config>(new Config(m_data->clone()));
    config->setValidityFlags(m_validityFlags);

    QFile file;
    if (QFile::exists(configsDirPath() % s_fixedConfigFileName)) {
        file.setFileName(configsDirPath() % s_fixedConfigFileName);
        qCDebug(KSCREEN_KDED) << "found a fixed config, will use " << file.fileName();
    } else {
        file.setFileName(configsDirPath() % fileName);
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(KSCREEN_KDED) << "failed to open file" << file.fileName();
        return nullptr;
    }

    QJsonDocument parser;
    QVariantList outputs = parser.fromJson(file.readAll()).toVariant().toList();
    Output::readInOutputs(config->data(), outputs);

    QSize screenSize;
    for (const auto &output : config->data()->outputs()) {
        if (!output->isPositionable()) {
            continue;
        }

        const QRect geom = output->geometry();
        if (geom.x() + geom.width() > screenSize.width()) {
            screenSize.setWidth(geom.x() + geom.width());
        }
        if (geom.y() + geom.height() > screenSize.height()) {
            screenSize.setHeight(geom.y() + geom.height());
        }
    }
    config->data()->screen()->setCurrentSize(screenSize);

    if (!canBeApplied(config->data())) {
        return nullptr;
    }
    return config;
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

bool Config::writeFile(const QString &filePath)
{
    if (id().isEmpty()) {
        return false;
    }
    const KScreen::OutputList outputs = m_data->outputs();

    const auto control = ControlConfig(m_data);

    const auto oldConfig = readFile();
    KScreen::OutputList oldOutputs;
    if (oldConfig) {
        oldOutputs = oldConfig->data()->outputs();
    }

    QVariantList outputList;
    for (const KScreen::OutputPtr &output : outputs) {
        QVariantMap info;

        const auto oldOutputIt = std::find_if(oldOutputs.constBegin(), oldOutputs.constEnd(),
                                              [output](const KScreen::OutputPtr &out) {
                                                  return out->hashMd5() == output->hashMd5();
                                               }
        );
        const KScreen::OutputPtr oldOutput = oldOutputIt != oldOutputs.constEnd() ? *oldOutputIt :
                                                                                    nullptr;

        if (!output->isConnected()) {
            continue;
        }

        Output::writeGlobalPart(output, info, oldOutput);
        info[QStringLiteral("primary")] = output->isPrimary();
        info[QStringLiteral("enabled")] = output->isEnabled();

        auto setOutputConfigInfo = [this, &info](const KScreen::OutputPtr &out) {
            if (!out) {
                return;
            }
            QString replicationSourceHash;
            if (int sourceId = out->replicationSource()) {
                replicationSourceHash = m_data->output(sourceId)->hashMd5();
            }
            info[QStringLiteral("replicate")] = replicationSourceHash;

            QVariantMap pos;
            pos[QStringLiteral("x")] = out->pos().x();
            pos[QStringLiteral("y")] = out->pos().y();
            info[QStringLiteral("pos")] = pos;
        };
        setOutputConfigInfo(output->isEnabled() ? output : oldOutput);

        if (output->isEnabled() &&
                control.getOutputRetention(output->hash(), output->name()) !=
                    Control::OutputRetention::Individual) {
            // try to update global output data
            Output::writeGlobal(output);
        }

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
