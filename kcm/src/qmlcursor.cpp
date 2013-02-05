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

#include "qmlcursor.h"

#include <QDeclarativeView>
#include <QCursor>

QMLCursor::QMLCursor(QDeclarativeView *parent):
    QObject(parent)
{

}

QMLCursor::~QMLCursor()
{

}

int QMLCursor::x() const
{
    const QWidget *widget = qobject_cast<const QWidget*>(parent());
    if (!widget) {
        return QCursor::pos().x();
    }

    return widget->mapFromGlobal(QCursor::pos()).x();
}

void QMLCursor::setX(int x)
{
    QPoint pos = QCursor::pos();
    pos.setX(x);

    const QWidget *widget = qobject_cast<const QWidget*>(parent());
    if (!widget) {
        QCursor::setPos(pos);
        return;
    }

    QCursor::setPos(widget->mapToGlobal(pos));
}


int QMLCursor::y() const
{
    const QWidget *widget = qobject_cast<const QWidget*>(parent());
    if (!widget) {
        return QCursor::pos().y();
    }

    return widget->mapFromGlobal(QCursor::pos()).y();
}

void QMLCursor::setY(int y)
{
    QPoint pos = QCursor::pos();
    pos.setY(y);

    const QWidget *widget = qobject_cast<const QWidget*>(parent());
    if (!widget) {
        QCursor::setPos(pos);
        return;
    }

    QCursor::setPos(widget->mapToGlobal(pos));
}


#include "qmlcursor.moc"
