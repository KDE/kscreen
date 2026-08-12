/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    if (output->isEnabled() && !(output->capabilities() & KScreen::Output::Capability::Disable)) {
        std::println(std::cerr, "Disabling this output is not supported");
        return 1;
    }

    output->setEnabled(!output->isEnabled());
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Enable or disable the output", "Basic")
