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

#ifndef COLLAPSABLE_BUTTON_H
#define COLLAPSABLE_BUTTON_H

#include <QLabel>

class CollapsableButton : public QWidget
{
    Q_OBJECT

  public:
    explicit CollapsableButton(const QString &text, QWidget *parent = nullptr);
    virtual ~CollapsableButton();

    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

    void setWidget(QWidget *widget);
    QWidget* widget() const;

    QLabel* label() const;

  public Q_SLOTS:
    void toggle();

  Q_SIGNALS:
    void toggled();

  protected:
    void paintEvent(QPaintEvent *ev) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;

  private:
    bool mCollapsed;
    QLabel *mLabel;
    QWidget *mWidget;
};

#endif // COLLAPSABLE_BUTTON_H
