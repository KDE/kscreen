/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "BOOL",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto hdrEnabled = parseBool(arguments[0]);
    if (!hdrEnabled.has_value()) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    output->setHdrEnabled(*hdrEnabled);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Turn on or turn off the HDR mode", "Color Management")
