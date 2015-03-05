/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2015 by Daniel Vrátil <dvratil@redhat.com>                         *
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

#include "device.h"
#include "debug.h"
#include "kded/freedesktop_interface.h"

#include <QLoggingCategory>

Device* Device::m_instance = 0;

Device* Device::self()
{
    if (!Device::m_instance) {
        m_instance = new Device();
    }

    return m_instance;
}

void Device::destroy()
{
    delete m_instance;
    m_instance = 0;
}

Device::Device(QObject* parent) 
   : QObject(parent)
   , m_isReady(false)
   , m_isLaptop(false)
   , m_isLidClosed(false)
   , m_isDocked(false)
{
    m_freedesktop = new OrgFreedesktopDBusPropertiesInterface("org.freedesktop.UPower",
                                                              "/org/freedesktop/UPower",
                                                              QDBusConnection::systemBus(),
                                                              this);
    if (!m_freedesktop->isValid()) {
        qCWarning(KSCREEN_KDED) << "UPower not available, lid detection won't work";
        qCDebug(KSCREEN_KDED) << m_freedesktop->lastError().message();
    } else {
        QDBusConnection::systemBus().connect(QLatin1String("org.freedesktop.UPower"),
                                             QLatin1String("/org/freedesktop/UPower"),
                                             QLatin1String("org.freedesktop.DBus.Properties"),
                                             QLatin1String("PropertiesChanged"),
                                             this, SLOT(changed()));
        fetchIsLaptop();
    }

    m_suspendSession = new QDBusInterface(QLatin1String("org.kde.Solid.PowerManagement"),
                                          QLatin1String("/org/kde/Solid/PowerManagement/Actions/SuspendSession"),
                                          QLatin1String("org.kde.Solid.PowerManagement.Actions.SuspendSession"),
                                          QDBusConnection::sessionBus(),
                                          this);
    if (m_suspendSession->isValid()) {
        connect(m_suspendSession, SIGNAL(resumingFromSuspend()),
                this, SIGNAL(resumingFromSuspend()));
        connect(m_suspendSession, SIGNAL(aboutToSuspend()),
                this, SIGNAL(aboutToSuspend()));
    } else {
        qCWarning(KSCREEN_KDED) << "PowerDevil SuspendSession action not available!";
        qCDebug(KSCREEN_KDED) << m_suspendSession->lastError().message();
    }

    fetchIsLaptop();
}

Device::~Device()
{
}

void Device::changed()
{
    fetchLidIsClosed();
}

void Device::setReady()
{
    if (m_isReady) {
        return;
    }

    m_isReady = true;
    Q_EMIT ready();
}

bool Device::isReady()
{
    return m_isReady;
}

bool Device::isLaptop()
{
    return m_isLaptop;
}

bool Device::isLidClosed()
{
    return m_isLidClosed;
}

bool Device::isDocked()
{
    return m_isDocked;
}

void Device::fetchIsLaptop()
{
    QDBusPendingReply<QVariant> res = m_freedesktop->Get("org.freedesktop.UPower", "LidIsPresent");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(res);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(isLaptopFetched(QDBusPendingCallWatcher*)));
}

void Device::isLaptopFetched(QDBusPendingCallWatcher* watcher)
{
    const QDBusPendingReply<QVariant> reply = *watcher;
    if (reply.isError()) {
        qCDebug(KSCREEN_KDED) << "Couldn't get if the device is a laptop: " << reply.error().message();
        return;
    }

    m_isLaptop = reply.value().toBool();
    watcher->deleteLater();

    if (!m_isLaptop) {
        setReady();
        return;
    }

    fetchLidIsClosed();
}

void Device::fetchLidIsClosed()
{
    QDBusPendingReply<QVariant> res = m_freedesktop->Get("org.freedesktop.UPower", "LidIsClosed");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(res);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(isLidClosedFetched(QDBusPendingCallWatcher*)));
}

void Device::isLidClosedFetched(QDBusPendingCallWatcher* watcher)
{
    const QDBusPendingReply<QVariant> reply = *watcher;
    if (reply.isError()) {
        qCDebug(KSCREEN_KDED) << "Couldn't get if the laptop has the lid closed: " << reply.error().message();
        return;
    }

    if (reply.argumentAt<0>() != m_isLidClosed) {
        m_isLidClosed = reply.value().toBool();
        if (m_isReady) {
            Q_EMIT lidClosedChanged(m_isLidClosed);;
        }
    }
    watcher->deleteLater();

    fetchIsDocked();
}

void Device::fetchIsDocked()
{
    setReady();
}
