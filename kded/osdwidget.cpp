/*************************************************************************************
*  Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>                        *
*  Copyright (C) 2015 Dan Vrátil <dvratil@redhat.com>                               *
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
#include <QCheckBox>
#include <QSettings>
#include <QToolButton>
#include <QAction>

#include <KLocalizedString>
#include <KToolInvocation>
#include <KIconLoader>

#include <kscreen/output.h>

static const QPoint outputMargin(20, 20);

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
            return i18n("PC screen only");
        case Generator::Clone:
            return i18n("Mirror");
        case Generator::ExtendToLeft:
            return i18n("Extend");
        case Generator::TurnOffEmbedded:
            return i18n("Second screen only");
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
    m_modeList(Q_NULLPTR),
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

    QCheckBox *showMe = new QCheckBox(QStringLiteral("<a href=\"#\">%1</a>").arg(i18n("Disable automatically popping up?")));
    connect(showMe, &QCheckBox::clicked, [=]() {
                QSettings settings("kscreen", "settings");
                settings.setValue("osd/showme", showMe->isChecked());
            });
    vbox->addWidget(showMe);

    setLayout(vbox);
}

OsdWidget::~OsdWidget() 
{
    clearOutputWidgets();
}

void OsdWidget::clearOutputWidgets()
{
    Q_FOREACH (OutputWidget *outputWidget, m_outputWidgets) {
        if (outputWidget) {
            delete outputWidget;
            outputWidget = Q_NULLPTR;
        }
    }
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

    outputConnected = config->connectedOutputs().size();
    if (outputConnected < 2) {
        hideAll();
        return false;
    }

    clearOutputWidgets();
    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        if (output.isNull())
            continue;

        if (!output->isConnected())
            continue;

        m_outputWidgets << new OutputWidget(output->name(), output->pos());

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
    }

    if (hasPrimary) {
        show();

        if (primaryEnabled && !secondEnabled) {
            // pc screen only
            move((primarySize.width() - width()) / 2, 
                 (primarySize.height() - height()) / 2);
            m_modeList->setCurrentRow(0);

            if (m_outputWidgets.size())
                m_outputWidgets[0]->show();
        }

        if (primaryEnabled && secondEnabled) {
            if (primaryPos == secondPos) {
                // mirror
                move((primarySize.width() - width()) / 2, 
                     (primarySize.height() - height()) / 2);
                m_modeList->setCurrentRow(2);
            } else {
                // extend
                move((primarySize.width() - width()) / 2, 
                     (primarySize.height() - height()) / 2);
                m_modeList->setCurrentRow(4);

                Q_FOREACH (OutputWidget *outputWidget, m_outputWidgets) {
                    if (outputWidget)
                        outputWidget->show();
                }
            }
        }

        if (!primaryEnabled && secondEnabled) {
            // second or Nth screen only
            move((secondSize.width() - width()) / 2, 
                 (secondSize.height() - height()) / 2);
            m_modeList->setCurrentRow(6);
        }

        return true;
    }

    hideAll();

    return false;
}

void OsdWidget::hideAll()
{
    hide();

    Q_FOREACH (OutputWidget *outputWidget, m_outputWidgets) {
        if (outputWidget)
            outputWidget->hide();
    }
}

void OsdWidget::slotItemClicked(OsdWidgetItem *item)
{
    m_pluggedIn = false;
    hideAll();

    emit displaySwitch(item->action());
}

OutputWidget::OutputWidget(const QString &name, 
                           const QPoint &position, 
                           QWidget *parent, 
                           Qt::WindowFlags f)
    : QWidget(parent, f)
{
    setFixedSize(90, 90);

    QVBoxLayout *vbox = new QVBoxLayout;
    QFont font;
    font.setPixelSize(90);
    QLabel *label = new QLabel(name);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);

    setLayout(vbox);

    move(position);
}
