/*
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>
 *  Copyright (c) 2018 Kai Uwe Broulik <kde@broulik.de>
 *                     Work sponsored by the LiMux project of
 *                     the city of Munich.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
    return {
        SwitchToExternal,
        SwitchToInternal,
        Clone,
        ExtendLeft,
        ExtendRight,
        NoAction
    };
}

QString OsdAction::actionLabel(OsdAction::Action action) const
{
    switch (action) {
    // this is built by both daemon and plasmoid, needs explicit translation domain here
    case SwitchToExternal: return i18nd("kscreen", "Switch to external screen");
    case SwitchToInternal: return i18nd("kscreen", "Switch to laptop screen");
    case Clone: return i18nd("kscreen", "Unify outputs");
    case ExtendLeft: return i18nd("kscreen", "Extend to left");
    case ExtendRight: return i18nd("kscreen", "Extend to right");
    case NoAction: return i18nd("kscreen", "Leave unchanged");
    }

    Q_UNREACHABLE();
    return QString();
}

QString OsdAction::actionIconName(OsdAction::Action action) const
{
    switch (action) {
    case SwitchToExternal: return QStringLiteral("osd-shutd-laptop");
    case SwitchToInternal: return QStringLiteral("osd-shutd-screen");
    case Clone: return QStringLiteral("osd-duplicate");
    case ExtendLeft: return QStringLiteral("osd-sbs-left");
    case ExtendRight: return QStringLiteral("osd-sbs-sright");
    case NoAction: return QStringLiteral("dialog-cancel");
    }

    Q_UNREACHABLE();
    return QString();
}
