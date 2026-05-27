/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "PATH",
    &arguments
)

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    output->setIccProfilePath(arguments[0]);
    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Set the ICC profile path", "Color Management")

OUTPUT_COMMAND_EXAMPLE("Assign an ICC profile to the current output. The profile will only be used in the SDR mode. For HDR, you want to use the `set-hdr-icc-profile` command",
                       "active-output set-icc-profile ~/path/to/profile.icc")
