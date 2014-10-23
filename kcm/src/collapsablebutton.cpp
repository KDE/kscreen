/*
 * Copyright 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "collapsablebutton.h"

#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QHBoxLayout>

CollapsableButton::CollapsableButton(const QString &text, QWidget *parent)
    : QWidget(parent)
    , mCollapsed(false)
    , mWidget(0)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    mLabel = new QLabel(text, this);
    layout->addWidget(mLabel);
    QFont f = mLabel->font();
    f.setBold(true);
    mLabel->setFont(f);

    mLabel->setIndent(20);
}

CollapsableButton::~CollapsableButton()
{
}

void CollapsableButton::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        toggle();
        Q_EMIT toggled();
    }

    QWidget::mouseReleaseEvent(ev);
}

void CollapsableButton::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);

    QStyleOption opt;
    const int h = 20;
    opt.rect = QRect(0, (height() - h) / 2, h, h);
    opt.palette = palette();
    QStyle::PrimitiveElement pe = mCollapsed ? QStyle::PE_IndicatorArrowRight : QStyle::PE_IndicatorArrowDown;

    style()->drawPrimitive(pe, &opt, &painter);
    painter.end();

    QWidget::paintEvent(ev);
}

bool CollapsableButton::isCollapsed() const
{
    return mCollapsed;
}

void CollapsableButton::setCollapsed(bool collapsed)
{
    if (mCollapsed == collapsed) {
        return;
    }

    mCollapsed = collapsed;
    if (mWidget) {
        mWidget->setHidden(collapsed);
    }

    update();
}

QLabel* CollapsableButton::label() const
{
    return mLabel;
}

void CollapsableButton::setWidget(QWidget *widget)
{
    mWidget = widget;
    if (mWidget) {
        mWidget->setHidden(isCollapsed());
    }
}

QWidget *CollapsableButton::widget() const
{
    return mWidget;
}

void CollapsableButton::toggle()
{
    setCollapsed(!isCollapsed());
    Q_EMIT toggled();
}
