/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "OUTPUT",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const KScreen::OutputPtr other = outputByQuery(config, arguments[0]);
    if (!other) {
        std::println(std::cerr, "Unknown output: {}", arguments[0].toStdString());
        return 1;
    }

    if (output->id() == other->id()) {
        std::println(std::cerr, "The reference output cannot be the same as the moved output");
        return 1;
    }

    output->setPos(QPoint(other->pos().x() + other->explicitLogicalSizeInt().width(), other->pos().y()));
    fixupOutputPositions(config);

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Place this output to the right of another output", "Positioning")
