/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
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
    KScreenDaemon(QObject *parent, const QList<QVariant> &);
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
    void disableOutput(const KScreen::OutputPtr &output);
    void showOsd(const QString &icon, const QString &text);

    void updateOrientation();

    std::unique_ptr<Config> m_monitoredConfig;
    bool m_monitoring;
    bool m_configDirty = true;
    QTimer *m_changeCompressor;
    QTimer *m_saveTimer;
    QTimer *m_lidClosedTimer;
    KScreen::OsdManager *m_osdManager;
    OrientationSensor *m_orientationSensor;
    bool m_startingUp = true;
};

#endif /*KSCREEN_DAEMON_H*/
