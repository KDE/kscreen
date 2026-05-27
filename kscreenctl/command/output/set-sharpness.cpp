/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "SHARPNESS",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto sharpness = parseInt(arguments[0]);
    if (!sharpness) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    if (sharpness < 0 || sharpness > 100) {
        std::println(std::cerr, "Allowed range for sharpness is 0 to 100");
        return 1;
    }

    output->setSharpness(*sharpness / 100.0);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the sharpness (possible values: 0-100)", "Misc")
