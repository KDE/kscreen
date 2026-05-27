/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "DIMMING",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto dimming = parsePercents(arguments[0]);
    if (!dimming) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    if (dimming < 0 || dimming > 1) {
        std::println(std::cerr, "Allowed range for dimming is 0% to 100%");
        return 1;
    }

    output->setDimming(*dimming);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the dimming level (possible values: 0%-100%)", "Brightness")
