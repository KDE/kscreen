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

#ifndef MODESELECTIONWIDGET_H
#define MODESELECTIONWIDGET_H

#include <QDeclarativeItem>
#include <QModelIndex>

class QListView;
class QMLOutput;
class ModesProxyModel;
class ResolutionSortModel;

class ModeSelectionWidget : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QMLOutput *output READ output WRITE setOutput)
public:
    explicit ModeSelectionWidget(QDeclarativeItem *parent = 0);
    virtual ~ModeSelectionWidget();

    void setOutput(QMLOutput *output);
    QMLOutput* output() const;

private Q_SLOTS:
    void resolutionChanged(const QModelIndex &index);
    void acceptMode(const QModelIndex &index);
    void refreshRateChanged();

Q_SIGNALS:
    void accepted();

private:
    QMLOutput *m_output;

    QListView *m_resolutionsView;
    QListView *m_refreshRatesView;
    ModesProxyModel *m_refreshRatesModel;
    ResolutionSortModel *m_resolutionsModel;
    QWidget *rootWidget;

    QGraphicsProxyWidget *m_proxyWidget;

};

#endif // MODESELECTIONWIDGET_H
