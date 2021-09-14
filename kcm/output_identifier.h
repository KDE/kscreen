/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <kscreen/config.h>

#include <QVector>

namespace PlasmaQuick
{
class Dialog;
}

class OutputIdentifier : public QObject
{
    Q_OBJECT

public:
    explicit OutputIdentifier(KScreen::ConfigPtr config, QObject *parent = nullptr);
    ~OutputIdentifier() override;

Q_SIGNALS:
    void identifiersFinished();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QVector<PlasmaQuick::Dialog *> m_views;
};
