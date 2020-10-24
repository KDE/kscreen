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

#include "../common/orientation_sensor.h"
#include "config.h"
#include "generator.h"
#include "device.h"
#include "kscreenadaptor.h"
#include "kscreen_daemon_debug.h"
#include "osdmanager.h"

#include <kscreen/log.h>
#include <kscreen/output.h>
#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

#include <KLocalizedString>
#include <KActionCollection>
#include <KPluginFactory>
#include <KGlobalAccel>

#include <QTimer>
#include <QAction>
#include <QOrientationReading>
#include <QShortcut>

K_PLUGIN_CLASS_WITH_JSON(KScreenDaemon, "kscreen.json")

KScreenDaemon::KScreenDaemon(QObject* parent, const QList< QVariant >& )
 : KDEDModule(parent)
 , m_monitoring(false)
 , m_changeCompressor(new QTimer(this))
 , m_saveTimer(nullptr)
 , m_lidClosedTimer(new QTimer(this))
 , m_orientationSensor(new OrientationSensor(this))
{
    connect(m_orientationSensor, &OrientationSensor::availableChanged,
            this, &KScreenDaemon::updateOrientation);
    connect(m_orientationSensor, &OrientationSensor::valueChanged,
            this, &KScreenDaemon::updateOrientation);

    KScreen::Log::instance();
    QMetaObject::invokeMethod(this, "getInitialConfig", Qt::QueuedConnection);
}

void KScreenDaemon::getInitialConfig()
{
    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
            this, [this](KScreen::ConfigOperation* op) {
        if (op->hasError()) {
            return;
        }

        m_monitoredConfig = std::unique_ptr<Config>(new Config(qobject_cast<KScreen::GetConfigOperation*>(op)->config()));
        m_monitoredConfig->setValidityFlags(KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen);
        qCDebug(KSCREEN_KDED) << "Config" << m_monitoredConfig->data().data() << "is ready";
        KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig->data());

        init();
    });
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
    connect(m_lidClosedTimer, &QTimer::timeout, this, &KScreenDaemon::disableLidOutput);

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

    connect(Generator::self(), &Generator::ready, this, [this] {
        applyConfig();

        if (Device::self()->isLaptop() && Device::self()->isLidClosed()) {
            disableLidOutput();
        }

        m_startingUp = false;
    });

    Generator::self()->setCurrentConfig(m_monitoredConfig->data());
    monitorConnectedChange();
}

void KScreenDaemon::updateOrientation()
{
    if (!m_monitoredConfig) {
        return;
    }
    const auto features = m_monitoredConfig->data()->supportedFeatures();
    if (!features.testFlag(KScreen::Config::Feature::AutoRotation)
          || !features.testFlag(KScreen::Config::Feature::TabletMode) ) {
        return;
    }

    if (!m_orientationSensor->available() || !m_orientationSensor->enabled()) {
        return;
    }

    const auto orientation = m_orientationSensor->value();
    if (orientation == QOrientationReading::Undefined) {
        // Orientation sensor went off. Do not change current orientation.
        return;
    }
    if (orientation == QOrientationReading::FaceUp ||
               orientation == QOrientationReading::FaceDown) {
        // We currently don't do anything with FaceUp/FaceDown, but in the future we could use them
        // to shut off and switch on again a display when display is facing downwards/upwards.
        return;
    }

    m_monitoredConfig->setDeviceOrientation(orientation);
    if (m_monitoring) {
        doApplyConfig(m_monitoredConfig->data());
    } else {
        m_configDirty = true;
    }
}

void KScreenDaemon::doApplyConfig(const KScreen::ConfigPtr& config)
{
    qCDebug(KSCREEN_KDED) << "Do set and apply specific config";
    auto configWrapper = std::unique_ptr<Config>(new Config(config));
    configWrapper->setValidityFlags(KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen);

    doApplyConfig(std::move(configWrapper));
}

void KScreenDaemon::doApplyConfig(std::unique_ptr<Config> config)
{
    m_monitoredConfig = std::move(config);

    m_monitoredConfig->activateControlWatching();
    m_orientationSensor->setEnabled(m_monitoredConfig->autoRotationRequested());

    connect(m_monitoredConfig.get(), &Config::controlChanged, this, [this]() {
            m_orientationSensor->setEnabled(m_monitoredConfig->autoRotationRequested());
        updateOrientation();
    });

    refreshConfig();
}

void KScreenDaemon::refreshConfig()
{
    setMonitorForChanges(false);
    m_configDirty = false;
    KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig->data());

    connect(new KScreen::SetConfigOperation(m_monitoredConfig->data()),
                &KScreen::SetConfigOperation::finished,
            this, [this]() {
        qCDebug(KSCREEN_KDED) << "Config applied";
        if (m_configDirty) {
            // Config changed in the meantime again, apply.
            doApplyConfig(m_monitoredConfig->data());
        } else {
            setMonitorForChanges(true);
        }
    });
}

