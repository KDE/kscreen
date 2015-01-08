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

#include <QtCore/QTimer>
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
K_EXPORT_PLUGIN(KScreenDaemonFactory("kscreen", "kscreen"))

KScreenDaemon::KScreenDaemon(QObject* parent, const QList< QVariant >& )
 : KDEDModule(parent)
 , m_monitoredConfig(0)
 , m_iteration(0)
 , m_monitoring(false)
 , m_changeCompressor(new QTimer())
 , m_buttonTimer(new QTimer())
 , m_saveTimer(new QTimer())
 
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

    Generator::destroy();
    Device::destroy();
}

void KScreenDaemon::init()
{
    KActionCollection *coll = new KActionCollection(this);
    QAction* action = coll->addAction(QStringLiteral("display"));
    action->setText(i18n("Switch Display" ));
    KGlobalAccel::self()->setShortcut(action, QList<QKeySequence>() << QKeySequence(Qt::Key_Display));
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


    connect(Device::self(), &Device::lidClosedChanged, this, &KScreenDaemon::lidClosedChanged);
    connect(Device::self(), &Device::resumingFromSuspend,
            [&]() {
                qCDebug(KSCREEN_KDED) << "Resumed from suspend, checking for screen changes";
                // We don't care about the result, we just want to force the backend
                // to query XRandR so that it will detect possible changes that happened
                // while the computer was suspended, and will emit the change events.
                new KScreen::GetConfigOperation(KScreen::GetConfigOperation::NoEDID, this);
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
    Serializer::saveConfig(m_monitoredConfig);
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
    Q_UNUSED(lidIsClosed);
//     KDebug::Block genericConfig(" Lid closed");
//     qDebug() << "Lid is closed:" << lidIsClosed;
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

    const KScreen::OutputList outputs = m_monitoredConfig->outputs();
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (m_monitoring) {
            enableMonitor(output);
        } else {
            disableMonitor(output);
        }
    }
}

void KScreenDaemon::enableMonitor(const KScreen::OutputPtr &output)
{
    connect(output.data(), &KScreen::Output::currentModeIdChanged,
            this, &KScreenDaemon::configChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            this, &KScreenDaemon::configChanged);
    connect(output.data(), &KScreen::Output::isPrimaryChanged,
            this, &KScreenDaemon::configChanged);
    connect(output.data(), &KScreen::Output::outputChanged,
            this, &KScreenDaemon::configChanged);
    connect(output.data(), &KScreen::Output::clonesChanged,
            this, &KScreenDaemon::configChanged);
    connect(output.data(), &KScreen::Output::posChanged,
            this, &KScreenDaemon::configChanged);
    connect(output.data(), &KScreen::Output::rotationChanged,
            this, &KScreenDaemon::configChanged);
}

void KScreenDaemon::disableMonitor(const KScreen::OutputPtr &output)
{
    disconnect(output.data(), &KScreen::Output::currentModeIdChanged,
               this, &KScreenDaemon::configChanged);
    disconnect(output.data(), &KScreen::Output::isEnabledChanged,
               this, &KScreenDaemon::configChanged);
    disconnect(output.data(), &KScreen::Output::isPrimaryChanged,
               this, &KScreenDaemon::configChanged);
    disconnect(output.data(), &KScreen::Output::outputChanged,
               this, &KScreenDaemon::configChanged);
    disconnect(output.data(), &KScreen::Output::clonesChanged,
               this, &KScreenDaemon::configChanged);
    disconnect(output.data(), &KScreen::Output::posChanged,
               this, &KScreenDaemon::configChanged);
    disconnect(output.data(), &KScreen::Output::rotationChanged,
               this, &KScreenDaemon::configChanged);
}

#include "daemon.moc"
