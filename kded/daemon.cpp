/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright 2016 by Sebastian Kügler <sebas@kde.org>                               *
 *  Copyright (c) 2018 Kai Uwe Broulik <kde@broulik.de>                              *
 *                    Work sponsored by the LiMux project of                         *
 *                    the city of Munich.                                            *
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

#include "daemon.h"
#include "serializer.h"
#include "generator.h"
#include "device.h"
#include "kscreenadaptor.h"
#include "kscreen_daemon_debug.h"
#include "osdmanager.h"

#include <QTimer>
#include <QAction>
#include <QShortcut>
#include <QLoggingCategory>

#include <KLocalizedString>
#include <KActionCollection>
#include <KPluginFactory>
#include <KGlobalAccel>

#include <kscreen/log.h>
#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

K_PLUGIN_FACTORY_WITH_JSON(KScreenDaemonFactory,
                           "kscreen.json",
                           registerPlugin<KScreenDaemon>();)

KScreenDaemon::KScreenDaemon(QObject* parent, const QList< QVariant >& )
 : KDEDModule(parent)
 , m_monitoredConfig(nullptr)
 , m_monitoring(false)
 , m_changeCompressor(new QTimer(this))
 , m_saveTimer(nullptr)
 , m_lidClosedTimer(new QTimer(this))
 
{
    KScreen::Log::instance();
    QMetaObject::invokeMethod(this, "requestConfig", Qt::QueuedConnection);
}

void KScreenDaemon::requestConfig()
{
    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
            this, &KScreenDaemon::configReady);
}

void KScreenDaemon::configReady(KScreen::ConfigOperation* op)
{
    if (op->hasError()) {
        return;
    }

    m_monitoredConfig = qobject_cast<KScreen::GetConfigOperation*>(op)->config();
    qCDebug(KSCREEN_KDED) << "Config" << m_monitoredConfig.data() << "is ready";
    KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig);

    init();
}

KScreenDaemon::~KScreenDaemon()
{
    Generator::destroy();
    Device::destroy();
}

void KScreenDaemon::init()
{
    KActionCollection *coll = new KActionCollection(this);
    QAction* action = coll->addAction(QStringLiteral("display"));
    action->setText(i18n("Switch Display" ));
    QList<QKeySequence> switchDisplayShortcuts({Qt::Key_Display, Qt::MetaModifier + Qt::Key_P});
    KGlobalAccel::self()->setGlobalShortcut(action, switchDisplayShortcuts);
    connect(action, &QAction::triggered, this, &KScreenDaemon::displayButton);

    new KScreenAdaptor(this);
    // Initialize OSD manager to register its dbus interface
    m_osdManager = new KScreen::OsdManager(this);

    m_changeCompressor->setInterval(10);
    m_changeCompressor->setSingleShot(true);
    connect(m_changeCompressor, &QTimer::timeout, this, &KScreenDaemon::applyConfig);

    m_lidClosedTimer->setInterval(1000);
    m_lidClosedTimer->setSingleShot(true);
    connect(m_lidClosedTimer, &QTimer::timeout, this, &KScreenDaemon::lidClosedTimeout);


    connect(Device::self(), &Device::lidClosedChanged, this, &KScreenDaemon::lidClosedChanged);
    connect(Device::self(), &Device::resumingFromSuspend, this,
            [&]() {
                KScreen::Log::instance()->setContext(QStringLiteral("resuming"));
                qCDebug(KSCREEN_KDED) << "Resumed from suspend, checking for screen changes";
                // We don't care about the result, we just want to force the backend
                // to query XRandR so that it will detect possible changes that happened
                // while the computer was suspended, and will emit the change events.
                new KScreen::GetConfigOperation(KScreen::GetConfigOperation::NoEDID, this);
            });
    connect(Device::self(), &Device::aboutToSuspend, this,
            [&]() {
                qCDebug(KSCREEN_KDED) << "System is going to suspend, won't be changing config (waited for " << (m_lidClosedTimer->interval() - m_lidClosedTimer->remainingTime()) << "ms)";
                m_lidClosedTimer->stop();
            });


    connect(Generator::self(), &Generator::ready,
            this, &KScreenDaemon::applyConfig);

    Generator::self()->setCurrentConfig(m_monitoredConfig);
    monitorConnectedChange();
}

