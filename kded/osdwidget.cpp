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

#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QToolButton>
#include <QAction>

#include <KLocalizedString>
#include <KToolInvocation>
#include <KIconLoader>

#include <kscreen/output.h>

static const QString PC_SCREEN_ONLY_MODE = i18n("PC");
static const QString MIRROR_MODE = i18n("Mirror");
static const QString EXTEND_MODE = i18n("Extend");
static const QString SECOND_SCREEN_ONLY_MODE = i18n("Second");
static const QSize modeIconSize(106, 110);
static const QPoint outputMargin(20, 20);
static const QSize lineSize(1, modeIconSize.height());

class OsdWidgetItem : public QToolButton
{
public:
    OsdWidgetItem(Generator::DisplaySwitchAction action, QWidget *parent = 0)
        : QToolButton(parent)
        , mAction(action)
    {
        const QString name = nameForAction(action);
        const QString iconName = iconForAction(action);

        setText(name);
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        // This is large times 2.5, where 2.5 is an arbitrary constant that should
        // make this reasonably big even on high-DPI screens
        setIconSize(QSize(IconSize(KIconLoader::Desktop) * 2, IconSize(KIconLoader::Desktop)) * 2);

        QIcon icon;
        icon.addPixmap(QPixmap(QStringLiteral(":/%1.png").arg(iconName)), QIcon::Normal);
        icon.addPixmap(QPixmap(QStringLiteral(":/%1-selected.png").arg(iconName)), QIcon::Active);
        setIcon(icon);
        setAutoRaise(true);
    }

    Generator::DisplaySwitchAction action() const
    {
        return mAction;
    }

private:
    QString nameForAction(Generator::DisplaySwitchAction action) const
    {
        switch (action) {
        case Generator::TurnOffExternal:
            return PC_SCREEN_ONLY_MODE;
        case Generator::Clone:
            return MIRROR_MODE;
        case Generator::ExtendToLeft:
            return EXTEND_MODE;
        case Generator::TurnOffEmbedded:
            return SECOND_SCREEN_ONLY_MODE;
        default:
            Q_ASSERT_X(false, "OsdWidgetItem::nameForAction()", "Unsupported action");
            return QString();
        }
    }

    QString iconForAction(Generator::DisplaySwitchAction action) const
    {
        switch (action) {
        case Generator::TurnOffExternal:
            return QStringLiteral("pc-screen-only");
        case Generator::Clone:
            return QStringLiteral("mirror");
        case Generator::ExtendToLeft:
            return QStringLiteral("extend");
        case Generator::TurnOffEmbedded:
            return QStringLiteral("second-screen-only");
        default:
            Q_ASSERT_X(false, "OsdWidgetItem::iconForAction()", "Unsupported action");
            return QString();
        }
    }

    Generator::DisplaySwitchAction mAction;
};

OsdWidget::OsdWidget(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_modeList(nullptr),
    m_pluggedIn(false)
{
    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    for (Generator::DisplaySwitchAction action : { Generator::TurnOffExternal,
                                                   Generator::Clone,
                                                   Generator::ExtendToLeft,
                                                   Generator::TurnOffEmbedded }) {
        OsdWidgetItem *item = new OsdWidgetItem(action);
        connect(item, &OsdWidgetItem::clicked,
                [this, item](bool) {
                    slotItemClicked(item);
                });
        hbox->addWidget(item);
    }

    vbox->addLayout(hbox);

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
    m_primaryOutputWidget->hide();
    m_secondOutputWidget->hide();
}

void OsdWidget::slotItemClicked(OsdWidgetItem *item)
{
    m_pluggedIn = false;
    hideAll();

    emit displaySwitch(item->action());
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
