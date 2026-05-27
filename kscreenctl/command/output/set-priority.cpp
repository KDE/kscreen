/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "PRIORITY",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto priority = parseInt(arguments[0]);
    if (!priority) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    if (priority < 1 || priority > 100) {
        std::println(std::cerr, "Allowed range for priority is 1 to 100");
        return 1;
    }

    config->setOutputPriority(output, *priority);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the output priority (possible values: 1-100)", "Basic")
