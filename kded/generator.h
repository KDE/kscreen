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

#include <QObject>

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
        enum DisplaySwitchAction {
            None = 0,
            Clone = 1,
            ExtendToLeft = 2,
            TurnOffEmbedded = 3,
            TurnOffExternal = 4,
            ExtendToRight = 5,
        };

        static Generator* self();
        static void destroy();

        void setCurrentConfig(const KScreen::ConfigPtr &currentConfig);

        KScreen::ConfigPtr idealConfig(const KScreen::ConfigPtr &currentConfig);
        KScreen::ConfigPtr displaySwitch(DisplaySwitchAction iteration);

        void setForceLaptop(bool force);
        void setForceLidClosed(bool force);
        void setForceDocked(bool force);
        void setForceNotLaptop(bool force);

        static KScreen::ModePtr biggestMode(const KScreen::ModeList &modes);

    Q_SIGNALS:
        void ready();

    private:
        explicit Generator();
        ~Generator() override;

        KScreen::ConfigPtr fallbackIfNeeded(const KScreen::ConfigPtr &config);

        void cloneScreens(KScreen::OutputList &connectedOutputs);
        void laptop(KScreen::OutputList &connectedOutputs);
        void singleOutput(KScreen::OutputList &connectedOutputs);
        void extendToRight(KScreen::OutputList &connectedOutputs);

        KScreen::ModePtr bestModeForSize(const KScreen::ModeList& modes, const QSize &size);
        KScreen::ModePtr bestModeForOutput(const KScreen::OutputPtr &output);
        qreal bestScaleForOutput(const KScreen::OutputPtr &output);

        KScreen::OutputPtr biggestOutput(const KScreen::OutputList &connectedOutputs);
        KScreen::OutputPtr embeddedOutput(const KScreen::OutputList &connectedOutputs);
        void disableAllDisconnectedOutputs(const KScreen::OutputList &connectedOutputs);

        bool isLaptop() const;
        bool isLidClosed() const;
        bool isDocked() const;

        bool m_forceLaptop;
        bool m_forceLidClosed;
        bool m_forceNotLaptop;
        bool m_forceDocked;

        KScreen::ConfigPtr m_currentConfig;

        static Generator* instance;
};

#endif //KDED_GENERATOR_H
