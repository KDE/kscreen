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
    const auto mode = modeByQuery(output, arguments[0]);
    if (!mode) {
        std::println(std::cerr, "Unknown mode: {}", arguments[0].toStdString());
        return 1;
    }

    const QSize previousSize = output->explicitLogicalSizeInt();
    output->setCurrentModeId(mode->id());
    output->setSize(mode->size());
    output->setExplicitLogicalSize(config->logicalSizeForOutputInt(*output));
    snapOutputAfterResize(output, previousSize, config);

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the current mode", "Modes")

OUTPUT_COMMAND_EXAMPLE("Set the 1920x1080@60 mode as the current mode. Note that you can also address modes by their IDs in `list-modes` or without the refresh rate, e.g. 1920x1080", "DP-1 set-mode 1920x1080@60")
