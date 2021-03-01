/*
 *  SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>
 *  Work sponsored by the LiMux project of the city of Munich.
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVector>

namespace KScreen
{
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
        ExtendRight,
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
