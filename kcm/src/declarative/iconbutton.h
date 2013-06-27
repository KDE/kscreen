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

#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QGraphicsProxyWidget>

class KPushButton;

class IconButton : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(QString iconName
               READ iconName
               WRITE setIconName
               NOTIFY iconNameChanged)

    Q_PROPERTY(QString text
               READ text
               WRITE setText
               NOTIFY textChanged)

    Q_PROPERTY(int iconSize
               READ iconSize
               WRITE setIconSize
               NOTIFY iconSizeChanged)

    Q_PROPERTY(QString tooltipText
               READ tooltipText
               WRITE setTooltipText
               NOTIFY tooltipTextChanged)

    Q_PROPERTY(bool iconEnabled
               READ iconEnabled
               WRITE setIconEnabled
               NOTIFY iconEnabledChanged)

  public:
    explicit IconButton(QGraphicsItem *parent = 0);
    virtual ~IconButton();

    void setIconName(const QString &iconName);
    QString iconName() const;

    void setText(const QString &text);
    QString text() const;

    void setIconSize(int size);
    int iconSize() const;

    void setTooltipText(const QString &text);
    QString tooltipText() const;

    void setIconEnabled(bool iconEnabled);
    bool iconEnabled() const;

  Q_SIGNALS:
    void iconNameChanged();
    void textChanged();
    void iconSizeChanged();
    void tooltipTextChanged();
    void iconEnabledChanged();

    void clicked();

  private:
    void loadIcon();

    KPushButton *m_button;
    bool m_iconEnabled;
    QString m_iconName;
};

#endif // ICONBUTTON_H