void KScreenDaemon::doApplyConfig(const KScreen::ConfigPtr& config)
{
    qCDebug(KSCREEN_KDED) << "doApplyConfig()";
    setMonitorForChanges(false);
    m_monitoredConfig = config;
    KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig);

    connect(new KScreen::SetConfigOperation(config), &KScreen::SetConfigOperation::finished, this,
            [&]() {
                qCDebug(KSCREEN_KDED) << "Config applied";
                setMonitorForChanges(true);
            });
}

void KScreenDaemon::applyConfig()
{
    qCDebug(KSCREEN_KDED) << "Applying config";
    if (Serializer::configExists(m_monitoredConfig)) {
        applyKnownConfig();
        return;
    }

    applyIdealConfig();
}

void KScreenDaemon::applyKnownConfig()
{
    const QString configId = Serializer::configId(m_monitoredConfig);
    qCDebug(KSCREEN_KDED) << "Applying known config" << configId;

    // We may look for a config that has been set when the lid was closed, Bug: 353029
    if (Device::self()->isLaptop() && !Device::self()->isLidClosed()) {
        Serializer::moveConfig(configId  + QLatin1String("_lidOpened"), configId);
    }

    KScreen::ConfigPtr config = Serializer::config(m_monitoredConfig, configId);
    // It's possible that the Serializer returned a nullptr
    if (!config || !KScreen::Config::canBeApplied(config, KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen)) {
        applyIdealConfig();
        return;
    }
    doApplyConfig(config);
}

void KScreenDaemon::applyLayoutPreset(const QString &presetName)
{
    const QMetaEnum actionEnum = QMetaEnum::fromType<KScreen::OsdAction::Action>();
    Q_ASSERT(actionEnum.isValid());

    bool ok;
    auto action = static_cast<KScreen::OsdAction::Action>(actionEnum.keyToValue(qPrintable(presetName), &ok));
    if (!ok) {
        qCWarning(KSCREEN_KDED) << "Cannot apply unknown screen layout preset named" << presetName;
        return;
    }
    applyOsdAction(action);
}

void KScreenDaemon::applyOsdAction(KScreen::OsdAction::Action action)
{
    switch (action) {
    case KScreen::OsdAction::NoAction:
        qCDebug(KSCREEN_KDED) << "OSD: no action";
        return;
    case KScreen::OsdAction::SwitchToInternal:
        qCDebug(KSCREEN_KDED) << "OSD: switch to internal";
        doApplyConfig(Generator::self()->displaySwitch(Generator::TurnOffExternal));
        return;
    case KScreen::OsdAction::SwitchToExternal:
        qCDebug(KSCREEN_KDED) << "OSD: switch to external";
        doApplyConfig(Generator::self()->displaySwitch(Generator::TurnOffEmbedded));
        return;
    case KScreen::OsdAction::ExtendLeft:
        qCDebug(KSCREEN_KDED) << "OSD: extend left";
        doApplyConfig(Generator::self()->displaySwitch(Generator::ExtendToLeft));
        return;
    case KScreen::OsdAction::ExtendRight:
        qCDebug(KSCREEN_KDED) << "OSD: extend right";
        doApplyConfig(Generator::self()->displaySwitch(Generator::ExtendToRight));
        return;
    case KScreen::OsdAction::Clone:
        qCDebug(KSCREEN_KDED) << "OSD: clone";
        doApplyConfig(Generator::self()->displaySwitch(Generator::Clone));
        return;
    }

    Q_UNREACHABLE();
}

void KScreenDaemon::applyIdealConfig()
{

    if (m_monitoredConfig->connectedOutputs().count() < 2) {
        m_osdManager->hideOsd();
        doApplyConfig(Generator::self()->idealConfig(m_monitoredConfig));
    } else {
        qCDebug(KSCREEN_KDED) << "Getting ideal config from user via OSD...";
        auto action = m_osdManager->showActionSelector();
        connect(action, &KScreen::OsdAction::selected,
                this, &KScreenDaemon::applyOsdAction);
    }
}

