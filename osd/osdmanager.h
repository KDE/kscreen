/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDBusContext>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>

#include "../common/osdaction.h"

namespace KScreen
{
class ConfigOperation;
class Osd;
class Output;

class OsdManager : public QObject, public QDBusContext
{
    Q_OBJECT

public:
    OsdManager(QObject *parent = nullptr);
    ~OsdManager() override;

public Q_SLOTS:
    void hideOsd();
    OsdAction::Action showActionSelector();

private:
    void quit();
    QMap<QString, KScreen::Osd *> m_osds;
    QTimer *m_cleanupTimer;
};

} // ns
