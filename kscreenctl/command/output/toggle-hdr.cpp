/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    output->setHdrEnabled(!output->isHdrEnabled());
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Toggle HDR", "Color Management")
