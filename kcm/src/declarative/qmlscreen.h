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

#include <kscreen/output.h>
#include "qmloutput.h"


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

    Q_PROPERTY(int connectedOutputsCount
               READ connectedOutputsCount
               NOTIFY connectedOutputsCountChanged)

    Q_PROPERTY(int enabledOutputsCount
               READ enabledOutputsCount
               NOTIFY enabledOutputsCountChanged)

    Q_PROPERTY(float outputScale
               READ outputScale
               NOTIFY outputScaleChanged)

  public:
    explicit QMLScreen(QQuickItem *parent = nullptr);
    ~QMLScreen() override;

    int connectedOutputsCount() const;
    int enabledOutputsCount() const;

    QMLOutput* primaryOutput() const;
    QList<QMLOutput*> outputs() const;

    QSize maxScreenSize() const;

    float outputScale() const;

    KScreen::ConfigPtr config() const;
    void setConfig(const KScreen::ConfigPtr &config);

    void updateOutputsPlacement();

    void setActiveOutput(QMLOutput *output);

  public Q_SLOTS:
    void setActiveOutput() {
        setActiveOutput(qobject_cast<QMLOutput*>(sender()));
    }


  Q_SIGNALS:
    void connectedOutputsCountChanged();
    void enabledOutputsCountChanged();

    void outputScaleChanged();

    void focusedOutputChanged(QMLOutput *output);

  private Q_SLOTS:
    void addOutput(const KScreen::OutputPtr &output);
    void removeOutput(int outputId);

    void outputConnectedChanged();
    void outputEnabledChanged();
    void outputPositionChanged();

    void viewSizeChanged();

  private:
    void qmlOutputMoved(QMLOutput *qmlOutput);
    void updateCornerOutputs();
    void setOutputScale(float scale);

    KScreen::ConfigPtr m_config;
    QHash<KScreen::OutputPtr,QMLOutput*> m_outputMap;
    QVector<QMLOutput*> m_manuallyMovedOutputs;
    int m_connectedOutputsCount = 0;
    int m_enabledOutputsCount = 0;
    float m_outputScale = 1.0 / 8.0;

    QMLOutput *m_leftmost = nullptr;
    QMLOutput *m_topmost = nullptr;
    QMLOutput *m_rightmost = nullptr;
    QMLOutput *m_bottommost = nullptr;

};

#endif // QMLSCREEN_H
