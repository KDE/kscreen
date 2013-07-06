/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "modesproxymodel.h"
#include "qmloutput.h"

#include <KComboBox>
#include <KDebug>
#include <KLocalizedString>

ModesProxyModel::ModesProxyModel(QObject* parent):
    QSortFilterProxyModel(parent),
    m_currentSourceRow(-1)
{
    setSortRole(QMLOutput::RefreshRateRole);
    setDynamicSortFilter(true);
}

ModesProxyModel::~ModesProxyModel()
{

}

int ModesProxyModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return 1;
}

int ModesProxyModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    if (!sourceModel()) {
        return 0;
    }

    QModelIndex parentIndex = sourceModel()->index(m_currentSourceRow, 0);
    return sourceModel()->rowCount(parentIndex) + 1;
}

QVariant ModesProxyModel::data(const QModelIndex& index, int role) const
{
    if (!sourceModel() || !index.isValid()) {
        return QVariant();
    }

    if (index.row() == 0) {
        if (role == Qt::DisplayRole) {
            return i18nc("As in automatic", "Auto");
        } else if (role == QMLOutput::RefreshRateRole) {
            return QLatin1String("-1");
        } else if (role == QMLOutput::ModeIdRole) {
            return QLatin1String("-1");
        } else {
            return QVariant();
        }
    }

    QModelIndex parentIndex = sourceModel()->index(m_currentSourceRow, 0);
    QModelIndex realIndex = parentIndex.child(index.row() - 1, 0);
    return sourceModel()->data(realIndex, role);
}

QModelIndex ModesProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return createIndex(row, column, 0);
}

QModelIndex ModesProxyModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child)

    /* Flatten the model */
    return QModelIndex();
}

QModelIndex ModesProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid()) {
        return QModelIndex();
    }

    QModelIndex realParentIndex = sourceModel()->index(m_currentSourceRow, 0);

    if (proxyIndex.row() == 0) {
        return realParentIndex.child(proxyIndex.row(), 0);
    }

    return realParentIndex.child(proxyIndex.row() - 1, 0);
}

bool ModesProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    float rateA = left.data(QMLOutput::RefreshRateRole).toFloat();
    float rateB = right.data(QMLOutput::RefreshRateRole).toFloat();

    if (rateA == -1) {
        return true;
    }

    return (rateA < rateB);
}

void ModesProxyModel::setSourceModelCurrentRow(int index)
{
    m_currentSourceRow = index;
}

#include "modesproxymodel.moc"
