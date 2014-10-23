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
#include <QDBusReply>

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
    Q_EMIT aboutToUpdateModel();

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

    Q_EMIT modelUpdated();
}

QVariant ProfilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == ProfileRole) {
        const QString profileId = QStandardItemModel::data(index, ProfileIDRole).toString();
        if (!mProfilesCache.contains(profileId)) {
            QVariantMap profile = mInterface->getProfile(profileId);
            profile["outputs"] = parseOutputs(profile[QLatin1String("outputs")]);

            mProfilesCache.insert(profileId, profile);
            return profile;
        }

        return mProfilesCache.value(profileId);
    }

    return QStandardItemModel::data(index, role);
}

int ProfilesModel::activeProfileIndex() const
{
    const QString activeProfile = mInterface->activeProfile();

    for (int i = 0; i < rowCount(); i++) {
        const QModelIndex rowIndex = index(i, 0);
        if (activeProfile.isEmpty()) {
            const QVariantMap map = data(rowIndex, ProfileRole).toMap();
            if (map[QLatin1String("preferred")].toBool()) {
                return i;
            }
        } else {
            if (data(rowIndex, ProfileIDRole).toString() == activeProfile) {
                return i;
            }
        }
    }

    return -1;
}

// FIXME: Yeah, if someone could explain me why this cannot happen
// automatically, that would be great.
QVariant ProfilesModel::parseOutputs(const QVariant &variant) const
{
    QVariantList outputs;

    QDBusArgument arg = variant.value<QDBusArgument>();
    arg >> outputs;

    for (int i = 0; i < outputs.count(); i++) {
        QDBusArgument arg = outputs.at(i).value<QDBusArgument>();
        QVariantMap output;
        arg >> output;

        QVariantMap metadata;
        arg = output[QLatin1String("metadata")].value<QDBusArgument>();
        arg >> metadata;
        output[QLatin1String("metadata")] = metadata;

        QVariantMap pos;
        arg = output[QLatin1String("pos")].value<QDBusArgument>();
        arg >> pos;
        output[QLatin1String("pos")] = pos;

        if (output.contains(QLatin1String("mode"))) {
            QVariantMap mode;
            arg = output[QLatin1String("mode")].value<QDBusArgument>();
            arg >> mode;

            QVariantMap size;
            arg = mode[QLatin1String("size")].value<QDBusArgument>();
            arg >> size;
            mode[QLatin1String("size")] = size;
            output[QLatin1String("mode")] = mode;
        }

        outputs.replace(i, output);
    }

    return outputs;
}
