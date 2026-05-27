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
    const auto enabled = parseBool(arguments[0]);
    if (!enabled.has_value()) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    output->setEnabled(*enabled);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Enable or disable the output", "Basic")

OUTPUT_COMMAND_EXAMPLE("Disable the current output",
                       "active-output set-enabled false")
