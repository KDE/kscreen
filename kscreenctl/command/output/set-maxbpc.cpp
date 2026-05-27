/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "BPC",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    if (arguments[0] == "automatic") {
        output->setMaxBitsPerColor(0);
    } else {
        const auto maxbpc = parseInt(arguments[0]);
        if (!maxbpc) {
            std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
            return 1;
        }

        if (maxbpc < 6 || maxbpc > 16 || *maxbpc % 2 != 0) {
            std::println(std::cerr, "Only whole numbers between 6 and 16 are allowed");
            return 1;
        }

        output->setMaxBitsPerColor(*maxbpc);
    }

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the maximum bits per color (possible values: 6, 8, 10, 12, 14, and 16)", "Misc")
