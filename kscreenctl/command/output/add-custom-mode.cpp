/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static bool reducedBlanking = false;
OUTPUT_COMMAND_TOGGLE_OPTION(
    "reduced-blanking",
    &reducedBlanking,
    "Add a custom mode with reduced blanking"
)

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "MODE",
    &arguments
)

static std::optional<KScreen::ModeInfo> tryBasicModeline()
{
    if (arguments.isEmpty()) {
        return std::nullopt;
    }

    const QString modeline = arguments[0];

    const int xIndex = modeline.indexOf('x');
    if (xIndex == -1) {
        return std::nullopt;
    }

    const int atIndex = modeline.indexOf('@', xIndex);
    if (atIndex == -1) {
        return std::nullopt;
    }

    const std::optional<int> width = parseInt(modeline.left(xIndex));
    if (!width) {
        return std::nullopt;
    }

    const std::optional<int> height = parseInt(modeline.mid(xIndex + 1, atIndex - xIndex - 1));
    if (!height) {
        return std::nullopt;
    }

    QString refreshRatePart = modeline.mid(atIndex + 1);
    if (refreshRatePart.endsWith("Hz")) {
        refreshRatePart.chop(2);
    }

    const std::optional<double> refreshRate = parseDouble(refreshRatePart);
    if (!refreshRate) {
        return std::nullopt;
    }

    KScreen::ModeInfo::Flags flags = KScreen::ModeInfo::Flag::Custom;
    if (reducedBlanking) {
        flags |= KScreen::ModeInfo::Flag::ReducedBlanking;
    }

    return KScreen::ModeInfo {
        .size = QSize(*width, *height),
        .refreshRate = float(*refreshRate),
        .flags = flags,
    };
}

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto modeInfo = tryBasicModeline();
    if (!modeInfo) {
        std::println(std::cerr, "Invalid custom mode: {}", arguments.join(' ').toStdString());
        return 1;
    }

    auto modes = output->customModes();
    if (modes.contains(*modeInfo)) {
        return 0;
    }

    modes.append(*modeInfo);
    output->setCustomModes(modes);

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Add a custom mode", "Modes")

OUTPUT_COMMAND_EXAMPLE("Add a custom mode with reduced blanking",
                       "active-output add-custom-mode --reduced-blanking 1920x1080@60.0Hz")
