/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "BRIGHTNESS",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto brightness = parsePercents(arguments[0]);
    if (!brightness) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    if (brightness < 0 || brightness > 1) {
        std::println(std::cerr, "Allowed range for brightness is 0% to 100%");
        return 1;
    }

    output->setBrightness(*brightness);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set screen brightness (possible values: 0%-100%)", "Brightness")

OUTPUT_COMMAND_EXAMPLE("Change the brightness of the current output to 50%", "active-output set-brightness 50%")
