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


#ifndef QMLOUTPUTCOMPONENT_H
#define QMLOUTPUTCOMPONENT_H

#include <QDeclarativeComponent>

class QDeclarativeEngine;
class QMLOutput;

namespace KScreen {
class Output;
}

class QMLOutputComponent : public QDeclarativeComponent
{
    Q_OBJECT

public:
    explicit QMLOutputComponent(QDeclarativeEngine* engine, QObject* parent = 0);
    virtual ~QMLOutputComponent();

    QMLOutput* createForOutput(KScreen::Output* output);

private:
    QDeclarativeEngine *m_engine;

};

#endif // QMLOUTPUTCOMPONENT_H
