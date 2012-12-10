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

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>

QString Serializer::currentId()
{
    KScreen::OutputList outputs = KScreen::Config::current()->outputs();

    QStringList hashList;
    Q_FOREACH(const KScreen::Output* output, outputs) {
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
    return false;
}

KScreen::Config* Serializer::config(const QString& id)
{
    return new KScreen::Config();
}

bool Serializer::saveConfig(KScreen::Config* config)
{
    return false;
}