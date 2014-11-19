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

Q_LOGGING_CATEGORY(KDED_SERIALIZER, "org.kde.KScreen.KDED.Serializer")

QString Serializer::configId(const KScreen::ConfigPtr &currentConfig)
{
    KScreen::OutputList outputs = currentConfig->outputs();

    QStringList hashList;
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        qCDebug(KDED_SERIALIZER) << "Part of the Id: " << Serializer::outputId(output);
        hashList.insert(0, Serializer::outputId(output));
    }

    qSort(hashList.begin(), hashList.end());

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(hashList.join(QString()).toLatin1());
    return hash.result().toHex();
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
    qCDebug(KDED_SERIALIZER) << "Config saved on: " << filePath;

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
        QSize size(modeSize["width"].toInt(), modeSize["height"].toInt());

        qCDebug(KDED_SERIALIZER) << "Finding a mode with: ";
        qCDebug(KDED_SERIALIZER) << size;
        qCDebug(KDED_SERIALIZER) << modeInfo["refresh"].toString();

        KScreen::ModeList modes = output->modes();
        Q_FOREACH(const KScreen::ModePtr &mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (QString::number(mode->refreshRate()) != modeInfo["refresh"].toString()) {
                continue;
            }

            qCDebug(KDED_SERIALIZER) << "Found: " << mode->id() << " " << mode->name();
            output->setCurrentModeId(mode->id());
            break;
        }
        return output;
    }

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
