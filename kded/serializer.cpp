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
#include <QtCore/QDebug>

#include <qjson/serializer.h>
#include <qjson/parser.h>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>
#include <KStandardDirs>

QString Serializer::currentId()
{
    KScreen::OutputList outputs = KScreen::Config::current()->outputs();

    QStringList hashList;
    Q_FOREACH(const KScreen::Output* output, outputs) {
        if (!output->isConnected()) {
            continue;
        }
        hashList.insert(0, output->edid()->hash());
    }

    qSort(hashList.begin(), hashList.end());

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(hashList.join(QString()).toAscii());
    return hash.result().toHex();
}

bool Serializer::configExists()
{
    return Serializer::configExists(Serializer::currentId());
}

bool Serializer::configExists(const QString& id)
{
    QString path = KStandardDirs::locateLocal("data", "kscreen/"+id);
    return QFile::exists(path);
}

KScreen::Config* Serializer::config(const QString& id)
{
    QJson::Parser parser;
    KScreen::Config* config = KScreen::Config::current();
    KScreen::OutputList outputList = config->outputs();
    QFile file(KStandardDirs::locateLocal("data", "kscreen/"+id));
    file.open(QIODevice::ReadOnly);

    QVariantList outputs = parser.parse(file.readAll()).toList();
    Q_FOREACH(const QVariant &info, outputs) {
        KScreen::Output* output = Serializer::findOutput(info.toMap());
        delete outputList.take(output->id());
        outputList.insert(output->id(), output);
    }

    config->setOutputs(outputList);

    return config;
}

bool Serializer::saveConfig(KScreen::Config* config)
{
    KScreen::OutputList outputs = config->outputs();

    QVariantList outputList;
    Q_FOREACH(KScreen::Output *output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        QVariantMap info;
        info["hash"] = output->edid()->hash();
        info["mode"] = output->currentMode();
        info["primary"] = output->isPrimary();
        info["enabled"] = output->isEnabled();
        QVariantMap pos;
        pos["x"] = output->pos().x();
        pos["y"] = output->pos().y();
        info["pos"] = pos;

        outputList.append(info);
    }

    qDebug() << outputList.count();
    bool ok;
    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(outputList);

    QString path = KStandardDirs::locateLocal("data", "kscreen/"+ Serializer::currentId());
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(json);
    file.close();

    qDebug() << "Config saved on: " << path;
    return true;
}

KScreen::Output* Serializer::findOutput(const QVariantMap& info)
{
    KScreen::OutputList outputs = KScreen::Config::current()->outputs();
    Q_FOREACH(KScreen::Output* output, outputs) {
        if (!output->isConnected()) {
            continue;
        }
        if (!output->edid()->isValid()) {
            continue;
        }
        if (output->edid()->hash() != info["hash"].toString()) {
            continue;
        }

        QVariantMap posInfo = info["pos"].toMap();
        QPoint point(posInfo["x"].toInt(), posInfo["y"].toInt());
        output->setPos(point);
        output->setCurrentMode(info["mode"].toInt());
        output->setPrimary(info["primary"].toBool());
        output->setEnabled(info["enabled"].toBool());

        return output;
    }

    return 0;
}