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

#include <QtCore/QTimer>

#include <kdebug.h>
#include <kdemacros.h>
#include <kaction.h>
#include <KLocalizedString>
#include <KActionCollection>
#include <KPluginFactory>

#include <kscreen/config.h>
#include <kscreen/configmonitor.h>

K_PLUGIN_FACTORY(KScreenDaemonFactory, registerPlugin<KScreenDaemon>();)
K_EXPORT_PLUGIN(KScreenDaemonFactory("kscreen", "kscreen"))

KScreenDaemon::KScreenDaemon(QObject* parent, const QList< QVariant >& )
 : KDEDModule(parent)
 , m_monitoredConfig(0)
 , m_iteration(0)
 , m_monitoring(false)
 , m_timer(new QTimer())
 , m_saveTimer(new QTimer())
 
{
    if (!KScreen::Config::loadBackend()) {
        return;
    }

    KActionCollection *coll = new KActionCollection(this);
    KAction* action = coll->addAction("display");
    action->setText(i18n("Switch Display" ));
    action->setGlobalShortcut(KShortcut(Qt::Key_Display));

    new KScreenAdaptor(this);

    connect(Device::self(), SIGNAL(lidIsClosedChanged(bool,bool)), SLOT(lidClosedChanged(bool)));

    m_timer->setInterval(300);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(applyGenericConfig()));

    m_saveTimer->setInterval(300);
    m_saveTimer->setSingleShot(true);
    connect(m_saveTimer, SIGNAL(timeout()), SLOT(saveCurrentConfig()));

    connect(action, SIGNAL(triggered(bool)), SLOT(displayButton()));
    connect(Generator::self(), SIGNAL(ready()), SLOT(init()));
    monitorConnectedChange();
}

KScreenDaemon::~KScreenDaemon()
{
    delete m_saveTimer;
    delete m_timer;

    Generator::destroy();
    Device::destroy();
}

void KScreenDaemon::init()
{
    disconnect(Generator::self(), SIGNAL(ready()), this, SLOT(init()));
    applyConfig();
}

void KScreenDaemon::applyConfig()
{
    kDebug() << "Applying config";
    if (Serializer::configExists()) {
        applyKnownConfig();
        return;
    }

    applyIdealConfig();
}

void KScreenDaemon::applyKnownConfig()
{
    kDebug() << "Applying known config";
    setMonitorForChanges(false);
    KScreen::Config* config = Serializer::config(Serializer::currentId());
    if (!KScreen::Config::canBeApplied(config)) {
        return applyIdealConfig();
    }

    KScreen::Config::setConfig(config);
    QMetaObject::invokeMethod(this, "scheduleMonitorChange", Qt::QueuedConnection);
}

void KScreenDaemon::applyIdealConfig()
{
    kDebug() << "Applying ideal config";
    setMonitorForChanges(true);
    KScreen::Config::setConfig(Generator::self()->idealConfig());
}

void KScreenDaemon::configChanged()
{
    kDebug() << "Change detected";
    // Reset timer, delay the writeback
    m_saveTimer->start();
}

void KScreenDaemon::saveCurrentConfig()
{
    kDebug() << "Saving current config";
    Serializer::saveConfig(KScreen::Config::current());
}

void KScreenDaemon::displayButton()
{
    kDebug() << "displayBtn triggered";
    if (m_timer->isActive()) {
        kDebug() << "Too fast cowboy";
        return;
    }

    m_timer->start();
}

void KScreenDaemon::resetDisplaySwitch()
{
    kDebug();
    m_iteration = 0;
}

void KScreenDaemon::applyGenericConfig()
{
    if (m_iteration == 5) {
        m_iteration = 0;
    }

    setMonitorForChanges(true);
    m_iteration++;
    kDebug() << "displayButton: " << m_iteration;
    KDebug::Block genericConfig("Applying display switch");
    KScreen::Config::setConfig(Generator::self()->displaySwitch(m_iteration));
}

