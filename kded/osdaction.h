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

#pragma once

#include <QObject>
#include <QString>
#include <QVector>

namespace KScreen {

class OsdAction : public QObject
{
    Q_OBJECT
public:
    enum Action {
        NoAction,
        SwitchToExternal,
        SwitchToInternal,
        Clone,
        ExtendLeft,
        ExtendRight
    };
    Q_ENUM(Action)

    explicit OsdAction(QObject *parent = nullptr);

    Q_INVOKABLE QVector<int> actionOrder() const;
    Q_INVOKABLE QString actionLabel(Action action) const;
    Q_INVOKABLE QString actionIconName(Action action) const;

Q_SIGNALS:
    void selected(Action action);

};

} // namespace KScreen
