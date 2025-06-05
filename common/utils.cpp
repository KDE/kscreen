/*
    SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "utils.h"

#include <kscreen/edid.h>
#include <kscreen/mode.h>

#include <KLocalizedString>

QString Utils::outputName(const KScreen::OutputPtr &output, bool shouldShowSerialNumber, bool shouldShowConnector)
{
    return outputName(output.data(), shouldShowSerialNumber, shouldShowConnector);
}

QString Utils::outputName(const KScreen::Output *output, bool shouldShowSerialNumber, bool shouldShowConnector)
{
    if (output->type() == KScreen::Output::Panel) {
        return i18nd("kscreen_common", "Built-in Screen");
    }

        // The name will be "VendorName ModelName (ConnectorName)",
        // but some components may be empty.
    QString name;
    if (!(output->vendor().isEmpty())) {
        name = output->vendor() + QLatin1Char(' ');
    }
    if (!output->model().isEmpty()) {
        name += output->model() + QLatin1Char(' ');
    }
    if (output->edid()) {
        if (!output->edid()->serial().isEmpty() && shouldShowSerialNumber) {
            name += output->edid()->serial() + QLatin1Char(' ');
        }
    }
    if (shouldShowConnector) {
        name += output->name();
    }
    if (!name.trimmed().isEmpty()) {
        return name;
    }
    return output->name();
}

QString Utils::sizeToString(const QSize &size)
{
    return QStringLiteral("%1x%2").arg(size.width()).arg(size.height());
}

KScreen::ModePtr Utils::biggestMode(const KScreen::ModeList &modes)
{
    Q_ASSERT(!modes.isEmpty());

    int modeArea, biggestArea = 0;
    KScreen::ModePtr biggestMode;
    for (const KScreen::ModePtr &mode : modes) {
        modeArea = mode->size().width() * mode->size().height();
        if (modeArea < biggestArea) {
            continue;
        }
        if (modeArea == biggestArea && mode->refreshRate() < biggestMode->refreshRate()) {
            continue;
        }
        if (modeArea == biggestArea && mode->refreshRate() > biggestMode->refreshRate()) {
            biggestMode = mode;
            continue;
        }

        biggestArea = modeArea;
        biggestMode = mode;
    }

    return biggestMode;
}