void KScreenDaemon::lidClosedChanged(bool lidIsClosed)
{
//     KDebug::Block genericConfig(" Lid closed");
//     kDebug() << "Lid is closed:" << lidIsClosed;
//     //If the laptop is closed, use ideal config WITHOUT saving it
//     if (lidIsClosed) {
//         setMonitorForChanges(false);
//         KScreen::Config::setConfig(Generator::self()->idealConfig());
//         return;
//     }
//
//     //If the lid is open, restore config (or generate a new one if needed
//     applyConfig();
}

void KScreenDaemon::outputConnectedChanged()
{
    KScreen::Output *output = qobject_cast<KScreen::Output*>(sender());

    if (output->isConnected()) {
        Q_EMIT outputConnected(output->name());

        if (!Serializer::configExists()) {
            Q_EMIT unknownOutputConnected(output->name());
        }
    }
}

void KScreenDaemon::monitorConnectedChange()
{
    if (!m_monitoredConfig) {
        m_monitoredConfig = KScreen::Config::current();
        if (!m_monitoredConfig) {
            return;
        }

        KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig);
    }

    KScreen::OutputList outputs = m_monitoredConfig->outputs();
    Q_FOREACH(KScreen::Output* output, outputs) {
        connect(output, SIGNAL(isConnectedChanged()), SLOT(applyConfig()));
        connect(output, SIGNAL(isConnectedChanged()), SLOT(resetDisplaySwitch()));
        connect(output, SIGNAL(isConnectedChanged()), SLOT(outputConnectedChanged()));
    }
}

void KScreenDaemon::setMonitorForChanges(bool enabled)
{
    if (m_monitoring == enabled) {
        return;
    }
    kDebug() << "Monitor for changes: " << enabled;
    if (!m_monitoredConfig) {
        m_monitoredConfig = KScreen::Config::current();
        if (!m_monitoredConfig) {
            return;
        }
        KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig);
    }

    m_monitoring = enabled;

    KScreen::OutputList outputs = m_monitoredConfig->outputs();
    Q_FOREACH(KScreen::Output* output, outputs) {
        if (m_monitoring) {
            enableMonitor(output);
        } else {
            disableMonitor(output);
        }
    }
}

void KScreenDaemon::scheduleMonitorChange()
{
    QMetaObject::invokeMethod(this, "setMonitorForChanges", Qt::QueuedConnection, Q_ARG(bool, true));
}

void KScreenDaemon::enableMonitor(KScreen::Output* output)
{
    connect(output, SIGNAL(currentModeIdChanged()), SLOT(configChanged()));
    connect(output, SIGNAL(isEnabledChanged()), SLOT(configChanged()));
    connect(output, SIGNAL(isPrimaryChanged()), SLOT(configChanged()));
    connect(output, SIGNAL(outputChanged()), SLOT(configChanged()));
    connect(output, SIGNAL(clonesChanged()), SLOT(configChanged()));
    connect(output, SIGNAL(posChanged()), SLOT(configChanged()));
    connect(output, SIGNAL(rotationChanged()), SLOT(configChanged()));
}

void KScreenDaemon::disableMonitor(KScreen::Output* output)
{
    disconnect(output, SIGNAL(currentModeIdChanged()), this, SLOT(configChanged()));
    disconnect(output, SIGNAL(isEnabledChanged()), this, SLOT(configChanged()));
    disconnect(output, SIGNAL(isPrimaryChanged()), this, SLOT(configChanged()));
    disconnect(output, SIGNAL(outputChanged()), this, SLOT(configChanged()));
    disconnect(output, SIGNAL(clonesChanged()), this, SLOT(configChanged()));
    disconnect(output, SIGNAL(posChanged()), this, SLOT(configChanged()));
    disconnect(output, SIGNAL(rotationChanged()), this, SLOT(configChanged()));
}

#include "daemon.moc"
