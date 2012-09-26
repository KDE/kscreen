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


#ifndef MONITORVIEW_H
#define MONITORVIEW_H

#include <QDeclarativeItem>

#define QML_PATH "kcm_displayconfiguration/qml/"

namespace KScreen {
class Output;
}
class QMLOutput;
class QDeclarativeContext;

class QMLOutputView : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QList<QMLOutput*> outputs READ outputs NOTIFY outputsChanged);
    Q_PROPERTY(QMLOutput* activeOutput READ activeOutput NOTIFY activeOutputChanged);
public:
    QMLOutputView();
    virtual ~QMLOutputView();

    void addOutput(QDeclarativeEngine* engine, KScreen::Output* output);

    QList<QMLOutput*> outputs() const;
    QMLOutput * activeOutput() const;

    Q_INVOKABLE QMLOutput* getPrimaryOutput() const;

Q_SIGNALS:
    void changed();

    /* Property notifications */
    void outputsChanged();
    void activeOutputChanged();

private Q_SLOTS:
    void outputMoved(bool snap);
    void outputClicked();
    void primaryOutputChanged();
    void viewSizeChanged();
    void viewSizeChanged(bool initialPlacement);

private:
    QDeclarativeContext * context() const;

    QList<QMLOutput*> m_outputs;
    QMLOutput *m_activeOutput;

};

#endif // MONITORVIEW_H
