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
        static QString currentConfigId();

        static bool configExists();
        static bool configExists(const QString &configId);

        static KScreen::Config* config(const QString &configId, const QString &profileId = QString());
        static bool saveConfig(KScreen::Config* config);

        static KScreen::Output* findOutput(const KScreen::Config *config, const QVariantMap &info);
        static QString outputId(const KScreen::Output* output);

        static QVariantMap metadata(const KScreen::Output* output);

        static QMap<QString,QString> listProfiles(const QString &configId);
        static void removeProfile(const QString &configId, const QString &profileId);

    private:
        static QString configFileName(const QString &configId);
        static QVariant loadConfigFile(const QString &configId);
};

#endif //KDED_SERIALIZER_H
