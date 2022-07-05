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

void OsdTest::setUseDBus(bool yesno)
{
    m_useDBus = yesno;
}


void OsdTest::showActionSelector()
{
    if (!m_useDBus) {
        getOsdManager()->showActionSelector();
        connect(getOsdManager(), &KScreen::OsdManager::selected, [](KScreen::OsdAction::Action action) {
            qCDebug(KSCREEN_KDED) << "Selected action:" << action;
            qApp->quit();
        });
    } else {
        QDBusMessage msg = QDBusMessage::createMethodCall(QLatin1String("org.kde.kscreen.osdService"),
                                                          QLatin1String("/org/kde/kscreen/osdService"),
                                                          QLatin1String("org.kde.kscreen.osdService"),
                                                          QLatin1String("showActionSelector"));
        QDBusConnection::sessionBus().asyncCall(msg);

        qCWarning(KSCREEN_KDED) << "Sent dbus message.";
        QTimer::singleShot(500, qApp, &QCoreApplication::quit);
    }
}

} // ns
