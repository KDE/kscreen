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

#include "osdwidget.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QListWidget>

OsdWidget::OsdWidget(QWidget *parent, Qt::WindowFlags f) 
  : QWidget(parent, f) 
{
    setFixedSize(467, 280);
    QDesktopWidget* desktop = QApplication::desktop();
    move((desktop->width() - width()) / 2, (desktop->height() - height()) / 2);

    QVBoxLayout* vbox = new QVBoxLayout;

    QListWidget* modeList = new QListWidget;
    modeList->setFlow(QListView::LeftToRight);
    connect(modeList, SIGNAL(itemClicked(QListWidgetItem*)), 
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    vbox->addWidget(modeList);

    QListWidgetItem* item = new QListWidgetItem("Laptop Only");
    modeList->addItem(item);

    item = new QListWidgetItem("Mirror");
    modeList->addItem(item);

    item = new QListWidgetItem("Extend");
    modeList->addItem(item);

    item = new QListWidgetItem("Secondary Only");
    modeList->addItem(item);

    setLayout(vbox);
}

OsdWidget::~OsdWidget() 
{
}

void OsdWidget::slotItemClicked(QListWidgetItem* itemClicked) 
{
    hide();
}
