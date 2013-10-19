/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2013 by Daniel Vrátil <dvratil@redhat.com>                         *
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
#include <QtCore/QUuid>

#include <qjson/serializer.h>
#include <qjson/parser.h>

#include <kdebug.h>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>

#include <KStandardDirs>
#include <KLocalizedString>

QString Serializer::configFileName(const QString &configId)
{
    return KStandardDirs::locateLocal("data", "kscreen/" + configId);
}

QVariant Serializer::loadConfigFile(const QString &configId)
{
    if (configId.isEmpty() || !Serializer::configExists(configId)) {
        return QVariant();
    }

    QFile file(configFileName(configId));
    if (!file.open(QIODevice::ReadOnly)) {
        return QVariant();
    }

    QJson::Parser parser;
    bool ok = false;
    const QVariant v = parser.parse(file.readAll(), &ok);
    if (!ok || !v.isValid()) {
        return QVariant();
    }

    return v;
}

void Serializer::saveConfigFile(const QString &configId, const QVariant &variant)
{
    if (configId.isEmpty()) {
        kWarning() << "Invalid config ID";
        return;
    }

    QJson::Serializer serializer;
    bool ok = false;
    const QByteArray json = serializer.serialize(variant, &ok);
    if (!ok) {
        kWarning() << "Failed to serialize configuration";
        return;
    }

    QFile file(Serializer::configFileName(configId));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        kWarning() << "Failed to open file" << file.fileName();
        return;
    }

    if (file.write(json) == -1) {
        kWarning() << "Failed to write data to file";
        return;
    }

    file.close();
}

QString Serializer::currentConfigId()
{
    KScreen::OutputList outputs = KScreen::Config::current()->outputs();

    QStringList hashList;
    Q_FOREACH(const KScreen::Output * output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        kDebug() << "Part of the Id: " << Serializer::outputId(output);
        hashList.insert(0, Serializer::outputId(output));
    }

    qSort(hashList.begin(), hashList.end());

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(hashList.join(QString()).toAscii());
    return hash.result().toHex();
}

bool Serializer::configExists()
{
    return Serializer::configExists(Serializer::currentConfigId());
}

bool Serializer::configExists(const QString &configId)
{
    QString path = KStandardDirs::locateLocal("data", "kscreen/" + configId);
    return QFile::exists(path);
}

KScreen::Config *Serializer::config(const QString &configId, const QString &profileId)
{
    QJson::Parser parser;
    KScreen::Config *config = KScreen::Config::current();
    if (!config) {
        return 0;
    }

    KScreen::OutputList outputList = config->outputs();

    QVariantList outputs;

    const QVariant v = loadConfigFile(configId);
    if (v.isNull()) {
        return 0;
    }

    const QVariantMap map = v.toMap();

    // No version? The config is from pre-profiles era
    if (!map.contains(QLatin1String("version"))) {
        outputs = v.toList();
    } else {
        // Version is specified (assume 2), we find the profile we want and get
        // list of it's outputs configuration
        const QVariantList profiles = map[QLatin1String("profiles")].toList();
        if (!profileId.isEmpty()) {
            Q_FOREACH(const QVariant & profile, profiles) {
                const QVariantMap info = profile.toMap();
                if (info[QLatin1String("id")].toString() == profileId) {
                    outputs = info[QLatin1String("outputs")].toList();
                    break;
                }
            }
        }

        // No profile was chosen, or invalid profile ID was given - just get
        // the first profile and continue
        if (profileId.isEmpty() || outputs.isEmpty()) {
            const QVariantMap profile = profiles.first().toMap();
            outputs = profile[QLatin1String("outputs")].toList();
        }
    }

    Q_FOREACH(KScreen::Output * output, outputList) {
        if (!output->isConnected() && output->isEnabled()) {
            output->setEnabled(false);
        }
    }

    KScreen::Config *outputsConfig = config->clone();
    Q_FOREACH(const QVariant & info, outputs) {
        KScreen::Output *output = Serializer::findOutput(outputsConfig, info.toMap());
        if (!output) {
            continue;
        }

        delete outputList.take(output->id());
        outputList.insert(output->id(), output);
    }
    config->setOutputs(outputList);

    return config;
}

bool Serializer::saveConfig(KScreen::Config *config, const QString &currentProfileId)
{
    KScreen::OutputList outputs = config->outputs();

    const QString configId = Serializer::currentConfigId();
    if (Serializer::configExists(configId)) {
        const QVariantMap map = loadConfigFile(configId).toMap();
        // Old version? Delete the file and generate a new one from scratch
        // for this configuration
        if (!map.contains(QLatin1String("version"))) {
            QFile::remove(Serializer::configFileName(configId));
            return saveConfig(config, currentProfileId);
        }

        // Update current profile. If no profile is set, it will update the
        // default one
        updateProfile(config, configId, currentProfileId);
        return true;
    }

    // Configuration either does not exist yes, so we simply create one.

    QVariantList profiles;
    profiles << serializeProfile(config, i18n("Default"));

    QVariantMap map;
    map[QLatin1String("version")] = 2;
    map[QLatin1String("profiles")] = profiles;

    saveConfigFile(configId, map);
    return true;
}

void Serializer::updateProfile(KScreen::Config *config, const QString &configId, const QString &profileId)
{
    const QVariant v = loadConfigFile(configId);
    if (v.isNull()) {
        return;
    }

    QVariantMap map = v.toMap();
    // assumes the file is valid version 2 file

    QVariantList profiles = map[QLatin1String("profiles")].toList();
    for (int i = 0; i < profiles.count(); i++) {
        const QVariantMap profile = profiles.at(i).toMap();
        if (profile[QLatin1String("id")].toString() == profileId) {
            QVariantMap newProfile = profile;
            newProfile[QLatin1String("outputs")] = serializeProfile(config, profile[QLatin1String("name")].toString(), profileId);
            profiles.replace(i, newProfile);
            map[QLatin1String("profiles")] = profiles;
            break;
        }
    }

    saveConfigFile(configId, map);
}

