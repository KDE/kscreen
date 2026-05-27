/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "MODE",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto index = parseInt(arguments[0]);
    if (!index) {
        std::println(std::cerr, "Invalid mode index: {}", arguments[0].toStdString());
        return 1;
    }

    auto modes = output->customModes();
    if (index < 0 || index >= modes.size()) {
        std::println(std::cerr, "Invalid mode index: {}", *index);
        return 1;
    }

    modes.removeAt(*index);
    output->setCustomModes(modes);

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Remove a custom mode. MODE is the index of the mode in list-custom-modes", "Modes")

OUTPUT_COMMAND_EXAMPLE("Remove the first custom mode. Note that the command takes the index of the custom mode in `list-custom-modes`",
                       "active-output remove-custom-mode 0")
