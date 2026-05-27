/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static int run(KScreen::OutputPtr output)
{
    const auto customModes = output->customModes();
    for (int i = 0; i < customModes.size(); ++i) {
        const auto &mode = customModes[i];
        std::print(std::cerr, "{}: {}x{}@{:.2f}Hz", i, mode.size.width(), mode.size.height(), mode.refreshRate);
        if (mode.flags & KScreen::ModeInfo::Flag::ReducedBlanking) {
            std::print(std::cerr, " +reduced-blanking");
        }
        std::print(std::cerr, "\n");
    }

    return 0;
}

OUTPUT_COMMAND(run, "List custom output modes", "Modes")

OUTPUT_COMMAND_EXAMPLE("List custom modes",
                       "active-output list-custom-modes")