QString Serializer::createProfile(KScreen::Config *config, const QString &name)
{
    const QVariant v = loadConfigFile(currentConfigId());
    if (v.isNull()) {
        return QString();
    }

    QVariantMap map = v.toMap();
    QVariantList profiles = map[QLatin1String("profiles")].toList();
    const QVariantMap newProfile = serializeProfile(config, name).toMap();

    profiles << newProfile;
    map[QLatin1String("version")] = 2;
    map[QLatin1String("profiles")] = profiles;

    saveConfigFile(Serializer::currentConfigId(), map);

    return newProfile[QLatin1String("id")].toString();
}

QVariant Serializer::serializeProfile(KScreen::Config *config, const QString &name, const QString &profileId)
{
    QVariantList outputList;
    Q_FOREACH(KScreen::Output * output, config->outputs()) {
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
            KScreen::Mode *mode = output->currentMode();

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

    QVariantMap map;
    map[QLatin1String("id")] = profileId.isEmpty() ? QUuid::createUuid().toString() : profileId;
    map[QLatin1String("name")] = name;
    map[QLatin1String("outputs")] = outputList;

    return map;
}

KScreen::Output *Serializer::findOutput(const KScreen::Config *config, const QVariantMap &info)
{
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(KScreen::Output * output, outputs) {
        if (!output->isConnected()) {
            continue;
        }
        if (Serializer::outputId(output) != info["id"].toString()) {
            continue;
        }

        QVariantMap posInfo = info["pos"].toMap();
        QPoint point(posInfo["x"].toInt(), posInfo["y"].toInt());
        output->setPos(point);
        output->setPrimary(info["primary"].toBool());
        output->setEnabled(info["enabled"].toBool());
        output->setRotation(static_cast<KScreen::Output::Rotation>(info["rotation"].toInt()));

        QVariantMap modeInfo = info["mode"].toMap();
        QVariantMap modeSize = modeInfo["size"].toMap();
        QSize size(modeSize["width"].toInt(), modeSize["height"].toInt());

        kDebug() << "Finding a mode with: ";
        kDebug() << size;
        kDebug() << modeInfo["refresh"].toString();

        KScreen::ModeList modes = output->modes();
        Q_FOREACH(KScreen::Mode * mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (QString::number(mode->refreshRate()) != modeInfo["refresh"].toString()) {
                continue;
            }

            kDebug() << "Found: " << mode->id() << " " << mode->name();
            output->setCurrentModeId(mode->id());
            break;
        }
        return output;
    }

    return 0;
}

QString Serializer::outputId(const KScreen::Output *output)
{
    if (output->edid() && output->edid()->isValid()) {
        return output->edid()->hash();
    }

    return output->name();
}

QVariantMap Serializer::metadata(const KScreen::Output *output)
{
    QVariantMap metadata;
    metadata["name"] = output->name();
    if (!output->edid() || !output->edid()->isValid()) {
        return metadata;
    }

    metadata["fullname"] = output->edid()->deviceId();
    return metadata;
}

QMap<QString, QString> Serializer::listProfiles(const QString &configId)
{
    QMap<QString, QString> profiles;

    const QVariant v = loadConfigFile(configId);
    const QVariantMap map = v.toMap();
    // Version 1, or invalid content - return default as current
    if (!map.contains(QLatin1String("version"))) {
        profiles.insert(QString(), i18n("Default"));
        return profiles;
    }

    const QVariantList profilesList = map[QLatin1String("profiles")].toList();
    Q_FOREACH(const QVariant & profile, profilesList) {
        const QVariantMap info = profile.toMap();
        profiles.insert(info[QLatin1String("id")].toString(),
                        info[QLatin1String("name")].toString());
    }

    return profiles;
}

void Serializer::removeProfile(const QString &configId, const QString &profileId)
{
    const QVariant v = loadConfigFile(configId);
    if (v.isNull()) {
        return;
    }

    QVariantMap map = v.toMap();

    // Version 1 did not support profiles
    if (!map.contains(QLatin1String("version"))) {
        return;
    }

    QVariantList profiles = map[QLatin1String("profiles")].toList();
    // Find the profile, remove it from the list
    for (int i = 0; i < profiles.count(); i++) {
        const QVariantMap info = profiles.at(i).toMap();
        if (info[QLatin1String("id")].toString() == profileId) {
            profiles.removeAt(i);
            break;
        }
    }

    // Update the parent map
    map[QLatin1String("profiles")] = profiles;

    // Write it back to file
    QJson::Serializer serializer;
    QFile file(configFileName(configId));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(serializer.serialize(map));
    file.close();
}

QVariant Serializer::loadProfile(const QString &configId, const QString &profileId)
{
    const QVariant v = loadConfigFile(configId);
    if (v.isNull()) {
        return QVariantList();
    }

    QVariantMap map = v.toMap();
    // Old version, "v" contains directly the list of outputs
    if (!map.contains(QLatin1String("version"))) {
        return v;
    }

    const QVariantList profiles = map[QLatin1String("profiles")].toList();
    Q_FOREACH (const QVariant &profile, profiles) {
        const QVariantMap info = profile.toMap();
        if (info[QLatin1String("id")].toString() == profileId) {
            return profile;
        }
    }

    return QVariantList();
}

