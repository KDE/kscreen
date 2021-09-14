/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDED_GENERATOR_H
#define KDED_GENERATOR_H

#include <QObject>

#include <kscreen/config.h>
#include <kscreen/mode.h>
#include <kscreen/output.h>

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

    static Generator *self();
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

    void initializeOutput(const KScreen::OutputPtr &output, KScreen::Config::Features features);
    KScreen::ModePtr bestModeForSize(const KScreen::ModeList &modes, const QSize &size);
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

    static Generator *instance;
};

#endif // KDED_GENERATOR_H
