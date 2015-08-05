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

#include <QApplication>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QSettings>
#include <QBitmap>
#include <QPainter>
#include <QStyledItemDelegate>

#include <KLocalizedString>
#include <KToolInvocation>

#include <kscreen/output.h>

static const QString PC_SCREEN_ONLY_MODE = i18n("PC");
static const QString MIRROR_MODE = i18n("Mirror");
static const QString EXTEND_MODE = i18n("Extend");
static const QString SECOND_SCREEN_ONLY_MODE = i18n("Second");
static const QSize modeIconSize(106, 110);
static const QPoint outputMargin(20, 20);
static const QSize lineSize(1, modeIconSize.height());

class ItemDelegate : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const
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
    m_pluggedIn(false)
{
    QVBoxLayout *vbox = new QVBoxLayout;

    m_modeList = new QListWidget;
    m_modeList->setFlow(QListView::LeftToRight);
    m_modeList->setViewMode(QListView::IconMode);
    m_modeList->setIconSize(modeIconSize);
    m_modeList->setSpacing(6);
    m_modeList->setFrameStyle(QFrame::NoFrame);
    m_modeList->setItemDelegate(new ItemDelegate(m_modeList));
    m_modeList->viewport()->setAttribute(Qt::WA_Hover);
    connect(m_modeList, &QListWidget::itemClicked,
            this, &OsdWidget::slotItemClicked);
    vbox->addWidget(m_modeList);

    createItem(QStringLiteral("pc-screen-only"), PC_SCREEN_ONLY_MODE);
    createLine();

    createItem(QStringLiteral("mirror"), MIRROR_MODE);
    createLine();

    createItem(QStringLiteral("extend"), EXTEND_MODE);
    createLine();

    createItem(QStringLiteral("second-screen-only"), SECOND_SCREEN_ONLY_MODE);

    QLabel *showMe = new QLabel(QStringLiteral("<a href=\"#\">%1</a>").arg(i18n("Disable automatically popping up?")));
    connect(showMe, &QLabel::linkActivated,
            []() {
                KToolInvocation::kdeinitExec(QStringLiteral("kcmshell5"),
                                             QStringList() << QStringLiteral("kcm_kscreen"));
            });
    vbox->addWidget(showMe);

    setLayout(vbox);

    m_primaryOutputWidget = new OutputWidget(QStringLiteral("1"), this);
    m_secondOutputWidget = new OutputWidget(QStringLiteral("2"), this);
}

OsdWidget::~OsdWidget()
{
}

void OsdWidget::pluggedIn()
{
    m_pluggedIn = true;
}

void OsdWidget::createItem(const QString &iconName, const QString &modeLabel)
{
    QIcon icon;
    icon.addPixmap(QPixmap(QStringLiteral(":/%1.png").arg(iconName)), QIcon::Normal);
    icon.addPixmap(QPixmap(QStringLiteral(":/%1-selected.png").arg(iconName)), QIcon::Selected);
    QListWidgetItem *item = new QListWidgetItem(icon, modeLabel);
    item->setSizeHint(modeIconSize);
    m_modeList->addItem(item);
}

void OsdWidget::createLine()
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

bool OsdWidget::isShowMe()
{
    QSettings settings(QStringLiteral("kscreen"), QStringLiteral("settings"));
    return settings.value(QStringLiteral("osd/showme"), true).toBool();
}

bool OsdWidget::isAbleToShow(const KScreen::ConfigPtr &config)
{
    unsigned int outputConnected = 0;
    bool hasPrimary = false;
    bool primaryEnabled = true;
    bool secondEnabled = true;
    QPoint primaryPos(0, 0);
    QPoint secondPos(0, 0);
    QSize primarySize(0, 0);
    QSize secondSize(0, 0);

    if (!isShowMe()) {
        hideAll();
        return false;
    }

    if (!m_pluggedIn) {
        hideAll();
        return false;
    }

    if (config.isNull()) {
        hideAll();
        return false;
    }

    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        if (output.isNull())
            continue;

        if (!output->isConnected())
            continue;

        if (output->isPrimary()) {
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
        show();

        if (primaryEnabled && !secondEnabled) {
            move((primarySize.width() - width()) / 2, 
                 (primarySize.height() - height()) / 2);
            m_modeList->setCurrentRow(0);
            m_primaryOutputWidget->move(outputMargin);
            m_primaryOutputWidget->show();
        }

        if (primaryEnabled && secondEnabled) {
            if (primaryPos == secondPos) {
                move((primarySize.width() - width()) / 2, 
                     (primarySize.height() - height()) / 2);
                m_modeList->setCurrentRow(2);
                // NOTE: it does not need to move && show primary and second 
                // output widget in mirror mode
            } else {
                // extend mode
                move((primarySize.width() - width()) / 2, 
                     (primarySize.height() - height()) / 2);
                m_modeList->setCurrentRow(4);
                m_primaryOutputWidget->move(outputMargin);
                m_primaryOutputWidget->show();
                m_secondOutputWidget->move(
                    outputMargin.x() + primarySize.width(), outputMargin.y());
                m_secondOutputWidget->show();
            }
        }

        if (!primaryEnabled && secondEnabled) {
            move((secondSize.width() - width()) / 2, 
                 (secondSize.height() - height()) / 2);
            m_modeList->setCurrentRow(6);
            m_secondOutputWidget->move(outputMargin);
            m_secondOutputWidget->show();
        }

        return true;
    }

    if (outputConnected > 2) {
        KToolInvocation::kdeinitExec(QStringLiteral("kcmshell5"),
                                     QStringList() << QStringLiteral("kcm_kscreen"));
    }

    hideAll();

    return false;
}

void OsdWidget::hideAll()
{
    hide();

    if (m_primaryOutputWidget)
        m_primaryOutputWidget->hide();

    if (m_secondOutputWidget)
        m_secondOutputWidget->hide();
}

void OsdWidget::slotItemClicked(QListWidgetItem *item)
{
    m_pluggedIn = false;
    hideAll();

    if (item->text() == PC_SCREEN_ONLY_MODE) {
        emit displaySwitch(Generator::TurnOffExternal);
    } else if (item->text() == MIRROR_MODE) {
        emit displaySwitch(Generator::Clone);
    } else if (item->text() == EXTEND_MODE) {
        emit displaySwitch(Generator::ExtendToRight);
    } else if (item->text() == SECOND_SCREEN_ONLY_MODE) {
        emit displaySwitch(Generator::TurnOffEmbedded);
    }
}

OutputWidget::OutputWidget(const QString &id, QWidget *parent, Qt::WindowFlags f)
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
