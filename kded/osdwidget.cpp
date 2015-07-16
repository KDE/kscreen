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

#include <KLocalizedString>

static const QString PC_SCREEN_ONLY_MODE = i18n("PC screen only");
static const QString MIRROR_MODE = i18n("Mirror");
static const QString EXTEND_MODE = i18n("Extend");
static const QString SECOND_SCREEN_ONLY_MODE = i18n("Second screen only");

OsdWidget::OsdWidget(QWidget *parent, Qt::WindowFlags f) 
  : QWidget(parent, f) 
{
    setFixedSize(467, 280);
    QDesktopWidget *desktop = QApplication::desktop();
    move((desktop->width() - width()) / 2, (desktop->height() - height()) / 2);

    QVBoxLayout *vbox = new QVBoxLayout;

    QListWidget *modeList = new QListWidget;
    modeList->setFlow(QListView::LeftToRight);
    connect(modeList, SIGNAL(itemClicked(QListWidgetItem*)), 
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    vbox->addWidget(modeList);

    QListWidgetItem *item = new QListWidgetItem(PC_SCREEN_ONLY_MODE);
    modeList->addItem(item);

    item = new QListWidgetItem(MIRROR_MODE);
    modeList->addItem(item);

    item = new QListWidgetItem(EXTEND_MODE);
    modeList->addItem(item);

    item = new QListWidgetItem(SECOND_SCREEN_ONLY_MODE);
    modeList->addItem(item);

    setLayout(vbox);
}

OsdWidget::~OsdWidget() 
{
}

void OsdWidget::showAll() 
{
    show();
}

void OsdWidget::slotItemClicked(QListWidgetItem *item) 
{
    hide();

    if (item->text() == PC_SCREEN_ONLY_MODE) {
        emit pcScreenOnly();
    } else if (item->text() == MIRROR_MODE) {
        emit mirror();
    } else if (item->text() == EXTEND_MODE) {
        emit extend();
    } else if (item->text() == SECOND_SCREEN_ONLY_MODE) {
        emit secondScreenOnly();
    }
}
