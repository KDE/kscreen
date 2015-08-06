/*************************************************************************************
*  Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>                        *
*  Copyright (C) 2015 Dan Vrátil <dvratil@redhat.com>                               *
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

#ifndef __OSD_WIDGET_H__
#define __OSD_WIDGET_H__

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QList>

#include <kscreen/config.h>

#include "generator.h"

class OutputWidget;
class OsdWidgetItem;

class OsdWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OsdWidget(QWidget *parent = Q_NULLPTR,
                       Qt::WindowFlags f = Qt::ToolTip);
    ~OsdWidget();

    bool isAbleToShow(const KScreen::ConfigPtr &config);
    void pluggedIn();
    void hideAll();

Q_SIGNALS:
    void displaySwitch(Generator::DisplaySwitchAction mode);

private slots:
    void slotItemClicked(OsdWidgetItem *item);

private:
    bool isShowMe();
    void clearOutputWidgets();

    QListWidget *m_modeList;
    bool m_pluggedIn;
    QList<OutputWidget *> m_outputWidgets;
};

class OutputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OutputWidget(const QString &name, 
                          const QPoint &position, 
                          QWidget *parent = Q_NULLPTR,
                          Qt::WindowFlags f = Qt::ToolTip);
};

#endif /* __OSD_WIDGET_H__ */
