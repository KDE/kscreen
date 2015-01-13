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

#include "daemon.h"
#include "serializer.h"
#include "generator.h"
#include "device.h"
#include "kscreenadaptor.h"
#include "debug.h"

#include <QTimer>
#include <QAction>
#include <QShortcut>
#include <QLoggingCategory>

#include <KLocalizedString>
#include <KActionCollection>
#include <KPluginFactory>
#include <KGlobalAccel>

#include <kscreen/config.h>
#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

K_PLUGIN_FACTORY(KScreenDaemonFactory, registerPlugin<KScreenDaemon>();)

KScreenDaemon::KScreenDaemon(QObject* parent, const QList< QVariant >& )
 : KDEDModule(parent)
 , m_monitoredConfig(0)
 , m_iteration(0)
 , m_monitoring(false)
 , m_changeCompressor(new QTimer())
 , m_buttonTimer(new QTimer())
 , m_saveTimer(new QTimer())
 , m_lidClosedTimer(new QTimer())
 
{
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
    delete m_changeCompressor;
    delete m_saveTimer;
    delete m_buttonTimer;
    delete m_lidClosedTimer;

    Generator::destroy();
    Device::destroy();
}

void KScreenDaemon::init()
{
    KActionCollection *coll = new KActionCollection(this);
    QAction* action = coll->addAction(QStringLiteral("display"));
    action->setText(i18n("Switch Display" ));
    KGlobalAccel::self()->setGlobalShortcut(action, Qt::Key_Display);
    connect(action, &QAction::triggered, [&](bool) { displayButton(); });

    new KScreenAdaptor(this);

    m_buttonTimer->setInterval(300);
    m_buttonTimer->setSingleShot(true);
    connect(m_buttonTimer, &QTimer::timeout, this, &KScreenDaemon::applyGenericConfig);

    m_saveTimer->setInterval(300);
    m_saveTimer->setSingleShot(true);
    connect(m_saveTimer, &QTimer::timeout, this, &KScreenDaemon::saveCurrentConfig);

    m_changeCompressor->setInterval(10);
    m_changeCompressor->setSingleShot(true);
    connect(m_changeCompressor, &QTimer::timeout, this, &KScreenDaemon::applyConfig);

    m_lidClosedTimer->setInterval(2000);
    m_lidClosedTimer->setSingleShot(true);
    connect(m_lidClosedTimer, &QTimer::timeout, this, &KScreenDaemon::lidClosedTimeout);


    connect(Device::self(), &Device::lidClosedChanged, this, &KScreenDaemon::lidClosedChanged);
    connect(Device::self(), &Device::resumingFromSuspend,
            [&]() {
                qCDebug(KSCREEN_KDED) << "Resumed from suspend, checking for screen changes";
                // We don't care about the result, we just want to force the backend
                // to query XRandR so that it will detect possible changes that happened
                // while the computer was suspended, and will emit the change events.
                new KScreen::GetConfigOperation(KScreen::GetConfigOperation::NoEDID, this);
            });
    connect(Device::self(), &Device::aboutToSuspend,
            m_lidClosedTimer, &QTimer::stop);


    connect(Generator::self(), &Generator::ready,
            this, &KScreenDaemon::applyConfig);

    Generator::self()->setCurrentConfig(m_monitoredConfig);
    monitorConnectedChange();
}

void KScreenDaemon::doApplyConfig(const KScreen::ConfigPtr& config)
{
    qCDebug(KSCREEN_KDED) << "doApplyConfig()";
    setMonitorForChanges(false);

    connect(new KScreen::SetConfigOperation(config), &KScreen::SetConfigOperation::finished,
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

    KScreen::ConfigPtr config = Serializer::config(m_monitoredConfig, configId);
    if (!KScreen::Config::canBeApplied(config)) {
        return applyIdealConfig();
    }

    doApplyConfig(config);
}

void KScreenDaemon::applyIdealConfig()
{
    qCDebug(KSCREEN_KDED) << "Applying ideal config";
    doApplyConfig(Generator::self()->idealConfig(m_monitoredConfig));
}

void KScreenDaemon::configChanged()
{
    qCDebug(KSCREEN_KDED) << "Change detected";
    // Reset timer, delay the writeback
    m_saveTimer->start();
}

void KScreenDaemon::saveCurrentConfig()
{
    qCDebug(KSCREEN_KDED) << "Saving current config to file";
    Serializer::saveConfig(m_monitoredConfig, Serializer::configId(m_monitoredConfig));
}

void KScreenDaemon::displayButton()
{
    qCDebug(KSCREEN_KDED) << "displayBtn triggered";
    if (m_buttonTimer->isActive()) {
        qCDebug(KSCREEN_KDED) << "Too fast, cowboy";
        return;
    }

    m_buttonTimer->start();
}

void KScreenDaemon::resetDisplaySwitch()
{
    qCDebug(KSCREEN_KDED) << "resetDisplaySwitch()";
    m_iteration = 0;
}

void KScreenDaemon::applyGenericConfig()
{
    if (m_iteration == 5) {
        m_iteration = 0;
    }

    m_iteration++;
    qCDebug(KSCREEN_KDED) << "displayButton: " << m_iteration;

    doApplyConfig(Generator::self()->displaySwitch(m_iteration));
}

void KScreenDaemon::lidClosedChanged(bool lidIsClosed)
{
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
            /*
            KScreen::OutputPtr closedLidOutput = findEmbeddedOutput(m_monitoredConfig);
            if (!closedLidOutput) {
                // WTF?
                return;
            }
            const KScreen::OutputPtr openedLidOutput = openedConfig->output(closedLidOutput->id());
            if (!openedLidOutput) {
                // WTF?
                return;
            }

            closedLidOutput->setEnabled(true);
            closedLidOutput->setPos(openedLidOutput->pos());
            closedLidOutput->setCurrentModeId(openedLidOutput->currentModeId());
            closedLidOutput->setPrimary(openedLidOutput->isPrimary());
            closedLidOutput->setRotation(openedLidOutput->rotation());

            const KScreen::ModePtr newMode = closedLidOutput->currentMode();
            if (!newMode) {
                // WTF?
                return;
            }
            const QRect closedGeometry = closedLidOutput->geometry();
            for (KScreen::OutputPtr &output : m_monitoredConfig->outputs()) {
                if (output == closedLidOutput) {
                    continue;
                }

                const QRect geom = output->geometry();

                if (geom.left() >= closedGeometry.left() &&
                    geom.top() >= closedGeometry.top() && geom.top() <= closedGeometry.bottom()) {
                    output->pos().rx() += closedGeometry.width();
                }
            }
            */
        }
    }
}

void KScreenDaemon::lidClosedTimeout()
{
    // Make sure nothing has changed in the past 2 seconds.. :-)
    if (!Device::self()->isLidClosed()) {
        return;
    }

    // If we are here, it means that closing the lid did not result in suspend
    // action.
    // FIXME: This could be simply because the suspend took longer than m_lidClosedTimer
    // timeout. Ideally we need to be able to look into PowerDevil config to see
    // what's the configured action for lid events, but there's no API to do that
    // and I'm no parsing PowerDevil's configs...

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

    resetDisplaySwitch();

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

KScreen::OutputPtr KScreenDaemon::findEmbeddedOutput(const KScreen::ConfigPtr &config)
{
    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        if (output->type() == KScreen::Output::Panel) {
            return output;
        }
    }

    return KScreen::OutputPtr();
}



#include "daemon.moc"
