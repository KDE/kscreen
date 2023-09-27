/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "../common/globals.h"
#include "../common/osdaction.h"
#include "config-X11.h"

#include <kscreen/config.h>

#include <kdedmodule.h>

#include <QVariant>

#include <memory>

class Config;
class OrientationSensor;
class OrgKdeKscreenOsdServiceInterface;

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
    KScreenDaemon(QObject *parent, const QList<QVariant> &);
    ~KScreenDaemon() override;

public Q_SLOTS:
    // DBus
    void applyLayoutPreset(const QString &presetName);
    bool getAutoRotate();
    void setAutoRotate(bool value);
    bool isAutoRotateAvailable();
    bool externalScreenPresent();

Q_SIGNALS:
    void externalScreenPresentChanged();

private:
    Q_INVOKABLE void getInitialConfig();
    void init();

    void applyConfig();
    void applyKnownConfig();
    void applyIdealConfig();
    void configChanged();
    void saveCurrentConfig();
    void displayButton();
#if HAVE_X11
    void alignX11TouchScreen();
#endif
    void lidClosedChanged(bool lidIsClosed);
    void disableLidOutput();
    void setMonitorForChanges(bool enabled);

    void outputConnectedChanged();
    void showOSD();
    void applyOsdAction(KScreen::OsdAction::Action action);

    void doApplyConfig(const KScreen::ConfigPtr &config);
    void doApplyConfig(std::unique_ptr<Config> config);
    void refreshConfig();

    void monitorConnectedChange();
    void disableOutput(const KScreen::OutputPtr &output);

    void updateOrientation();

    std::unique_ptr<Config> m_monitoredConfig;
    bool m_monitoring;
    bool m_configDirty = true;
    QTimer *m_changeCompressor;
    QTimer *m_saveTimer;
    QTimer *m_lidClosedTimer;
    OrgKdeKscreenOsdServiceInterface *m_osdServiceInterface;
    OrientationSensor *m_orientationSensor;

    bool m_startingUp = true;
    bool m_exernalScreenPresent = false;
};
