/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "SCALE",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto scale = parsePercents(arguments[0]);
    if (!scale) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    const double minScale = 0.5;
    if (scale < minScale) {
        std::println(std::cerr, "The scale factor should be at least {}%", std::round(minScale * 100));
        return 1;
    }

    const double maxScale = 10.0;
    if (scale > maxScale) {
        std::println(std::cerr, "The scale factor cannot be greater than {}%", std::round(maxScale * 100));
        return 1;
    }

    const QSize previousSize = output->explicitLogicalSizeInt();
    output->setScale(*scale);
    output->setExplicitLogicalSize(config->logicalSizeForOutputInt(*output));
    snapOutputAfterResize(output, previousSize, config);

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the output scale", "Basic")

OUTPUT_COMMAND_EXAMPLE("Change the scale factor of output DP-1 to 200%",
                       "DP-1 set-scale 200%")
