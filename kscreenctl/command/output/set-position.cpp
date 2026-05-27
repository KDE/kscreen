/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "X Y",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto x = parseDouble(arguments[0]);
    if (!x) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    const auto y = parseDouble(arguments[1]);
    if (!y) {
        std::println(std::cerr, "Invalid argument: {}", arguments[1].toStdString());
        return 1;
    }

    output->setPos(QPoint(*x, *y));
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the position", "Positioning")
