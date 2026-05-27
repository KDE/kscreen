/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto enabled = output->isHdrEnabled() || output->isWcgEnabled();
    output->setHdrEnabled(!enabled);
    output->setWcgEnabled(!enabled);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Toggle both HDR and wide color gamut", "Color Management")

OUTPUT_COMMAND_EXAMPLE("Toggle both HDR and wide color gamut on the current output",
                       "active-output toggle-hdr-and-wcg")
