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

#ifndef QMLCURSOR_H
#define QMLCURSOR_H

#include <QObject>
#include <QMetaType>

class QDeclarativeView;

/**
 * Wrapper around QCursor that allows to expose current cursor position
 * in QML without using MouseArea
 */
class QMLCursor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int x READ x WRITE setX)
    Q_PROPERTY(int y READ y WRITE setY)

  public:
    QMLCursor(QDeclarativeView *parent = 0);
    virtual ~QMLCursor();

    int x() const;
    void setX(int x);

    int y() const;
    void setY(int y);

};

Q_DECLARE_METATYPE(QMLCursor*)

#endif // QMLCURSOR_H
