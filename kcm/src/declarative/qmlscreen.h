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

#include <QQuickItem>
#include <QPointer>

class QMLOutput;
class QQmlEngine;

namespace KScreen {
class Output;
class Config;
}

class QMLScreen : public QQuickItem
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

    Q_PROPERTY(float outputScale
               READ outputScale
               NOTIFY outputScaleChanged)

    Q_PROPERTY(QQmlEngine* engine
               MEMBER m_engine)

  public:
    explicit QMLScreen(QQuickItem *parent = 0);
    virtual ~QMLScreen();

    Q_INVOKABLE void addOutput(KScreen::Output *output);

    int connectedOutputsCount() const;
    int enabledOutputsCount() const;

    QMLOutput* primaryOutput() const;
    QList<QMLOutput*> outputs() const;

    QSize maxScreenSize() const;

    float outputScale() const;

    KScreen::Config* config() const;
    void setConfig(KScreen::Config *config);

    void updateOutputsPlacement();
    void setEngine(QQmlEngine* engine);

  Q_SIGNALS:
    void connectedOutputsCountChanged();
    void enabledOutputsCountChanged();
    void primaryOutputChanged();

    void outputScaleChanged();

    void focusedOutputChanged(QMLOutput *output);

  private Q_SLOTS:
    void loadOutputs();

    void outputConnectedChanged();
    void outputEnabledChanged();
    void outputPrimaryChanged();
    void outputPositionChanged();

    void qmlOutputMoved();
    void qmlOutputClicked();

    void viewSizeChanged();

  private:
    void qmlOutputMoved(QMLOutput *qmlOutput);
    void updateCornerOutputs();

    QPointer<KScreen::Config> m_config;
    QHash<KScreen::Output*,QMLOutput*> m_outputMap;
    int m_connectedOutputsCount;
    int m_enabledOutputsCount;

    QQmlEngine* m_engine;
    QMLOutput *m_leftmost, *m_topmost, *m_rightmost, *m_bottommost;

};

#endif // QMLSCREEN_H
