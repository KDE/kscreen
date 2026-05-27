/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

static int run(KScreen::OutputPtr output)
{
    std::println(std::cout, "{}", output->name().toStdString());

    return 0;
}

OUTPUT_COMMAND(run, "Get connector name", "Basic")

OUTPUT_COMMAND_EXAMPLE("Get the connector name of the active output",
                       "active-output get-name")
