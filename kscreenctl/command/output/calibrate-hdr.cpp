/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

#include <QProcess>

static int run(KScreen::OutputPtr output)
{
    if (!output->isHdrEnabled()) {
        std::println(std::cerr, "HDR is not active on output {}", output->name().toStdString());
        return 1;
    }

    const bool ok = QProcess::startDetached(u"hdrcalibrator"_s, {output->name()});
    return ok ? 0 : 1;
}

OUTPUT_COMMAND(run, "Calibrate HDR", "Color Management")
