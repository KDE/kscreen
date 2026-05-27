/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "POLICY",
    &arguments
)

static std::optional<KScreen::Output::AutoRotatePolicy> parseAutoRotatePolicy(const QString &input)
{
    if (input == "never") {
        return KScreen::Output::AutoRotatePolicy::Never;
    } else if (input == "in-tablet-mode") {
        return KScreen::Output::AutoRotatePolicy::InTabletMode;
    } else if (input == "always") {
        return KScreen::Output::AutoRotatePolicy::Always;
    }
    return std::nullopt;
}

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto policy = parseAutoRotatePolicy(arguments[0]);
    if (!policy) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    output->setAutoRotatePolicy(*policy);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Turn on or turn off auto rotation", "Misc")
