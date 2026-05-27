/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command.h"

#include <KScreenDpms/Dpms>

#include <QGuiApplication>
#include <QTimer>

static int run()
{
    auto dpms = new KScreen::Dpms();
    QObject::connect(dpms, &KScreen::Dpms::hasPendingChangesChanged, qGuiApp, [](bool hasChanges) {
        if (!hasChanges) {
            // We need to hit the event loop, otherwise .quit() hangs
            QTimer::singleShot(0, qApp->quit);
        }
    });

    dpms->switchMode(KScreen::Dpms::Off, qGuiApp->screens());

    return qGuiApp->exec();
}

COMMAND(run, "Turn all outputs off")

COMMAND_EXAMPLE("Turn all outputs off", "")
