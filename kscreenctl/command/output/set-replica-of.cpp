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
    if (arguments[0] == "none") {
        output->setReplicationSource(0);
    } else {
        const KScreen::OutputPtr replicationSource = outputByQuery(config, arguments[0]);
        if (!replicationSource) {
            std::println(std::cerr, "Unknown replication source: {}", arguments[0].toStdString());
            return 1;
        }

        if (output->id() == replicationSource->id()) {
            std::println(std::cerr, "{} cannot mirror itself", arguments[0].toStdString());
            return 1;
        }

        output->setReplicationSource(replicationSource->id());
    }

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Mirror or unmirror another output (possible values: none or an OUTPUT)", "Misc")
