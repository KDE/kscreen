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

#ifndef QMLSCREEN_H
#define QMLSCREEN_H

#include <QtDeclarative/QDeclarativeItem>

class QMLOutput;

namespace KScreen {
class Output;
}

class QMLScreen : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QSize maxScreenSize
               READ maxScreenSize
               CONSTANT)

    Q_PROPERTY(QMLOutput* primaryOutput
               READ primaryOutput
               NOTIFY primaryOutputChanged)

    Q_PROPERTY(int connectedOutputsCount
               READ connectedOutputsCount
               NOTIFY connectedOutputsCountChanged)

    Q_PROPERTY(int enabledOutputsCount
               READ enabledOutputsCount
               NOTIFY enabledOutputsCountChanged)

  public:
    explicit QMLScreen(QDeclarativeItem *parent = 0);
    virtual ~QMLScreen();

    Q_INVOKABLE void addOutput(QDeclarativeEngine *engine, KScreen::Output *output);

    int connectedOutputsCount() const;
    int enabledOutputsCount() const;

    QMLOutput* primaryOutput() const;

    QSize maxScreenSize() const;

  private Q_SLOTS:
    void outputConnectedChanged();
    void outputEnabledChanged();
    void outputPrimaryChanged();
    void outputPositionChanged();

    void qmlOutputMoved();

  Q_SIGNALS:
    void connectedOutputsCountChanged();
    void enabledOutputsCountChanged();
    void primaryOutputChanged();

  private:
    void qmlOutputMoved(QMLOutput *qmlOutput);
    void updateCornerOutputs();

    QHash<KScreen::Output*,QMLOutput*> m_outputMap;
    int m_connectedOutputsCount;
    int m_enabledOutputsCount;

    QMLOutput *m_leftmost, *m_topmost, *m_rightmost, *m_bottommost;

};

#endif // QMLSCREEN_H
