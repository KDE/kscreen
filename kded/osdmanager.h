/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OSDM_H
#define OSDM_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>

#include "osdaction.h"

namespace KScreen
{
class ConfigOperation;
class Osd;
class Output;

class OsdManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kscreen.osdService")

public:
    OsdManager(QObject *parent = nullptr);
    ~OsdManager() override;

public Q_SLOTS:
    void showOutputIdentifiers();
    void showOsd(const QString &icon, const QString &text);
    void hideOsd();
    OsdAction *showActionSelector();

private:
    void slotIdentifyOutputs(KScreen::ConfigOperation *op);
    QMap<QString, KScreen::Osd *> m_osds;
    QTimer *m_cleanupTimer;
};

} // ns
#endif // OSDM_H
