/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static int run(KScreen::OutputPtr output)
{
    for (const auto modes = output->modes(); const auto &mode : modes) {
        std::println(std::cout, "{}", printableMode(output, mode).toStdString());
    }

    return 0;
}

OUTPUT_COMMAND(run, "List available output modes", "Modes")
