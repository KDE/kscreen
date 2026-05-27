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

static std::optional<KScreen::Output::EdrPolicy> parseEdrPolicy(const QString &input)
{
    if (input == "never") {
        return KScreen::Output::EdrPolicy::Never;
    } else if (input == "always") {
        return KScreen::Output::EdrPolicy::Always;
    }
    return std::nullopt;
}

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto policy = parseEdrPolicy(arguments[0]);
    if (!policy) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    output->setEdrPolicy(*policy);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the extended dynamic range policy (possible values: never or always)", "Color Management")
