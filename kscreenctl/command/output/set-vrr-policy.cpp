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

static std::optional<KScreen::Output::VrrPolicy> parseVrrPolicy(const QString &input)
{
    if (input == "never") {
        return KScreen::Output::VrrPolicy::Never;
    } else if (input == "always") {
        return KScreen::Output::VrrPolicy::Always;
    } else if (input == "automatic") {
        return KScreen::Output::VrrPolicy::Automatic;
    }
    return std::nullopt;
}

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto policy = parseVrrPolicy(arguments[0]);
    if (!policy) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    output->setVrrPolicy(*policy);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the VRR policy (possible values: never, always, and automatic)", "Misc")
