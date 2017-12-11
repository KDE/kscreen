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

namespace KScreen {

OsdManager* OsdManager::s_instance = nullptr;

OsdManager::OsdManager(QObject *parent)
    : QObject(parent)
    , m_cleanupTimer(new QTimer(this))
{
    // free up memory when the osd hasn't been used for more than 1 minute
    m_cleanupTimer->setInterval(60000);
    m_cleanupTimer->setSingleShot(true);
    connect(m_cleanupTimer, &QTimer::timeout, this, [this]() {
        qDeleteAll(m_osds);
        m_osds.clear();
    });
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.kscreen.osdService"));
    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/kde/kscreen/osdService"), this, QDBusConnection::ExportAllSlots)) {
        qCWarning(KSCREEN_KDED) << "Failed to registerObject";
    }
}

OsdManager::~OsdManager()
{
}

OsdManager* OsdManager::self()
{
    if (!OsdManager::s_instance) {
        s_instance = new OsdManager();
    }
    return s_instance;
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
        KScreen::Osd* osd = nullptr;
        if (m_osds.keys().contains(output->name())) {
            osd = m_osds.value(output->name());
        } else {
            osd = new KScreen::Osd(output, this);
            m_osds.insert(output->name(), osd);
        }
        osd->showOutputIdentifier(output);
    }
    m_cleanupTimer->start();
}

void OsdManager::showOsd(const QString& icon, const QString& text)
{
    qDeleteAll(m_osds);
    m_osds.clear();
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
                KScreen::Osd* osd = nullptr;
                if (m_osds.keys().contains(output->name())) {
                    osd = m_osds.value(output->name());
                } else {
                    osd = new KScreen::Osd(output, this);
                    m_osds.insert(output->name(), osd);
                }
                osd->showGenericOsd(icon, text);
            }
            m_cleanupTimer->start();
        }
    );
}


}
