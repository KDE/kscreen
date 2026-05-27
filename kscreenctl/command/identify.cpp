/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

static int run()
{
    const QDBusReply<void> reply = QDBusConnection::sessionBus().call(
        QDBusMessage::createMethodCall(u"org.kde.KWin"_s,
                                        u"/org/kde/KWin/Effect/OutputLocator1"_s,
                                        u"org.kde.KWin.Effect.OutputLocator1"_s,
                                        u"show"_s)
    );

    if (!reply.isValid()) {
        std::println(std::cerr, "org.kde.KWin.Effect.OutputLocator1.show failed: {}", reply.error().message().toStdString());
        return 1;
    }

    return 0;
}

COMMAND_WITH_DESCRIPTION(run,
                         "Identify available outputs",
                         "This command will show a popup on every screen with detailed output information. It can be used to determine the connector names to be used with other commands.")
