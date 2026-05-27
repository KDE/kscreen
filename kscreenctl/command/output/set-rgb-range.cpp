/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "RANGE",
    &arguments
)

static std::optional<KScreen::Output::RgbRange> parseRgbRange(const QString &input)
{
    if (input == "automatic") {
        return KScreen::Output::RgbRange::Automatic;
    } else if (input == "full") {
        return KScreen::Output::RgbRange::Full;
    } else if (input == "limited") {
        return KScreen::Output::RgbRange::Limited;
    }
    return std::nullopt;
}

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto range = parseRgbRange(arguments[0]);
    if (!range) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    output->setRgbRange(*range);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the RGB range (possible values: full, limited, or automatic)", "Color Management")
