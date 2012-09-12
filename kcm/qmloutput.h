/*
    <one line to give the library's name and an idea of what it does.>
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


#ifndef QMLOUTPUT_H
#define QMLOUTPUT_H

#include <QDeclarativeItem>
#include <kscreen/mode.h>

class /*KScreen::*/Output;

class QMLOutput : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(/*KScreen::*/Output* output READ output WRITE setOutput NOTIFY outputChanged);
    Q_PROPERTY(QDeclarativeListProperty</*KScreen::*/Mode> modes READ modes CONSTANT);
public:
    QMLOutput();
    virtual ~QMLOutput();

    void setOutput(/*KScreen::*/Output* output);
    /*KScreen::*/Output* output() const;

    QDeclarativeListProperty</*KScreen::*/Mode> modes();

Q_SIGNALS:
    void outputChanged();

private:
    /*KScreen::*/Output* m_output;
    QList<Mode*> m_modes;
};

#endif // QMLOUTPUT_H
