/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

#include <QCollator>

static int run(KScreen::OutputPtr output)
{
    QCollator collator;
    collator.setNumericMode(true);

    const auto modes = output->modes();
    auto modeKeys = modes.keys();
    std::sort(modeKeys.begin(), modeKeys.end(), collator);

    for (const auto &key : modeKeys) {
        const auto mode = *modes.find(key);
        std::println(std::cout, "{}", printableMode(output, mode).toStdString());
    }

    return 0;
}

OUTPUT_COMMAND(run, "List available output modes", "Modes")
