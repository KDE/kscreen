/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "LEVEL",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto level = parseInt(arguments[0]);
    if (!level) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    if (level < 0 || level > 4) {
        std::println(std::cerr, "Allowed values for abm level are 0, 1, 2, 3, 4");
        return 1;
    }

    output->setAbmLevel(*level);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Change the adaptive backlight management level (possible values: 0-4)", "Misc")
