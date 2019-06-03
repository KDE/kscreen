/*
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>
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

#ifndef OSDM_H
#define OSDM_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QTimer>

#include "osdaction.h"


namespace KScreen {

class ConfigOperation;
class Osd;
class Output;

class OsdManager : public QObject {
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
    QMap<QString, KScreen::Osd*> m_osds;
    QTimer* m_cleanupTimer;
};

} // ns
#endif // OSDM_H
