/*************************************************************************************
*  Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>                        *
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

#include <kscreen/config.h>

#include "generator.h"

class OutputWidget;

class OsdWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OsdWidget(QWidget *parent = nullptr, 
                       Qt::WindowFlags f = Qt::ToolTip);
    ~OsdWidget();

    bool isAbleToShow(const KScreen::ConfigPtr &config);
    void pluggedIn();
    void hideAll();

Q_SIGNALS:
    void displaySwitch(Generator::DisplaySwitchAction mode);

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void slotItemClicked(QListWidgetItem*);

private:
    void createItem(const QString &iconName, const QString &modeLabel);
    void createLine();
    bool isShowMe();

    QListWidget *m_modeList;
    bool m_pluggedIn;
    OutputWidget *m_primaryOutputWidget;
    OutputWidget *m_secondOutputWidget;
};

class OutputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OutputWidget(const QString &id,
                          QWidget *parent = nullptr, 
                          Qt::WindowFlags f = Qt::ToolTip);
    ~OutputWidget();
};

#endif /* __OSD_WIDGET_H__ */
