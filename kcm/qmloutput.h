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
    Q_PROPERTY(QDeclarativeListProperty</*KScreen::*/Mode> modes READ modes NOTIFY outputChanged);
    Q_PROPERTY(QMLOutput* cloneOf READ cloneOf WRITE setCloneOf NOTIFY cloneOfChanged);
public:
    QMLOutput();
    virtual ~QMLOutput();

    void setOutput(/*KScreen::*/Output* output);
    /*KScreen::*/Output* output() const;

    void setCloneOf(QMLOutput *other);
    QMLOutput* cloneOf() const;


    QDeclarativeListProperty</*KScreen::*/Mode> modes();

    Q_INVOKABLE QStringList getResolutions() const;
    /* Can't use QList<float>, see QTBUG-20826 */
    Q_INVOKABLE QList<QVariant> getRefreshRatesForResolution(const QString &res);

    Q_INVOKABLE void setMode(const QString &resolution, const float &refreshRate);
Q_SIGNALS:
    void changed();

    /* Property notifications */
    void outputChanged();
    void cloneOfChanged();

private:
    /*KScreen::*/Output* m_output;
    QMLOutput *m_cloneOf;
    QList<Mode*> m_modes;
};

#endif // QMLOUTPUT_H