void logConfig(const KScreen::ConfigPtr &config) {
    if (config) {
        foreach (auto o, config->outputs()) {
            if (o->isConnected()) {
                qCDebug(KSCREEN_KDED) << o;
            }
        }
    }
}

void KScreenDaemon::configChanged()
{
    qCDebug(KSCREEN_KDED) << "Change detected";
    logConfig(m_monitoredConfig);

    // Modes may have changed, fix-up current mode id
    bool changed = false;
    Q_FOREACH(const KScreen::OutputPtr &output, m_monitoredConfig->outputs()) {
        if (output->isConnected() && output->isEnabled() && output->currentMode().isNull()) {
            qCDebug(KSCREEN_KDED) << "Current mode" << output->currentModeId() << "invalid, setting preferred mode" << output->preferredModeId();
            output->setCurrentModeId(output->preferredModeId());
            changed = true;
        }
    }
    if (changed) {
        doApplyConfig(m_monitoredConfig);
    }

    // Reset timer, delay the writeback
    if (!m_saveTimer) {
        m_saveTimer = new QTimer(this);
        m_saveTimer->setInterval(300);
        m_saveTimer->setSingleShot(true);
        connect(m_saveTimer, &QTimer::timeout, this, &KScreenDaemon::saveCurrentConfig);
    }
    m_saveTimer->start();
}

void KScreenDaemon::saveCurrentConfig()
{
    qCDebug(KSCREEN_KDED) << "Saving current config to file";

    // We assume the config is valid, since it's what we got, but we are interested
    // in the "at least one enabled screen" check

    const bool valid = KScreen::Config::canBeApplied(m_monitoredConfig, KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen);
    if (valid) {
        Serializer::saveConfig(m_monitoredConfig, Serializer::configId(m_monitoredConfig));
        logConfig(m_monitoredConfig);
    } else {
        qCWarning(KSCREEN_KDED) << "Config does not have at least one screen enabled, WILL NOT save this config, this is not what user wants.";
        logConfig(m_monitoredConfig);
    }
}

void KScreenDaemon::showOsd(const QString &icon, const QString &text)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QLatin1Literal("org.kde.plasmashell"),
        QLatin1Literal("/org/kde/osdService"),
        QLatin1Literal("org.kde.osdService"),
        QLatin1Literal("showText")
    );
    msg << icon << text;
    QDBusConnection::sessionBus().asyncCall(msg);
}

void KScreenDaemon::showOutputIdentifier()
{
    m_osdManager->showOutputIdentifiers();
}

void KScreenDaemon::displayButton()
{
    qCDebug(KSCREEN_KDED) << "displayBtn triggered";

    auto action = m_osdManager->showActionSelector();
    connect(action, &KScreen::OsdAction::selected,
            this, &KScreenDaemon::applyOsdAction);
}

void KScreenDaemon::lidClosedChanged(bool lidIsClosed)
{
    // Ignore this when we don't have any external monitors, we can't turn off our
    // only screen
    if (m_monitoredConfig->connectedOutputs().count() == 1) {
        return;
    }

    if (lidIsClosed) {
        // Lid is closed, now we wait for couple seconds to find out whether it
        // will trigger a suspend (see Device::aboutToSuspend), or whether we should
        // turn off the screen
        qCDebug(KSCREEN_KDED) << "Lid closed, waiting to see if the computer goes to sleep...";
        m_lidClosedTimer->start();
        return;
    } else {
        qCDebug(KSCREEN_KDED) << "Lid opened!";
        // We should have a config with "_lidOpened" suffix lying around. If not,
        // then the configuration has changed while the lid was closed and we just
        // use applyConfig() and see what we can do ...

        const QString openConfigId = Serializer::configId(m_monitoredConfig) + QLatin1String("_lidOpened");
        if (Serializer::configExists(openConfigId)) {
            const KScreen::ConfigPtr openedConfig = Serializer::config(m_monitoredConfig, openConfigId);
            Serializer::removeConfig(openConfigId);

            doApplyConfig(openedConfig);
        }
    }
}

