/*
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "osdmanager.h"
#include "osd.h"
#include "kscreen_daemon_debug.h"

#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>

#include <QDBusConnection>

#include <QQmlEngine>

namespace KScreen {

class OsdActionImpl : public OsdAction
{
    Q_OBJECT
public:
    OsdActionImpl(QObject *parent = nullptr)
        : OsdAction(parent)
    {}

    void setOsd(Osd *osd) {
        connect(osd, &Osd::osdActionSelected,
                this, [this](Action action) {
                    Q_EMIT selected(action);
                    deleteLater();
                });
    }
};

OsdManager::OsdManager(QObject *parent)
    : QObject(parent)
    , m_cleanupTimer(new QTimer(this))
{
    qmlRegisterSingletonType<KScreen::OsdAction>("org.kde.KScreen", 1, 0, "OsdAction", [](QQmlEngine *, QJSEngine *) -> QObject* {
        return new KScreen::OsdAction();
    });

    // free up memory when the osd hasn't been used for more than 1 minute
    m_cleanupTimer->setInterval(60000);
    m_cleanupTimer->setSingleShot(true);
    connect(m_cleanupTimer, &QTimer::timeout, this, [this]() {
        hideOsd();
    });
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.kscreen.osdService"));
    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/kde/kscreen/osdService"), this, QDBusConnection::ExportAllSlots)) {
        qCWarning(KSCREEN_KDED) << "Failed to registerObject";
    }
}

void OsdManager::hideOsd()
{
    qDeleteAll(m_osds);
    m_osds.clear();
}

OsdManager::~OsdManager()
{
}

void OsdManager::showOutputIdentifiers()
{
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, &OsdManager::slotIdentifyOutputs);
}

void OsdManager::slotIdentifyOutputs(KScreen::ConfigOperation *op)
{
    if (op->hasError()) {
        return;
    }

    const KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();

    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        if (!output->isConnected() || !output->isEnabled() || !output->currentMode()) {
            continue;
        }
        auto osd = m_osds.value(output->name());
        if (!osd) {
            osd = new KScreen::Osd(output, this);
            m_osds.insert(output->name(), osd);
        }
        osd->showOutputIdentifier(output);
    }
    m_cleanupTimer->start();
}

void OsdManager::showOsd(const QString& icon, const QString& text)
{
    hideOsd();

    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
        this, [this, icon, text] (KScreen::ConfigOperation *op) {
            if (op->hasError()) {
                return;
            }

            const KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();

            Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
                if (!output->isConnected() || !output->isEnabled() || !output->currentMode()) {
                    continue;
                }
                auto osd = m_osds.value(output->name());
                if (!osd) {
                    osd = new KScreen::Osd(output, this);
                    m_osds.insert(output->name(), osd);
                }
                osd->showGenericOsd(icon, text);
            }
            m_cleanupTimer->start();
        }
    );
}

OsdAction *OsdManager::showActionSelector()
{
    hideOsd();

    OsdActionImpl *action = new OsdActionImpl(this);
    connect(action, &OsdActionImpl::selected,
            this, [this]() {
                for (auto osd : qAsConst(m_osds)) {
                    osd->hideOsd();
                }
            });
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
        this, [this, action](const KScreen::ConfigOperation *op) {
            if (op->hasError()) {
                qCWarning(KSCREEN_KDED) << op->errorString();
                return;
            }

            // Show selector on all enabled screens
            const auto outputs = op->config()->outputs();
            KScreen::OutputPtr osdOutput;
            for (const auto &output : outputs) {
                if (!output->isConnected() || !output->isEnabled() || !output->currentMode()) {
                    continue;
                }

                // Prefer laptop screen
                if (output->type() == KScreen::Output::Panel) {
                    osdOutput = output;
                    break;
                }

                // Fallback to primary
                if (output->isPrimary()) {
                    osdOutput = output;
                    break;
                }
            }
            // no laptop or primary screen, just take the first usable one
            if (!osdOutput) {
                for (const auto &output : outputs) {
                    if (output->isConnected() && output->isEnabled() && output->currentMode()) {
                        osdOutput = output;
                        break;
                    }
                }
            }

            if (!osdOutput) {
                // huh!?
                return;
            }

            KScreen::Osd* osd = nullptr;
            if (m_osds.contains(osdOutput->name())) {
                osd = m_osds.value(osdOutput->name());
            } else {
                osd = new KScreen::Osd(osdOutput, this);
                m_osds.insert(osdOutput->name(), osd);
            }
            action->setOsd(osd);
            osd->showActionSelector();
            m_cleanupTimer->start();
        }
    );

    return action;
}


}

#include "osdmanager.moc"
