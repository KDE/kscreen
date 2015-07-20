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
#include <QMessageBox>

#include <KLocalizedString>
#include <KToolInvocation>

#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

static const QString lvdsPrefix = "LVDS";
static const QString PC_SCREEN_ONLY_MODE = i18n("PC screen only");
static const QString MIRROR_MODE = i18n("Mirror");
static const QString EXTEND_MODE = i18n("Extend");
static const QString SECOND_SCREEN_ONLY_MODE = i18n("Second screen only");

OsdWidget::OsdWidget(QWidget *parent, Qt::WindowFlags f) 
  : QWidget(parent, f),
    m_modeList(nullptr),
    m_configIsReady(false)
{
    setFixedSize(467, 280);
    QDesktopWidget *desktop = QApplication::desktop();
    move((desktop->width() - width()) / 2, (desktop->height() - height()) / 2);

    QVBoxLayout *vbox = new QVBoxLayout;

    m_modeList = new QListWidget;
    m_modeList->setFlow(QListView::LeftToRight);
    connect(m_modeList, SIGNAL(itemClicked(QListWidgetItem*)), 
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    vbox->addWidget(m_modeList);

    QListWidgetItem *item = new QListWidgetItem(PC_SCREEN_ONLY_MODE);
    m_modeList->addItem(item);

    item = new QListWidgetItem(MIRROR_MODE);
    m_modeList->addItem(item);

    item = new QListWidgetItem(EXTEND_MODE);
    m_modeList->addItem(item);

    item = new QListWidgetItem(SECOND_SCREEN_ONLY_MODE);
    m_modeList->addItem(item);

    setLayout(vbox);

    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
            this, &OsdWidget::slotConfigReady);
}

OsdWidget::~OsdWidget() 
{
}

void OsdWidget::slotConfigReady(KScreen::ConfigOperation* op)                   
{                                                                               
    if (op->hasError()) {
        return;
    }

    m_config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();
    
    isAbleToShow();
}

bool OsdWidget::isAbleToShow() 
{
    unsigned int outputConnected = 0;
    bool hasPrimary = false;
    bool primaryEnabled = true;
    bool secondEnabled = true;
    QPoint primaryPos(0, 0);
    QPoint secondPos(0, 0);

    if (m_config.isNull()) {
        QCoreApplication::quit();
        return false;
    }

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output.isNull())
            continue;
        
        if (!output->isConnected())
            continue;

        if (output->isPrimary() || output->name().contains(lvdsPrefix)) {
            hasPrimary = true;
            primaryEnabled = output->isEnabled();
            primaryPos = output->pos();
        } else {
            secondEnabled = output->isEnabled();
            secondPos = output->pos();
        }

        if (output->isConnected())
            outputConnected++;
    }

    if (hasPrimary && outputConnected == 2) {
        if (primaryEnabled && !secondEnabled)
            m_modeList->setCurrentRow(0);

        if (primaryEnabled && secondEnabled) {
            if (primaryPos == secondPos) {
                m_modeList->setCurrentRow(1);
            } else {
                // extend mode
                QDesktopWidget *desktop = QApplication::desktop();
                move(desktop->width() / 5, (desktop->height() - height()) / 2);
                m_modeList->setCurrentRow(2);
            }
        }

        if (!primaryEnabled && secondEnabled)
            m_modeList->setCurrentRow(3);

        return true;
    }

    if (outputConnected > 2) {
        KToolInvocation::kdeinitExec(QString("kcmshell5"),
                                     QStringList() << QString("kcm_kscreen"));
    }

    QCoreApplication::quit();

    return false;
}

void OsdWidget::m_doApplyConfig()
{
    if (m_config.isNull())
        return;
    
    if (!KScreen::Config::canBeApplied(m_config)) {
        // Invalid screen config
        QMessageBox msgBox;
        msgBox.setText("Invalid screen config");
        msgBox.exec();
    }

    auto *op = new KScreen::SetConfigOperation(m_config);
    op->exec();
}

void OsdWidget::m_pcScreenOnly() 
{
    if (m_config.isNull())
        return;

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output.isNull())
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

