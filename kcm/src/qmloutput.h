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


#ifndef QMLOUTPUT_H
#define QMLOUTPUT_H

#include <QDeclarativeItem>
#include <kscreen/mode.h>

class QStandardItemModel;
class QAbstractItemModel;

namespace KScreen {
class Output;
}

class QMLOutput : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(KScreen::Output* output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(QMLOutput* cloneOf READ cloneOf WRITE setCloneOf NOTIFY cloneOfChanged)

    Q_PROPERTY(int currentOutputHeight READ currentOutputHeight NOTIFY currentOutputSizeChanged)
    Q_PROPERTY(int currentOutputWidth READ currentOutputWidth NOTIFY currentOutputSizeChanged)

    /* Workaround for possible QML bug when calling output.pos.y = VALUE works,
     * but output.pos.x = VALUE has no effect */
    Q_PROPERTY(int outputX READ outputX WRITE setOutputX)
    Q_PROPERTY(int outputY READ outputY WRITE setOutputY)

    Q_PROPERTY(float displayScale READ displayScale CONSTANT)
public:
    enum {
      ModeRole = Qt::UserRole,
      ModeIdRole,
      SizeRole,
      RefreshRateRole
    };

    QMLOutput();
    virtual ~QMLOutput();

    void setOutput(KScreen::Output* output);
    KScreen::Output* output() const;

    void setCloneOf(QMLOutput *other);
    QMLOutput* cloneOf() const;

    int currentOutputHeight() const;
    int currentOutputWidth() const;

    int outputX() const;
    void setOutputX(int x);

    int outputY() const;
    void setOutputY(int y);

    /**
     * Returns scale in which the output is drawn on the screen.
     *
     * @return Currently we use 1/6th scale
     */
    float displayScale() const;

    Q_INVOKABLE QAbstractItemModel* modesModel();

Q_SIGNALS:
    void changed();

    /* Property notifications */
    void outputChanged();
    void cloneOfChanged();
    void currentOutputSizeChanged();

private:
    /**
     * Returns the biggest resolution available assuming it's the preferred one
     */
    KScreen::Mode* bestMode() const;

    KScreen::Output *m_output;
    QMLOutput *m_cloneOf;

    QStandardItemModel *m_modesModel;
};

#endif // QMLOUTPUT_H

