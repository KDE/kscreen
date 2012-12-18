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

#ifndef KDED_GENERATOR_H
#define KDED_GENERATOR_H

#include <QtCore/QObject>
#include <QtCore/QString>

namespace KScreen
{
    class Config;
}
class Generator : public QObject
{
    public:
        explicit Generator(QObject* parent = 0);
        virtual ~Generator();

        KScreen::Config* idealConfig();

        void setForceLaptop(bool force);
        void setForceLidClosed(bool force);
        void setForceDocked(bool force);

    private:
        KScreen::Config* laptop();
        KScreen::Config* dockedLaptop();
        KScreen::Config* desktop();

        bool isLaptop();
        bool isEmbedded(const QString &name);
        bool isLidClosed();
        bool isDocked();

        bool m_forceLaptop;
        bool m_forceLidClosed;
        bool m_forceDocked;
};

#endif //KDED_GENERATOR_H