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

namespace KScreen
{
    class Config;
    class Output;
}
class Serializer
{
    public:
        static QString currentId();
        static bool configExists();
        static bool configExists(const QString& id);
        static KScreen::Config* config(const QString& id);
        static bool saveConfig(KScreen::Config* config);

        static KScreen::Output* findOutput(const QVariantMap &info);
        static QString outputId(const KScreen::Output* output);
        static QVariantMap metadata(const KScreen::Output* output);
};

#endif //KDED_SERIALIZER_H