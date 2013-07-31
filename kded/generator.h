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

#include <kscreen/output.h>
#include <kscreen/mode.h>

namespace KScreen
{
    class Config;
}
class Generator : public QObject
{
    Q_OBJECT
    public:
        static Generator* self();
        static void destroy();

        KScreen::Config* idealConfig();
        KScreen::Config* displaySwitch(int iteration);

        void setForceLaptop(bool force);
        void setForceLidClosed(bool force);
        void setForceDocked(bool force);

    Q_SIGNALS:
        void ready();

    private:
        explicit Generator();
        virtual ~Generator();

        KScreen::Config* fallbackIfNeeded(KScreen::Config *config);

        void laptop(KScreen::OutputList& outputs);

        void singleOutput(KScreen::OutputList& outputs);
        void extendToRight(KScreen::OutputList& outputs);
        KScreen::Mode* biggestMode(const KScreen::ModeList &modes);
        KScreen::Output* biggestOutput(const KScreen::OutputList &outputs);
        KScreen::Output* embeddedOutput(const KScreen::OutputList &outputs);
        void disableAllDisconnectedOutputs(const KScreen::OutputList &outputs);

        bool isLaptop();
        bool isLidClosed();
        bool isDocked();

        bool m_isReady;
        bool m_forceLaptop;
        bool m_forceLidClosed;
        bool m_forceDocked;

        static Generator* instance;
};

#endif //KDED_GENERATOR_H