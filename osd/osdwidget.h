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

#include <kscreen/configoperation.h>
#include <kscreen/config.h>
#include <kscreen/output.h>

class OsdWidget : public QWidget 
{
    Q_OBJECT

public:
    explicit OsdWidget(QWidget *parent = nullptr,
                       Qt::WindowFlags f = Qt::ToolTip);
    ~OsdWidget();

    bool isAbleToShow();

private slots:
    void slotItemClicked(QListWidgetItem*);
    void slotConfigReady(KScreen::ConfigOperation*);

private:
    void m_doApplyConfig();
    void m_pcScreenOnly();
    QSize m_findSimilarResolution(KScreen::OutputPtr primary, 
                                  KScreen::OutputPtr second);
    void m_mirror();
    void m_extend();
    void m_secondScreenOnly();

    bool m_configIsReady;
    QListWidget *m_modeList;
    KScreen::ConfigPtr m_config;
};

#endif /* __OSD_WIDGET_H__ */
