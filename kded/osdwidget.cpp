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
#include <KToolInvocation>

#include <kscreen/output.h>
#include <kscreen/setconfigoperation.h>

static const QString PC_SCREEN_ONLY_MODE = i18n("PC screen only");
static const QString MIRROR_MODE = i18n("Mirror");
static const QString EXTEND_MODE = i18n("Extend");
static const QString SECOND_SCREEN_ONLY_MODE = i18n("Second screen only");

OsdWidget::OsdWidget(KScreen::ConfigPtr config, 
                     QWidget *parent, 
                     Qt::WindowFlags f) 
  : QWidget(parent, f),
    m_config(config)
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
    unsigned int outputConnected = 0;
    bool hasPrimary = false;

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output->isPrimary() || output->name().contains(lvdsPrefix))
            hasPrimary = true;

        if (output->isConnected())
            outputConnected++;
    }

    if (hasPrimary && outputConnected == 2) {
        show();
    }

    if (outputConnected > 2) {
        KToolInvocation::kdeinitExec(QString("kcmshell5"),
                                     QStringList() << QString("kcm_kscreen"));
    }
}

void OsdWidget::m_doApplyConfig()
{
    if (!KScreen::Config::canBeApplied(m_config)) {
        // Invalid screen config
    }

    connect(new KScreen::SetConfigOperation(m_config), &KScreen::SetConfigOperation::finished,
            [&]() {
                qDebug() << "Config applied";
            });
}

void OsdWidget::m_pcScreenOnly() 
{
    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (!output)
            continue;

        if (!output->isConnected())
            continue;

        if (output->isPrimary() || output->name().contains(lvdsPrefix)) {
            output->setEnabled(true);
            output->setPrimary(true);
        } else {
            output->setEnabled(false);
            output->setPrimary(false);
        }
    }

    m_doApplyConfig();
}

void OsdWidget::m_mirror() 
{
    QPoint primaryPos(0, 0);

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (!output)
            continue;

        if (!output->isConnected())
            continue;

        if (!output->isEnabled()) {
            output->setEnabled(true);
            output->setCurrentModeId(output->preferredModeId());
        }

        if (output->isPrimary() || output->name().contains(lvdsPrefix)) {
            primaryPos = output->pos();
        } else {
            output->setPos(primaryPos);
        }
    }

    m_doApplyConfig();
}

void OsdWidget::m_extend() 
{
    QPoint secondPos(0, 0);

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (!output)
            continue;

        if (!output->isConnected())
            continue;

        if (!output->isEnabled()) {
            output->setEnabled(true);
            output->setCurrentModeId(output->preferredModeId());
        }

        if (output->isPrimary() || output->name().contains(lvdsPrefix)) {
            output->setPos(secondPos);
            secondPos.setX(output->pos().x() + output->size().width());
        } else {
            output->setPos(secondPos);
        }
    }

    m_doApplyConfig();
}

void OsdWidget::m_secondScreenOnly() 
{
    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (!output)
            continue;

        if (!output->isConnected())
            continue;

        if (output->isPrimary() || output->name().contains(lvdsPrefix)) {
            output->setEnabled(false);
            output->setPrimary(false);
        } else {
            output->setEnabled(true);
            output->setCurrentModeId(output->preferredModeId());
            output->setPrimary(true);
        }
    }

    m_doApplyConfig();
}

void OsdWidget::slotItemClicked(QListWidgetItem *item) 
{
    hide();

    if (item->text() == PC_SCREEN_ONLY_MODE) {
        m_pcScreenOnly();
    } else if (item->text() == MIRROR_MODE) {
        m_mirror();
    } else if (item->text() == EXTEND_MODE) {
        m_extend();
    } else if (item->text() == SECOND_SCREEN_ONLY_MODE) {
        m_secondScreenOnly();
    }
}
