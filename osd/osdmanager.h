/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>
#include <qqmlregistration.h>

#include "common/osdaction.h"

namespace KScreen
{
class ConfigOperation;
class Osd;
class Output;

struct OsdActionForeign : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(OsdAction)
    QML_UNCREATABLE("Only for enums")
    QML_EXTENDED_NAMESPACE(KScreen::OsdAction)
};

class OsdManager : public QObject
{
    Q_OBJECT

public:
    OsdManager(QObject *parent = nullptr);
    ~OsdManager() override;

public Q_SLOTS:
    void hideOsd();
    void showActionSelector();

private:
    void quit();
    QMap<QString, KScreen::Osd *> m_osds;
    QTimer *m_cleanupTimer;
};

} // ns
