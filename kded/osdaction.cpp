/*
 *  SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>
 *  Work sponsored by the LiMux project of the city of Munich.
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "osdaction.h"

#include <KLocalizedString>

using namespace KScreen;

OsdAction::OsdAction(QObject *parent)
    : QObject(parent)
{
}

QVector<int> OsdAction::actionOrder() const
{
    return {SwitchToExternal, SwitchToInternal, Clone, ExtendLeft, ExtendRight, NoAction};
}

QString OsdAction::actionLabel(OsdAction::Action action) const
{
    switch (action) {
    // this is built by both daemon and plasmoid, needs explicit translation domain here
    case SwitchToExternal:
        return i18nd("kscreen", "Switch to external screen");
    case SwitchToInternal:
        return i18nd("kscreen", "Switch to laptop screen");
    case Clone:
        return i18nd("kscreen", "Unify outputs");
    case ExtendLeft:
        return i18nd("kscreen", "Extend to left");
    case ExtendRight:
        return i18nd("kscreen", "Extend to right");
    case NoAction:
        return i18nd("kscreen", "Leave unchanged");
    }

    Q_UNREACHABLE();
    return QString();
}

QString OsdAction::actionIconName(OsdAction::Action action) const
{
    switch (action) {
    case SwitchToExternal:
        return QStringLiteral("osd-shutd-laptop");
    case SwitchToInternal:
        return QStringLiteral("osd-shutd-screen");
    case Clone:
        return QStringLiteral("osd-duplicate");
    case ExtendLeft:
        return QStringLiteral("osd-sbs-left");
    case ExtendRight:
        return QStringLiteral("osd-sbs-sright");
    case NoAction:
        return QStringLiteral("dialog-cancel");
    }

    Q_UNREACHABLE();
    return QString();
}
