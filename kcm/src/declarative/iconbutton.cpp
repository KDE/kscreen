/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "iconbutton.h"

#include <KPushButton>

IconButton::IconButton(QGraphicsItem *parent):
    QGraphicsProxyWidget(parent),
    m_iconEnabled(true)
{
    m_button = new KPushButton();
    m_button->setAttribute(Qt::WA_NoSystemBackground);
    m_button->setMaximumSize(QSize(24, 24));
    m_button->setFlat(true);
    connect(m_button, SIGNAL(clicked(bool)), this, SIGNAL(clicked()));

    setWidget(m_button);
}

IconButton::~IconButton()
{
}

QString IconButton::iconName() const
{
    return m_iconName;
}

void IconButton::setIconName(const QString &iconName)
{
    if (m_iconName == iconName) {
        return;
    }

    m_iconName = iconName;
    Q_EMIT iconNameChanged();

    loadIcon();
}

QString IconButton::text() const
{
    return m_button->text();
}

void IconButton::setText(const QString &text)
{
    if (m_button->text() == text) {
        return;
    }

    m_button->setText(text);
    Q_EMIT textChanged();
}

int IconButton::iconSize() const
{
    return m_button->iconSize().width();
}

void IconButton::setIconSize(int size)
{
    if (m_button->iconSize().width() == size) {
        return;
    }

    m_button->setIconSize(QSize(size,size));
    Q_EMIT iconSizeChanged();

    m_button->setMinimumSize(QSize(size + 10, size + 10));
    m_button->setMaximumSize(QSize(size + 10, size + 10));
}

QString IconButton::tooltipText() const
{
    return m_button->toolTip();
}

void IconButton::setTooltipText(const QString &text)
{
    if (m_button->toolTip() == text) {
        return;
    }

    m_button->setToolTip(text);
    Q_EMIT tooltipTextChanged();
}

bool IconButton::iconEnabled() const
{
    return m_iconEnabled;
}

void IconButton::setIconEnabled(bool iconEnabled)
{
    m_iconEnabled = iconEnabled;
    Q_EMIT iconEnabledChanged();

    loadIcon();
}

void IconButton::loadIcon()
{
    if (m_iconName.isEmpty()) {
        return;
    }

    const KIcon icon(m_iconName);
    if (m_iconEnabled) {
        m_button->setIcon(icon);
    } else {
        // FIXME: Is there a better way to do this?
        const QIcon disabled = QIcon(icon.pixmap(iconSize(), QIcon::QIcon::Disabled));
        m_button->setIcon(disabled);
    }
}
