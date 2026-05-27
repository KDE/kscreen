/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "ROTATION",
    &arguments
)

static std::optional<KScreen::Output::Rotation> parseRotation(const QString &value)
{
    static const QHash<QString, KScreen::Output::Rotation> rotationMap {
        { u"none"_s, KScreen::Output::None },
        { u"normal"_s, KScreen::Output::None },
        { u"left"_s, KScreen::Output::Left },
        { u"right"_s, KScreen::Output::Right },
        { u"inverted"_s, KScreen::Output::Inverted },
        { u"flipped"_s, KScreen::Output::Flipped },
        { u"flipped90"_s, KScreen::Output::Flipped90 },
        { u"flipped180"_s, KScreen::Output::Flipped180 },
        { u"flipped270"_s, KScreen::Output::Flipped270 },
        { u"0"_s, KScreen::Output::None },
        { u"90"_s, KScreen::Output::Left },
        { u"180"_s, KScreen::Output::Inverted },
        { u"270"_s, KScreen::Output::Right },
    };

    if (const auto it = rotationMap.find(value); it != rotationMap.end()) {
        return *it;
    }

    return std::nullopt;
}

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto rotation = parseRotation(arguments[0]);
    if (!rotation) {
        std::println(std::cerr, "Invalid argument: {}", arguments[0].toStdString());
        return 1;
    }

    const QSize previousSize = output->explicitLogicalSizeInt();
    output->setRotation(*rotation);
    output->setExplicitLogicalSize(config->logicalSizeForOutputInt(*output));
    snapOutputAfterResize(output, previousSize, config);

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the output transform. Outputs are rotated counter-clockwise (possible values: none, normal, 0, 90, 180, 270, flipped, flipped90, flipped180, or flipped270)", "Basic")
