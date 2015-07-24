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
#include <QLabel>
#include <QDir>
#include <QSettings>
#include <QBitmap>
#include <QPainter>
#include <QStyledItemDelegate>

#include <KLocalizedString>
#include <KToolInvocation>

#include <kscreen/getconfigoperation.h>

static const QString lvdsPrefix = "LVDS";
static const QString PC_SCREEN_ONLY_MODE = i18n("PC");
static const QString MIRROR_MODE = i18n("Mirror");
static const QString EXTEND_MODE = i18n("Extend");
static const QString SECOND_SCREEN_ONLY_MODE = i18n("Second");
static const QSize modeIconSize(111, 115);
static const QPoint outputMargin(20, 20);
static const QSize lineSize(1, modeIconSize.height());

class ItemDelegate : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter* painter, 
               const QStyleOptionViewItem& option, 
               const QModelIndex& index) const
    {
        Q_ASSERT(index.isValid());

        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);

        opt.decorationPosition = QStyleOptionViewItem::Bottom;

        if (opt.state & QStyle::State_MouseOver)
            opt.icon = opt.icon.pixmap(opt.decorationSize, QIcon::Selected);

        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt,
                                           painter, 0);
    }
};

OsdWidget::OsdWidget(QWidget *parent, Qt::WindowFlags f) 
  : QWidget(parent, f),
    m_modeList(nullptr),
    m_configIsReady(false)
{
    setFixedSize(520, 166);
    QDesktopWidget *desktop = QApplication::desktop();
    move((desktop->width() - width()) / 2, (desktop->height() - height()) / 2);

    setStyleSheet("QListWidget { background: #e7ebf0; }"
                  "QListWidget::item:selected { background: #b5b5b6; }"
                  "QListWidget::item::hover {background: #b5b5b6; }");

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(10, 0, 10, 10);

    m_modeList = new QListWidget;
    m_modeList->setFlow(QListView::LeftToRight);
    m_modeList->setViewMode(QListView::IconMode);
    m_modeList->setIconSize(modeIconSize);
    m_modeList->setSpacing(6);
    m_modeList->setFrameStyle(QFrame::NoFrame);
    m_modeList->setItemDelegate(new ItemDelegate(m_modeList));
    m_modeList->viewport()->setAttribute(Qt::WA_Hover);
    connect(m_modeList, SIGNAL(itemClicked(QListWidgetItem*)), 
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    vbox->addWidget(m_modeList);

    m_createItem("pc-screen-only", PC_SCREEN_ONLY_MODE);
    m_createLine();

    m_createItem("mirror", MIRROR_MODE);
    m_createLine();

    m_createItem("extend", EXTEND_MODE);
    m_createLine();

    m_createItem("second-screen-only", SECOND_SCREEN_ONLY_MODE);

    QLabel *showMe = new QLabel(i18n("<a href=\"#\">Disable automatically popping up?</a>"));
    connect(showMe, &QLabel::linkActivated, [this]() {
                KToolInvocation::kdeinitExec(QString("kcmshell5"), 
                    QStringList() << QString("kcm_kscreen"));
            });
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

void OsdWidget::m_createItem(QString iconName, QString modeLabel)
{
    QIcon icon;
    icon.addPixmap(QPixmap(":/" + iconName + ".png"), QIcon::Normal);
    icon.addPixmap(QPixmap(":/" + iconName + "-selected.png"), QIcon::Selected);
    QListWidgetItem *item = new QListWidgetItem(icon, modeLabel);
    item->setSizeHint(modeIconSize);
    m_modeList->addItem(item);
}

void OsdWidget::m_createLine() 
{
    QListWidgetItem *item = new QListWidgetItem;
    m_modeList->addItem(item);
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::VLine);
    item->setSizeHint(lineSize);
    m_modeList->setItemWidget(item, line);
}

void OsdWidget::paintEvent(QPaintEvent *) 
{
    QBitmap bmp(size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(bmp.rect(), 6, 6);
    setMask(bmp);
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
                m_modeList->setCurrentRow(2);
                // NOTE: it does not need to move && show primary and second 
                // output widget in mirror mode
            } else {
                // extend mode
                QDesktopWidget *desktop = QApplication::desktop();
                move((primarySize.width() - width()) / 2, 
                     (desktop->height() - height()) / 2);
                m_modeList->setCurrentRow(4);
                m_primaryOutputWidget->move(outputMargin);
                m_primaryOutputWidget->show();
                m_secondOutputWidget->move(
                    outputMargin.x() + primarySize.width(), outputMargin.y());
                m_secondOutputWidget->show();
            }
        }

        if (!primaryEnabled && secondEnabled) {
            m_modeList->setCurrentRow(6);
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
    for (KScreen::ModePtr &primaryMode : primary->modes()) {
        for (KScreen::ModePtr &secondMode : second->modes()) {
            if (primaryMode->size() == secondMode->size())
                return primaryMode->size();
        }
    }

    return QSize(0, 0);
}

void OsdWidget::m_mirror() 
{
    KScreen::OutputPtr primary;
    KScreen::OutputPtr second;
    QSize similar(0, 0);
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

        if (output->isPrimary() || output->name().contains(lvdsPrefix))
            primaryName = output->name();
        else
            secondName = output->name();
    }

    if (primaryName == "" || secondName == "")
        return;

    if (similar.isNull()) {
        // xrandr --output LVDS1 --auto --output VGA1 --auto --same-as LVDS1
        KToolInvocation::kdeinitExec(QString("xrandr"), QStringList() 
            << QString("--output") << primaryName << QString("--auto") 
            << QString("--output") << secondName << QString("--auto") 
            << QString("--same-as") << primaryName);
    } else {
        QString mode = QString::number(similar.width()) + "x" + 
            QString::number(similar.height());
        // xrandr --output LVDS1 --mode 1024x768 --output VGA1 --mode 1024x768 
        // --same-as LVDS1
        KToolInvocation::kdeinitExec(QString("xrandr"), QStringList() 
            << QString("--output") << primaryName << QString("--mode") << mode 
            << QString("--output") << secondName << QString("--mode") << mode 
            << QString("--same-as") << primaryName);
    }
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
