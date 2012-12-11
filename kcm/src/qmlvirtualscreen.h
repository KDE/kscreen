/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef QMLVIRTUALSCREEN_H
#define QMLVIRTUALSCREEN_H

#include <QDeclarativeItem>


namespace KScreen
{
class Screen;
}

/* Can't use KScreen::Screen directly in QML because of QTBUG-23214 */
class QMLVirtualScreen : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QSize minSize READ minSize CONSTANT)
    Q_PROPERTY(QSize maxSize READ maxSize CONSTANT)

public:
    QMLVirtualScreen(QDeclarativeItem *parent = 0);
    virtual ~QMLVirtualScreen();

    QSize minSize() const;
    QSize maxSize() const;

private:
    KScreen::Screen *m_screen;
};

#endif // QMLVIRTUALSCREEN_H
