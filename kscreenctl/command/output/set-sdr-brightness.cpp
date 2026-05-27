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
    const auto brightness = parseInt(arguments[0]);
    if (!brightness) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    if (brightness < 50 || brightness > 10000) {
        std::println(std::cerr, "Allowed range for sdr-brightness is 50 to 10000");
        return 1;
    }

    output->setSdrBrightness(*brightness);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the SDR content brightness in the HDR mode (possible values: 0-10000)", "Color Management")
