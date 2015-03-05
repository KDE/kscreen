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
#include "debug.h"
#include "generator.h"

#include <QtCore/QStringList>
#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>
#include <QtCore/QStandardPaths>
#include <QJsonDocument>
#include <QDir>
#include <QLoggingCategory>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>

QString Serializer::configId(const KScreen::ConfigPtr &currentConfig)
{
    KScreen::OutputList outputs = currentConfig->outputs();

    QStringList hashList;
    qCDebug(KSCREEN_KDED) << "Calculating config ID for" << currentConfig.data();
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        qCDebug(KSCREEN_KDED) << "\tPart of the Id: " << Serializer::outputId(output);
        hashList.insert(0, Serializer::outputId(output));
    }

    qSort(hashList.begin(), hashList.end());

    const QByteArray hash = QCryptographicHash::hash(hashList.join(QString()).toLatin1(),
                                                     QCryptographicHash::Md5).toHex();
    qCDebug(KSCREEN_KDED) << "\tConfig ID:" << hash;
    return hash;
}

bool Serializer::configExists(const KScreen::ConfigPtr &config)
{
    return Serializer::configExists(Serializer::configId(config));
}

bool Serializer::configExists(const QString& id)
{
    QString path(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kscreen/") + id);
    return QFile::exists(path);
}

KScreen::ConfigPtr Serializer::config(const KScreen::ConfigPtr &currentConfig, const QString& id)
{
    KScreen::ConfigPtr config = currentConfig->clone();

    QFile file(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kscreen/") + id);
    if (!file.open(QIODevice::ReadOnly))
        return KScreen::ConfigPtr();

    KScreen::OutputList outputList = config->outputs();
    QJsonDocument parser;
    QVariantList outputs = parser.fromJson(file.readAll()).toVariant().toList();
    Q_FOREACH(KScreen::OutputPtr output, outputList) {
        if (!output->isConnected() && output->isEnabled()) {
            output->setEnabled(false);
        }
    }

    Q_FOREACH(const QVariant &info, outputs) {
        KScreen::OutputPtr output = Serializer::findOutput(config, info.toMap());
        if (!output) {
            continue;
        }

        outputList.remove(output->id());
        outputList.insert(output->id(), output);
    }
    config->setOutputs(outputList);

    return config;
}

bool Serializer::saveConfig(const KScreen::ConfigPtr &config)
{
    const KScreen::OutputList outputs = config->outputs();

    QVariantList outputList;
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        QVariantMap info;

        info["id"] = Serializer::outputId(output);
        info["primary"] = output->isPrimary();
        info["enabled"] = output->isEnabled();
        info["rotation"] = output->rotation();

        QVariantMap pos;
        pos["x"] = output->pos().x();
        pos["y"] = output->pos().y();
        info["pos"] = pos;

        if (output->isEnabled()) {
            const KScreen::ModePtr mode = output->currentMode();
            if (!mode) {
                qWarning() << "CurrentMode is null" << output->name();
                return false;
            }

            QVariantMap modeInfo;
            modeInfo["refresh"] = mode->refreshRate();

            QVariantMap modeSize;
            modeSize["width"] = mode->size().width();
            modeSize["height"] = mode->size().height();
            modeInfo["size"] = modeSize;

            info["mode"] = modeInfo;
        }

        info["metadata"] = Serializer::metadata(output);

        outputList.append(info);
    }

    const QString directory = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kscreen/");
    bool b = QDir().mkpath(directory);
    Q_ASSERT(b);
    QString filePath = directory + Serializer::configId(config);
    QFile file(filePath);
    b = file.open(QIODevice::WriteOnly);
    Q_ASSERT(b);
    file.write(QJsonDocument::fromVariant(outputList).toJson());
    qCDebug(KSCREEN_KDED) << "Config saved on: " << filePath;

    return true;
}

KScreen::OutputPtr Serializer::findOutput(const KScreen::ConfigPtr &config, const QVariantMap& info)
{
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(KScreen::OutputPtr output, outputs) {
        if (!output->isConnected()) {
            continue;
        }
        if (Serializer::outputId(output) != info["id"].toString()) {
            continue;
        }

        const QVariantMap posInfo = info["pos"].toMap();
        QPoint point(posInfo["x"].toInt(), posInfo["y"].toInt());
        output->setPos(point);
        output->setPrimary(info["primary"].toBool());
        output->setEnabled(info["enabled"].toBool());
        output->setRotation(static_cast<KScreen::Output::Rotation>(info["rotation"].toInt()));

        const QVariantMap modeInfo = info["mode"].toMap();
        const QVariantMap modeSize = modeInfo["size"].toMap();
        const QSize size = QSize(modeSize["width"].toInt(), modeSize["height"].toInt());

        qCDebug(KSCREEN_KDED) << "Finding a mode for" << size << "@" << modeInfo["refresh"].toFloat();

        KScreen::ModeList modes = output->modes();
        KScreen::ModePtr matchingMode;
        Q_FOREACH(const KScreen::ModePtr &mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (!qFuzzyCompare(mode->refreshRate(), modeInfo["refresh"].toFloat())) {
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
