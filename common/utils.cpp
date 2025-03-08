/*
    SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "utils.h"

#include <kscreen/edid.h>

#include <KLocalizedString>

QString Utils::outputName(const KScreen::OutputPtr &output, bool shouldShowSerialNumber, bool shouldShowConnector)
{
    return outputName(output.data(), shouldShowSerialNumber, shouldShowConnector);
}

QString Utils::outputName(const KScreen::Output *output, bool shouldShowSerialNumber, bool shouldShowConnector)
{
    Q_UNUSED(shouldShowSerialNumber);
    Q_UNUSED(shouldShowConnector);

    QString name;
    if (output->type() == KScreen::Output::Panel) {
        // Give laptop panels a good name!
        name = i18nd("kscreen_common", "Built-in Screen");
    } else if (!output->model().isEmpty()) {
        // Otherwise, first, try the model...
        name = output->model();

        // Prepend vendor if it's unlikely to be part of the model string
        // This seems typical of older displays, but not of newer ones.
        if (output->model().split(' ').length() <= 1 && !output->vendor().isEmpty()) {
            name.prepend(output->vendor() + ' ');
        }
    } else if (output->edid() && !output->edid()->serial().isEmpty()) {
        // Maybe the serial is good?
        name = output->edid()->serial();
    } else {
        // Bugger all.
        name = i18nd("kscreen_common", "Unknown");
    }

    // Let's always show the index // TODO only when there are more than one display!
    name.append(QStringLiteral(" (%1)").arg(output->id()));

    return name;

    /*
    // The name will be "VendorName ModelName (ConnectorName)",
    // but some components may be empty.
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
    */
}

QString Utils::sizeToString(const QSize &size)
{
    return QStringLiteral("%1x%2").arg(size.width()).arg(size.height());
}
