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
#include <QLabel>
#include <QCheckBox>
#include <QDir>
#include <QSettings>

#include <KLocalizedString>
#include <KToolInvocation>

#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

static const QString lvdsPrefix = "LVDS";
static const QString PC_SCREEN_ONLY_MODE = i18n("PC screen only");
static const QString MIRROR_MODE = i18n("Mirror");
static const QString EXTEND_MODE = i18n("Extend");
static const QString SECOND_SCREEN_ONLY_MODE = i18n("Second screen only");
static const QSize modeIconSize(111, 112);
static const QPoint outputMargin(20, 20);

OsdWidget::OsdWidget(QWidget *parent, Qt::WindowFlags f) 
  : QWidget(parent, f),
    m_modeList(nullptr),
    m_configIsReady(false)
{
    setFixedSize(520, 176);
    QDesktopWidget *desktop = QApplication::desktop();
    move((desktop->width() - width()) / 2, (desktop->height() - height()) / 2);

    QVBoxLayout *vbox = new QVBoxLayout;

    m_modeList = new QListWidget;
    m_modeList->setFlow(QListView::LeftToRight);
    m_modeList->setViewMode(QListView::IconMode);
    m_modeList->setIconSize(QSize(90, 90));
    m_modeList->setSpacing(6);
    connect(m_modeList, SIGNAL(itemClicked(QListWidgetItem*)), 
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    vbox->addWidget(m_modeList);

    QListWidgetItem *item = new QListWidgetItem(
        QIcon(QPixmap(":/pc-screen-only.png")), PC_SCREEN_ONLY_MODE);
    item->setSizeHint(modeIconSize);
    m_modeList->addItem(item);

    item = new QListWidgetItem(QIcon(QPixmap(":/mirror.png")), MIRROR_MODE);
    item->setSizeHint(modeIconSize);
    m_modeList->addItem(item);

    item = new QListWidgetItem(QIcon(QPixmap(":/extend.png")), EXTEND_MODE);
    item->setSizeHint(modeIconSize);
    m_modeList->addItem(item);

    item = new QListWidgetItem(
        QIcon(QPixmap(":/second-screen-only.png")), SECOND_SCREEN_ONLY_MODE);
    item->setSizeHint(modeIconSize);
    m_modeList->addItem(item);

    QCheckBox *showMe = new QCheckBox(i18n("Still show me next time"));
    if (m_isShowMe())
        showMe->setCheckState(Qt::Checked);
    connect(showMe, SIGNAL(stateChanged(int)), this, SLOT(slotShowMeChanged(int)));
    vbox->addWidget(showMe);

    setLayout(vbox);

    m_primaryOutputWidget = new OutputWidget("1");
    m_secondOutputWidget = new OutputWidget("2");

    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
            this, &OsdWidget::slotConfigReady);
}

OsdWidget::~OsdWidget() 
{
    while (m_modeList->count())
        m_modeList->takeItem(0);

    if (m_primaryOutputWidget) {
        delete m_primaryOutputWidget;
        m_primaryOutputWidget = nullptr;
    }

    if (m_secondOutputWidget) {
        delete m_secondOutputWidget;
        m_secondOutputWidget = nullptr;
    }
}

void OsdWidget::slotShowMeChanged(int state) 
{
    QSettings settings("kscreen", "settings");
    
    if (state == Qt::Unchecked)
        settings.setValue("osd/showme", false);
    else if (state == Qt::Checked)
        settings.setValue("osd/showme", true);
}

bool OsdWidget::m_isShowMe() 
{
    QSettings settings("kscreen", "settings");
    
    QString settingsDir = QDir::homePath() + "/.config/kscreen";
    QDir dir(settingsDir);
    if (!dir.exists()) {
        dir.mkdir(settingsDir);
        return true;
    }

    QString settingsPath = settingsDir + "/settings.conf";
    QFile file(settingsPath);
    if (!file.exists())
        return true;
    
    return settings.value("osd/showme").toBool();
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
    QSize primarySize(0, 0);
    QSize secondSize(0, 0);

    if (!m_isShowMe()) {
        QCoreApplication::quit();
        return false;
    }

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
            primarySize = output->size();
        } else {
            secondEnabled = output->isEnabled();
            secondPos = output->pos();
            secondSize = output->size();
        }

        if (output->isConnected())
            outputConnected++;
    }

    if (hasPrimary && outputConnected == 2) {
        if (primaryEnabled && !secondEnabled) {
            m_modeList->setCurrentRow(0);
            m_primaryOutputWidget->move(outputMargin);
            m_primaryOutputWidget->show();
        }

        if (primaryEnabled && secondEnabled) {
            if (primaryPos == secondPos) {
                m_modeList->setCurrentRow(1);
                // NOTE: it does not need to move && show primary and second 
                // output widget in mirror mode
            } else {
                // extend mode
                QDesktopWidget *desktop = QApplication::desktop();
                move((primarySize.width() - width()) / 2, 
                     (desktop->height() - height()) / 2);
                m_modeList->setCurrentRow(2);
                m_primaryOutputWidget->move(outputMargin);
                m_primaryOutputWidget->show();
                m_secondOutputWidget->move(
                    outputMargin.x() + primarySize.width(), outputMargin.y());
                m_secondOutputWidget->show();
            }
        }

        if (!primaryEnabled && secondEnabled) {
            m_modeList->setCurrentRow(3);
            m_secondOutputWidget->move(outputMargin);
            m_secondOutputWidget->show();
        }

        show();

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

    // xrandr --output LVDS1 --auto --output VGA1 --off
    KToolInvocation::kdeinitExec(QString("xrandr"), QStringList() 
        << QString("--output") << primaryName << QString("--auto") 
        << QString("--output") << secondName << QString("--off"));
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

OutputWidget::OutputWidget(QString id, QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f) 
{
    setFixedSize(90, 90);

    QVBoxLayout *vbox = new QVBoxLayout;
    QFont font;
    font.setPixelSize(90);
    QLabel *label = new QLabel(id);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);

    setLayout(vbox);
}

OutputWidget::~OutputWidget() 
{
}
