/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "TRADEOFF",
    &arguments
)

static std::optional<KScreen::Output::ColorPowerTradeoff> parseColorPowerTradeoff(const QString &input)
{
    if (input == "prefer-accuracy") {
        return KScreen::Output::ColorPowerTradeoff::PreferAccuracy;
    } else if (input == "prefer-efficiency") {
        return KScreen::Output::ColorPowerTradeoff::PreferEfficiency;
    }
    return std::nullopt;
}

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto tradeoff = parseColorPowerTradeoff(arguments[0]);
    if (!tradeoff) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    output->setColorPowerPreference(*tradeoff);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set color power tradeoff (prefer-accuracy or prefer-efficiency)", "Color Management")
