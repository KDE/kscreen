/*
SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "osdtest.h"
#include "../../kded/osdmanager.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(KSCREEN_KDED, "kscreen.kded")

namespace KScreen
{
OsdTest::OsdTest(QObject *parent)
    : QObject(parent)
{
}

OsdTest::~OsdTest()
{
}

OsdManager *OsdTest::getOsdManager()
{
    if (m_osdManager) {
        return m_osdManager;
    }
    m_osdManager = new OsdManager(this);
    return m_osdManager;
}

void OsdTest::showOutputIdentifiers()
{
    if (!m_useDBus) {
        QTimer::singleShot(5500, qApp, &QCoreApplication::quit);
        getOsdManager()->showOutputIdentifiers();
    } else {
        QDBusMessage msg = QDBusMessage::createMethodCall(QLatin1String("org.kde.kscreen.osdService"),
                                                          QLatin1String("/org/kde/kscreen/osdService"),
                                                          QLatin1String("org.kde.kscreen.osdService"),
                                                          QLatin1String("showOutputIdentifiers"));
        // msg << icon << text;
        QDBusConnection::sessionBus().asyncCall(msg);

        qCWarning(KSCREEN_KDED) << "Sent dbus message.";
        QTimer::singleShot(500, qApp, &QCoreApplication::quit);
    }
}

void OsdTest::setUseDBus(bool yesno)
{
    m_useDBus = yesno;
}

void OsdTest::showGenericOsd(const QString &icon, const QString &message)
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
        connect(action, &KScreen::OsdAction::selected, [](KScreen::OsdAction::Action action) {
            qCDebug(KSCREEN_KDED) << "Selected action:" << action;
            qApp->quit();
        });
    } else {
        qCWarning(KSCREEN_KDED) << "Implement me.";
        QTimer::singleShot(100, qApp, &QCoreApplication::quit);
    }
}

} // ns
