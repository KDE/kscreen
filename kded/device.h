/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef KDED_DEVICE_H
#define KDED_DEVICE_H

#include <QtCore/QObject>

class QDBusPendingCallWatcher;
class OrgFreedesktopDBusPropertiesInterface;
class Device : public QObject
{
    Q_OBJECT
    public:
        static Device* self();
        static void destroy();

        bool isReady();
        bool isLaptop();
        bool isLidClosed();
        bool isDocked();

    private Q_SLOTS:
        void init();
        void changed();
        void isLaptopFetched(QDBusPendingCallWatcher* watcher);
        void isLidClosedFetched(QDBusPendingCallWatcher* watcher);

    Q_SIGNALS:
        void ready();
        void lidIsClosedChanged(bool after, bool before);

    private:
        explicit Device(QObject* parent = 0);
        virtual ~Device();

        void setReady();
        void fetchIsLaptop();
        void fetchLidIsClosed();
        void fetchIsDocked();

        bool m_isReady;
        bool m_isLaptop;
        bool m_isLidClosed;
        bool m_isDocked;

        static Device* m_instance;

        OrgFreedesktopDBusPropertiesInterface* m_freedesktop;
};

#endif //KDED_DEVICE_H