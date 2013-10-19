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


#ifndef PROFILESMODEL_H
#define PROFILESMODEL_H

#include <QtGui/QStandardItemModel>

#include "kscreeninterface.h"

class ProfilesModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    enum {
        ProfileIDRole = Qt::UserRole + 1,
        ProfileRole
    };

    explicit ProfilesModel(QObject *parent = 0);
    virtual ~ProfilesModel();

    int activeProfileIndex() const;
    int profileIndex(const QString &profileId);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

  public Q_SLOTS:
    void reloadProfiles();

  Q_SIGNALS:
    void aboutToUpdateModel();
    void modelUpdated();

  private:
    QVariant parseOutputs(const QVariant &variant) const;

    org::kde::KScreen *mInterface;

    mutable QMap<QString, QVariant> mProfilesCache;
};

#endif // PROFILESMODEL_H
