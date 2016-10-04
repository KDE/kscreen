/***************************************************************************
 *                                                                         *
 *   Copyright 2016 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KSCREEN_CONFIGMODULE_H
#define KSCREEN_CONFIGMODULE_H

#include <QObject>
#include <QVariant>

#include <KQuickAddons/ConfigModule>

#include <KScreen/Config>
#include <KScreen/ConfigMonitor>

class QMLScreen;
class QMLOutput;

namespace KScreen {

class ConfigOperation;
class ModeSelector;

class ConfigModule : public KQuickAddons::ConfigModule
{
    Q_OBJECT

    Q_PROPERTY(KScreen::ModeSelector* modeSelector
               READ modeSelector
               CONSTANT)

    public:
        ConfigModule(QObject* parent, const QVariantList& args);
        virtual ~ConfigModule();

        KScreen::ModeSelector* modeSelector() const;

    public Q_SLOTS:
        virtual void load();
        virtual void save();
        virtual void defaults();
        void changed();

    Q_SIGNALS:
        void configSet();

    private:
        friend class TestConfigModule;
        void configReady(KScreen::ConfigOperation *op);
        void focusedOutputChanged(QMLOutput *output);
        void setConfig(const KScreen::ConfigPtr &config);
        KScreen::ConfigPtr currentConfig() const;

        KScreen::ModeSelector* m_modeSelector;
        QMLScreen *mScreen;
        KScreen::ConfigPtr mConfig;
        KScreen::ConfigPtr mPrevConfig;

};

}
#endif // KSCREEN_CONFIGMODULE_H
