/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static bool json = false;
OUTPUT_COMMAND_TOGGLE_OPTION("json",
                             &json,
                             "Show output information as JSON")

static int run(KScreen::OutputPtr output)
{
    if (json) {
        showOutputAsJson(output);
    } else {
        showOutputAsText(output);
    }

    return 0;
}

OUTPUT_COMMAND_WITH_DESCRIPTION(
    run,
    "Query or change output settings",
    "The OUTPUT specifies the connector name, e.g. DP-1, or one of the special identifiers - \"active-output\" or \"primary-output\".",
    "Basic")

OUTPUT_COMMAND_EXAMPLE("Show information about the current output",
                       "active-output")

OUTPUT_COMMAND_EXAMPLE("Show information about the current output as JSON",
                       "active-output --json")
