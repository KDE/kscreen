/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "profilesmodel.h"

#include <QTimer>
#include <QDBusConnection>
#include <QDBusMetaType>

ProfilesModel::ProfilesModel(QObject *parent):
    QStandardItemModel(parent)
{

    mInterface = new org::kde::KScreen(QLatin1String("org.kde.KScreen"),
                                       QLatin1String("/modules/kscreen"),
                                       QDBusConnection::sessionBus(),
                                       this);
    connect(mInterface, SIGNAL(profilesChanged()),
            this, SLOT(reloadProfiles()));

    QTimer::singleShot(0, this, SLOT(reloadProfiles()));
}

ProfilesModel::~ProfilesModel()
{
}

void ProfilesModel::reloadProfiles()
{
    const QMap<QString,QString> profiles = mInterface->listCurrentProfiles();

    clear();
    mProfilesCache.clear();

    QMapIterator<QString, QString> iter(profiles);
    while (iter.hasNext()) {
        iter.next();

        QStandardItem *item = new QStandardItem(iter.value());
        item->setData(iter.key(), ProfileIDRole);

        appendRow(item);
    }
}

QVariant ProfilesModel::data(const QModelIndex &index, int role) const
{
    if (role == ConfigRole) {
        const QString profileId = QStandardItemModel::data(index, ProfileIDRole).toString();
        if (!mProfilesCache.contains(profileId)) {
            const QVariant config = mInterface->getProfile(profileId).value().variant();
            mProfilesCache.insert(profileId, config);
            return config;
        }

        return mProfilesCache.value(profileId);
    }

    return QStandardItemModel::data(index, role);
}


#include "profilesmodel.moc"
