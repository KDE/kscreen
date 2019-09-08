/*************************************************************************************
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>                                  *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#include "osdtest.h"
#include "../../kded/osdmanager.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(KSCREEN_KDED, "kscreen.kded")

namespace KScreen {
OsdTest::OsdTest(QObject *parent)
    : QObject(parent)
{
}

OsdTest::~OsdTest()
{
}

OsdManager *OsdTest::getOsdManager()
{
    if (m_osdManager)
        return m_osdManager;
    m_osdManager = new OsdManager(this);
    return m_osdManager;
}

void OsdTest::showOutputIdentifiers()
{
    if (!m_useDBus) {
        QTimer::singleShot(5500, qApp, &QCoreApplication::quit);
        getOsdManager()->showOutputIdentifiers();
    } else {
        QDBusMessage msg = QDBusMessage::createMethodCall(
            QLatin1String("org.kde.kscreen.osdService"),
            QLatin1String("/org/kde/kscreen/osdService"),
            QLatin1String("org.kde.kscreen.osdService"),
            QLatin1String("showOutputIdentifiers")
        );
        //msg << icon << text;
        QDBusConnection::sessionBus().asyncCall(msg);

        qCWarning(KSCREEN_KDED) << "Sent dbus message.";
        QTimer::singleShot(500, qApp, &QCoreApplication::quit);
    }
}


void OsdTest::setUseDBus(bool yesno)
{
    m_useDBus = yesno;
}

void OsdTest::showGenericOsd(const QString& icon, const QString& message)
{
    if (!m_useDBus) {
        QTimer::singleShot(5500, qApp, &QCoreApplication::quit);
        getOsdManager()->showOsd(!icon.isEmpty() ? icon : QStringLiteral("preferences-desktop-display-randr"),
                                             !message.isEmpty() ? message : QStringLiteral("On-Screen-Display"));
    } else {
        qCWarning(KSCREEN_KDED) << "Implement me.";
        QTimer::singleShot(100, qApp, &QCoreApplication::quit);
    }
}

void OsdTest::showActionSelector()
{
    if (!m_useDBus) {
        auto action = getOsdManager()->showActionSelector();
        connect(action, &KScreen::OsdAction::selected,
                [](KScreen::OsdAction::Action action) {
                    qCDebug(KSCREEN_KDED) << "Selected action:" << action;
                    qApp->quit();
                });
    } else {
        qCWarning(KSCREEN_KDED) << "Implement me.";
        QTimer::singleShot(100, qApp, &QCoreApplication::quit);
    }
}

} // ns