void KScreenDaemon::lidClosedTimeout()
{
    // Make sure nothing has changed in the past second... :-)
    if (!Device::self()->isLidClosed()) {
        return;
    }

    // If we are here, it means that closing the lid did not result in suspend
    // action.
    // FIXME: This could be because the suspend took longer than m_lidClosedTimer
    // timeout. Ideally we need to be able to look into PowerDevil config to see
    // what's the configured action for lid events, but there's no API to do that
    // and I'm not parsing PowerDevil's configs...

    qCDebug(KSCREEN_KDED) << "Lid closed without system going to suspend -> turning off the screen";
    for (KScreen::OutputPtr &output : m_monitoredConfig->outputs()) {
        if (output->type() == KScreen::Output::Panel) {
            if (output->isConnected() && output->isEnabled()) {
                // Save the current config with opened lid, just so that we know
                // how to restore it later
                const QString configId = Serializer::configId(m_monitoredConfig) + QLatin1String("_lidOpened");
                Serializer::saveConfig(m_monitoredConfig, configId);
                disableOutput(m_monitoredConfig, output);
                doApplyConfig(m_monitoredConfig);
                return;
            }
        }
    }
}


void KScreenDaemon::outputConnectedChanged()
{
    if (!m_changeCompressor->isActive()) {
        m_changeCompressor->start();
    }

    KScreen::Output *output = qobject_cast<KScreen::Output*>(sender());
    qCDebug(KSCREEN_KDED) << "outputConnectedChanged():" << output->name();

    if (output->isConnected()) {
        Q_EMIT outputConnected(output->name());

        if (!Serializer::configExists(m_monitoredConfig)) {
            Q_EMIT unknownOutputConnected(output->name());
        }
    }
}

void KScreenDaemon::monitorConnectedChange()
{
    KScreen::OutputList outputs = m_monitoredConfig->outputs();
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        connect(output.data(), &KScreen::Output::isConnectedChanged,
                this, &KScreenDaemon::outputConnectedChanged,
                Qt::UniqueConnection);
    }
    connect(m_monitoredConfig.data(), &KScreen::Config::outputAdded, this,
        [this] (const KScreen::OutputPtr output) {
            if (output->isConnected()) {
                m_changeCompressor->start();
            }
            connect(output.data(), &KScreen::Output::isConnectedChanged,
                    this, &KScreenDaemon::outputConnectedChanged,
                    Qt::UniqueConnection);
        }, Qt::UniqueConnection
    );
    connect(m_monitoredConfig.data(), &KScreen::Config::outputRemoved,
            this, &KScreenDaemon::applyConfig,
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
}

void KScreenDaemon::setMonitorForChanges(bool enabled)
{
    if (m_monitoring == enabled) {
        return;
    }

    qCDebug(KSCREEN_KDED) << "Monitor for changes: " << enabled;
    m_monitoring = enabled;
    if (m_monitoring) {
        connect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged,
                this, &KScreenDaemon::configChanged, Qt::UniqueConnection);
    } else {
        disconnect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged,
                   this, &KScreenDaemon::configChanged);
    }
}

void KScreenDaemon::disableOutput(KScreen::ConfigPtr &config, KScreen::OutputPtr &output)
{
    const QRect geom = output->geometry();
    qCDebug(KSCREEN_KDED) << "Laptop geometry:" << geom << output->pos() << (output->currentMode() ? output->currentMode()->size() : QSize());

    // Move all outputs right from the @p output to left
    for (KScreen::OutputPtr &otherOutput : config->outputs()) {
        if (otherOutput == output || !otherOutput->isConnected() || !otherOutput->isEnabled()) {
            continue;
        }

        QPoint otherPos = otherOutput->pos();
        if (otherPos.x() >= geom.right() && otherPos.y() >= geom.top() && otherPos.y() <= geom.bottom()) {
            otherPos.setX(otherPos.x() - geom.width());
        }
        qCDebug(KSCREEN_KDED) << "Moving" << otherOutput->name() << "from" << otherOutput->pos() << "to" << otherPos;
        otherOutput->setPos(otherPos);
    }

    // Disable the output
    output->setEnabled(false);
}


#include "daemon.moc"
