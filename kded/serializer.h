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

#ifndef KDED_SERIALIZER_H
#define KDED_SERIALIZER_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include <kscreen/config.h>

class Serializer
{
    public:
        static QString configId(const KScreen::ConfigPtr &config);
        static bool configExists(const KScreen::ConfigPtr &config);
        static bool configExists(const QString& id);
        static KScreen::ConfigPtr config(const KScreen::ConfigPtr &currentConfig, const QString& id);
        static bool saveConfig(const KScreen::ConfigPtr &config);

        static KScreen::OutputPtr findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info);
        static QString outputId(const KScreen::OutputPtr &output);
        static QVariantMap metadata(const KScreen::OutputPtr &output);
};

#endif //KDED_SERIALIZER_H