QSize OsdWidget::m_findSimilarResolution(KScreen::OutputPtr primary, 
                                         KScreen::OutputPtr second) 
{
    QSize similarSize(0, 0);
    
    return similarSize;
}

void OsdWidget::m_mirror() 
{
    QPoint primaryPos(0, 0);
    KScreen::OutputPtr primary;
    KScreen::OutputPtr second;
    QSize similar(0, 0);

    if (m_config.isNull())
        return;

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output.isNull())
            continue;

        if (!output->isConnected())
            continue;

        if (output->isPrimary() || output->name().contains(lvdsPrefix))
            primary = output;
        else
            second = output;
    }

    similar = m_findSimilarResolution(primary, second);

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output.isNull())
            continue;

        if (!output->isConnected())
            continue;

        if (!output->isEnabled()) {
            output->setEnabled(true);
            if (similar.isNull())
                output->setCurrentModeId(output->preferredModeId());
        }

        if (!similar.isNull())
            output->setSize(similar);

        if (output->isPrimary() || output->name().contains(lvdsPrefix)) {
            output->setPrimary(true);
            primaryPos = output->pos();
        } else {
            output->setPrimary(false);
            output->setPos(primaryPos);
        }
    }

    m_doApplyConfig();
}

#if 0
// FIXME: when second screen only switch to extend, it fails to be extended, 
// but mirror! it should be extended.
void OsdWidget::m_extend() 
{
    QPoint secondPos(0, 0);

    if (m_config.isNull())
        return;

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output.isNull())
            continue;

        if (!output->isConnected())
            continue;

        if (!output->isEnabled()) {
            output->setEnabled(true);
            output->setCurrentModeId(output->preferredModeId());
        }

        if (output->isPrimary() || output->name().contains(lvdsPrefix)) {
            output->setPrimary(true);
            output->setPos(secondPos);
            secondPos.setX(output->pos().x() + output->size().width());
        } else {
            output->setPrimary(false);
            output->setPos(secondPos);
        }
    }

    m_doApplyConfig();
}
#endif

void OsdWidget::m_extend() 
{
    QString primaryName = "";
    QString secondName = "";

    if (m_config.isNull())
        return;

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output.isNull())
            continue;

        if (!output->isConnected())
            continue;

        if (output->isPrimary() || output->name().contains(lvdsPrefix))
            primaryName = output->name();
        else
            secondName = output->name();
    }

    if (primaryName == "" || secondName == "")
        return;

    // xrandr --output LVDS1 --auto --output VGA1 --auto --right-of LVDS1
    KToolInvocation::kdeinitExec(QString("xrandr"), QStringList() 
        << QString("--output") << primaryName << QString("--auto") 
        << QString("--output") << secondName << QString("--auto") 
        << QString("--right-of") << primaryName);
}

#if 0
// FIXME: fail to set second screen only canBeAppled: There are no enabled 
// screens, at least one required
void OsdWidget::m_secondScreenOnly() 
{
    if (m_config.isNull())
        return;
    
    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output.isNull())
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
#endif

void OsdWidget::m_secondScreenOnly() 
{
    QString primaryName = "";
    QString secondName = "";

    if (m_config.isNull())
        return;

    for (KScreen::OutputPtr &output : m_config->outputs()) {
        if (output.isNull())
            continue;

        if (!output->isConnected())
            continue;

        if (output->isPrimary() || output->name().contains(lvdsPrefix))
            primaryName = output->name();
        else 
            secondName = output->name();
    }

    if (primaryName == "" || secondName == "")
        return;

    // xrandr --output LVDS1 --off --output VGA1 --auto
    KToolInvocation::kdeinitExec(QString("xrandr"), QStringList() 
        << QString("--output") << primaryName << QString("--off") 
        << QString("--output") << secondName << QString("--auto"));
}

void OsdWidget::slotItemClicked(QListWidgetItem *item) 
{
    if (item->text() == PC_SCREEN_ONLY_MODE) {
        m_pcScreenOnly();
    } else if (item->text() == MIRROR_MODE) {
        m_mirror();
    } else if (item->text() == EXTEND_MODE) {
        m_extend();
    } else if (item->text() == SECOND_SCREEN_ONLY_MODE) {
        m_secondScreenOnly();
    }

    QCoreApplication::quit();
}
