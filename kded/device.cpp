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

#include "device.h"
#include "kded/freedesktop_interface.h"

#include <QDBusServiceWatcher>

#include <kdebug.h>

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
 , m_nothingOnLidClose(false)
{
    m_freedesktop = new OrgFreedesktopDBusPropertiesInterface("org.freedesktop.UPower",
                                                              "/org/freedesktop/UPower",
                                                              QDBusConnection::systemBus());

    QDBusConnection::systemBus().connect("org.freedesktop.UPower", "/org/freedesktop/UPower", 
                                         "org.freedesktop.UPower", "Changed", this, SLOT(changed()));


    if (!QDBusConnection::systemBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
        QDBusServiceWatcher *watcher = new QDBusServiceWatcher("org.kde.Solid.PowerManagement", QDBusConnection::sessionBus());
        connect(watcher, SIGNAL(serviceRegistered(QString)), SLOT(init()));
        return;
    }

    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

Device::~Device()
{
    delete m_freedesktop;
}

void Device::init()
{
    fetchIsLaptop();
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

bool Device::nothingOnLidClose()
{
    return m_nothingOnLidClose;
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
        kDebug() << "Couldn't get if the device is a laptop: " << reply.error().message();
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
        kDebug() << "Couldn't get if the laptop has the lid closed: " << reply.error().message();
        return;
    }

    bool oldValue = m_isLidClosed;
    m_isLidClosed = reply.value().toBool();
    watcher->deleteLater();

    if (m_isReady && (oldValue != m_isLidClosed)) {
        Q_EMIT lidIsClosedChanged(m_isLidClosed, oldValue);
    }

    fetchIsDocked();
}

void Device::fetchIsDocked()
{
    fetchLidAction();
}

void Device::fetchLidAction()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                   "/org/kde/Solid/PowerManagement/Actions/HandleButtonEvents",
                                   "org.kde.Solid.PowerManagement.Actions.HandleButtonEvents",
                                   "lidAction");
    QDBusPendingReply<int> res = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(res);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(lidActionFetched(QDBusPendingCallWatcher*)));
}

void Device::lidActionFetched(QDBusPendingCallWatcher* watcher)
{
    const QDBusPendingReply<int> reply = *watcher;
    if (reply.isError()) {
        kDebug() << "Couldn't the lidAction" << reply.error().message();
        setReady();
        return;
    }

    m_nothingOnLidClose = reply.value() == 0;
    kDebug() << "NothingOnLidClose: " << m_nothingOnLidClose;

    watcher->deleteLater();

    setReady();
}


#include "device.moc"