void KScreenDaemon::applyConfig()
{
    qCDebug(KSCREEN_KDED) << "Applying config";
    if (m_monitoredConfig->fileExists()) {
        applyKnownConfig();
        return;
    }
    applyIdealConfig();
}

void KScreenDaemon::applyKnownConfig()
{
    qCDebug(KSCREEN_KDED) << "Applying known config";

    std::unique_ptr<Config> readInConfig = m_monitoredConfig->readFile();
    if (readInConfig) {
        doApplyConfig(std::move(readInConfig));
    } else {
        // loading not successful, fall back to ideal config
        applyIdealConfig();
    }
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

bool KScreenDaemon::getAutoRotate()
{
    return m_monitoredConfig->getAutoRotate();
}

void KScreenDaemon::setAutoRotate(bool value)
{
    if (!m_monitoredConfig) {
        return;
    }
    m_monitoredConfig->setAutoRotate(value);
    m_orientationSensor->setEnabled(value);
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
    const bool showOsd = m_monitoredConfig->data()->connectedOutputs().count() > 1 && !m_startingUp;

    doApplyConfig(Generator::self()->idealConfig(m_monitoredConfig->data()));

    if (showOsd) {
        qCDebug(KSCREEN_KDED) << "Getting ideal config from user via OSD...";
        auto action = m_osdManager->showActionSelector();
        connect(action, &KScreen::OsdAction::selected,
                this, &KScreenDaemon::applyOsdAction);
    } else {
        m_osdManager->hideOsd();
    }
}

void KScreenDaemon::configChanged()
{
    qCDebug(KSCREEN_KDED) << "Change detected";
    m_monitoredConfig->log();

    // Modes may have changed, fix-up current mode id
    bool changed = false;
    Q_FOREACH(const KScreen::OutputPtr &output, m_monitoredConfig->data()->outputs()) {
        if (output->isConnected() && output->isEnabled() && (output->currentMode().isNull() || (output->followPreferredMode() && output->currentModeId() != output->preferredModeId()))) {
            qCDebug(KSCREEN_KDED) << "Current mode was" << output->currentModeId() << ", setting preferred mode" << output->preferredModeId();
            output->setCurrentModeId(output->preferredModeId());
            changed = true;
        }
    }
    if (changed) {
        refreshConfig();
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

    if (m_monitoredConfig->canBeApplied()) {
        m_monitoredConfig->writeFile();
        m_monitoredConfig->log();
    } else {
        qCWarning(KSCREEN_KDED) << "Config does not have at least one screen enabled, WILL NOT save this config, this is not what user wants.";
        m_monitoredConfig->log();
    }
}

void KScreenDaemon::showOsd(const QString &icon, const QString &text)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QLatin1String("org.kde.plasmashell"),
        QLatin1String("/org/kde/osdService"),
        QLatin1String("org.kde.osdService"),
        QLatin1String("showText")
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
    if (m_monitoredConfig->data()->connectedOutputs().count() == 1) {
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
        if (auto openCfg = m_monitoredConfig->readOpenLidFile()) {
            doApplyConfig(std::move(openCfg));
        }
    }
}

void KScreenDaemon::disableLidOutput()
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

    qCDebug(KSCREEN_KDED) << "Lid closed, finding lid to disable";
    for (KScreen::OutputPtr &output : m_monitoredConfig->data()->outputs()) {
        if (output->type() == KScreen::Output::Panel) {
            if (output->isConnected() && output->isEnabled()) {
                // Save the current config with opened lid, just so that we know
                // how to restore it later
                m_monitoredConfig->writeOpenLidFile();
                disableOutput(output);
                refreshConfig();
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

        if (!m_monitoredConfig->fileExists()) {
            Q_EMIT unknownOutputConnected(output->name());
        }
    }
}

void KScreenDaemon::monitorConnectedChange()
{
    KScreen::OutputList outputs = m_monitoredConfig->data()->outputs();
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        connect(output.data(), &KScreen::Output::isConnectedChanged,
                this, &KScreenDaemon::outputConnectedChanged,
                Qt::UniqueConnection);
    }
    connect(m_monitoredConfig->data().data(), &KScreen::Config::outputAdded, this,
        [this] (const KScreen::OutputPtr output) {
            if (output->isConnected()) {
                m_changeCompressor->start();
            }
            connect(output.data(), &KScreen::Output::isConnectedChanged,
                    this, &KScreenDaemon::outputConnectedChanged,
                    Qt::UniqueConnection);
        }, Qt::UniqueConnection
    );
    connect(m_monitoredConfig->data().data(), &KScreen::Config::outputRemoved,
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

void KScreenDaemon::disableOutput(KScreen::OutputPtr &output)
{
    const QRect geom = output->geometry();
    qCDebug(KSCREEN_KDED) << "Laptop geometry:" << geom << output->pos() << (output->currentMode() ? output->currentMode()->size() : QSize());

    // Move all outputs right from the @p output to left
    for (KScreen::OutputPtr &otherOutput : m_monitoredConfig->data()->outputs()) {
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
