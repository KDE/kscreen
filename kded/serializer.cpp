/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "serializer.h"
#include "kscreen_daemon_debug.h"
#include "generator.h"

#include <QStringList>
#include <QCryptographicHash>
#include <QFile>
#include <QStandardPaths>
#include <QStandardPaths>
#include <QRect>
#include <QStringBuilder>
#include <QJsonDocument>
#include <QDir>
#include <QLoggingCategory>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>

QString Serializer::sConfigPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % QStringLiteral("/kscreen/");

void Serializer::setConfigPath(const QString &path)
{
    sConfigPath = path;
    if (!sConfigPath.endsWith(QLatin1Char('/'))) {
        sConfigPath += QLatin1Char('/');
    }
}

QString Serializer::configFileName(const QString &configId)
{
    if (!QDir().mkpath(sConfigPath)) {
        return QString();
    }
    return sConfigPath % configId;
}

QString Serializer::configId(const KScreen::ConfigPtr &currentConfig)
{
    if (!currentConfig) {
        return QString();
    }
    KScreen::OutputList outputs = currentConfig->outputs();

    QStringList hashList;
    //qCDebug(KSCREEN_KDED) << "Calculating config ID for" << currentConfig.data();
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        //qCDebug(KSCREEN_KDED) << "\tPart of the Id: " << Serializer::outputId(output);
        hashList.insert(0, Serializer::outputId(output));
    }

    qSort(hashList.begin(), hashList.end());

    const QByteArray hash = QCryptographicHash::hash(hashList.join(QString()).toLatin1(),
                                                     QCryptographicHash::Md5).toHex();
    //qCDebug(KSCREEN_KDED) << "\tConfig ID:" << hash;
    return hash;
}

bool Serializer::configExists(const KScreen::ConfigPtr &config)
{
    return Serializer::configExists(Serializer::configId(config));
}

bool Serializer::configExists(const QString &id)
{
    return QFile::exists(sConfigPath % id);
}

KScreen::ConfigPtr Serializer::config(const KScreen::ConfigPtr &currentConfig, const QString &id)
{
    KScreen::ConfigPtr config = currentConfig->clone();

    QFile file(configFileName(id));
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(KSCREEN_KDED) << "failed to open file" << id;
        return KScreen::ConfigPtr();
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
        KScreen::OutputPtr output = Serializer::findOutput(config, info.toMap());
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


    return config;
}

bool Serializer::saveConfig(const KScreen::ConfigPtr &config, const QString &configId)
{
    if (!config || configId.isEmpty()) {
        return false;
    }
    const KScreen::OutputList outputs = config->outputs();

    QVariantList outputList;
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        QVariantMap info;

        info[QStringLiteral("id")] = Serializer::outputId(output);
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

        info[QStringLiteral("metadata")] = Serializer::metadata(output);

        outputList.append(info);
    }

    QFile file(configFileName(configId));
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(KSCREEN_KDED) << "Failed to open config file for writing! " << file.errorString();
        return false;
    }

    file.write(QJsonDocument::fromVariant(outputList).toJson());
    qCDebug(KSCREEN_KDED) << "Config saved on: " << file.fileName();

    return true;
}

void Serializer::removeConfig(const QString &id)
{
    QFile::remove(configFileName(id));
}

bool Serializer::moveConfig(const QString &srcId, const QString &destId)
{
    const QFile srcFile(configFileName(srcId));
    if (srcFile.exists()) {
        removeConfig(destId);
        if (QFile::copy(configFileName(srcId), configFileName(destId))) {
            removeConfig(srcId);
            qCDebug(KSCREEN_KDED) << "Restored config" << srcId << "to" << destId;
            return true;
        }
    }
    return false;
}

KScreen::OutputPtr Serializer::findOutput(const KScreen::ConfigPtr &config, const QVariantMap& info)
{
    const KScreen::OutputList outputs = config->outputs();    // As individual outputs are indexed by a hash of their edid, which is not unique,
    // to be able to tell apart multiple identical outputs, these need special treatment
    QStringList duplicateIds;
    QStringList allIds;
    allIds.reserve(outputs.count());
    Q_FOREACH (const KScreen::OutputPtr &output, outputs) {
        const auto outputId = Serializer::outputId(output);
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
        const auto outputId = Serializer::outputId(output);
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

QString Serializer::outputId(const KScreen::OutputPtr &output)
{
    if (output->edid() && output->edid()->isValid()) {
        return output->edid()->hash();
    }

    return output->name();
}

QVariantMap Serializer::metadata(const KScreen::OutputPtr &output)
{
    QVariantMap metadata;
    metadata[QStringLiteral("name")] = output->name();
    if (!output->edid() || !output->edid()->isValid()) {
        return metadata;
    }

    metadata[QStringLiteral("fullname")] = output->edid()->deviceId();
    return metadata;
}
