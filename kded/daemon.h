/************************************************************************************
*  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
*  Copyright 2018 Roman Gilg <subdiff@gmail.com>                                    *
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
#ifndef KSCREEN_DAEMON_H
#define KSCREEN_DAEMON_H

#include "../common/globals.h"
#include "osdaction.h"

#include <kscreen/config.h>

#include <kdedmodule.h>

#include <QVariant>

#include <memory>

class Config;
class OrientationSensor;

namespace KScreen
{
class OsdManager;
}

class QTimer;

class KScreenDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KScreen")

public:
    KScreenDaemon(QObject *parent, const QList<QVariant>&);
    ~KScreenDaemon() override;

public Q_SLOTS:
    // DBus
    void applyLayoutPreset(const QString &presetName);
    bool getAutoRotate();
    void setAutoRotate(bool value);

Q_SIGNALS:
    // DBus
    void outputConnected(const QString &outputName);
    void unknownOutputConnected(const QString &outputName);

private:
    Q_INVOKABLE void getInitialConfig();
    void init();

    void applyConfig();
    void applyKnownConfig();
    void applyIdealConfig();
    void configChanged();
    void saveCurrentConfig();
    void displayButton();
    void lidClosedChanged(bool lidIsClosed);
    void disableLidOutput();
    void setMonitorForChanges(bool enabled);

    void outputConnectedChanged();
    void showOutputIdentifier();
    void applyOsdAction(KScreen::OsdAction::Action action);

    void doApplyConfig(const KScreen::ConfigPtr &config);
    void doApplyConfig(std::unique_ptr<Config> config);
    void refreshConfig();

    void monitorConnectedChange();
    void disableOutput(KScreen::OutputPtr &output);
    void showOsd(const QString &icon, const QString &text);

    void updateOrientation();

    std::unique_ptr<Config> m_monitoredConfig;
    bool m_monitoring;
    bool m_configDirty = true;
    QTimer* m_changeCompressor;
    QTimer* m_saveTimer;
    QTimer* m_lidClosedTimer;
    KScreen::OsdManager *m_osdManager;
    OrientationSensor *m_orientationSensor;
    bool m_startingUp = true;
};

#endif /*KSCREEN_DAEMON_H*/
