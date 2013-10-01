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

#include "qmlscreen.h"
#include "qmloutputcomponent.h"
#include "qmloutput.h"

#include <kscreen/output.h>
#include <kscreen/config.h>
#include <kscreen/configmonitor.h>

#include <QGraphicsScene>
#include <QDeclarativeView>

#include <KDebug>
#include <QTimer>
#include <sys/socket.h>

QMLScreen::QMLScreen(QDeclarativeItem *parent):
    QDeclarativeItem(parent),
    m_connectedOutputsCount(0),
    m_enabledOutputsCount(0),
    m_leftmost(0),
    m_topmost(0),
    m_rightmost(0),
    m_bottommost(0)
{
    m_config = KScreen::Config::current();
    KScreen::ConfigMonitor::instance()->addConfig(m_config);

    connect(this, SIGNAL(widthChanged()), this, SLOT(viewSizeChanged()));
    connect(this, SIGNAL(heightChanged()), this, SLOT(viewSizeChanged()));

    QTimer::singleShot(0, this, SLOT(loadOutputs()));
}

QMLScreen::~QMLScreen()
{
}

void QMLScreen::addOutput(QDeclarativeEngine *engine, KScreen::Output *output)
{
    //QDeclarativeItem *container = findChild<QDeclarativeItem*>(QLatin1String("outputContainer"));

    QMLOutputComponent comp(engine, this);
    QMLOutput *qmloutput = comp.createForOutput(output);
    if (!qmloutput) {
        kWarning() << "Failed to create QMLOutput";
        return;
    }

    m_outputMap.insert(output, qmloutput);

    qmloutput->setParentItem(this);
    qmloutput->setZValue(m_outputMap.count());

    connect(output, SIGNAL(isConnectedChanged()),
            this, SLOT(outputConnectedChanged()));
    connect(output, SIGNAL(isEnabledChanged()),
            this, SLOT(outputEnabledChanged()));
    connect(output, SIGNAL(isPrimaryChanged()),
            this, SLOT(outputPrimaryChanged()));
    connect(output, SIGNAL(posChanged()),
            this, SLOT(outputPositionChanged()));
    connect(qmloutput, SIGNAL(yChanged()),
            this, SLOT(qmlOutputMoved()));
    connect(qmloutput, SIGNAL(xChanged()),
            this, SLOT(qmlOutputMoved()));
    connect(qmloutput, SIGNAL(clicked()),
            this, SLOT(qmlOutputClicked()));
}

void QMLScreen::loadOutputs()
{
    const QList<QGraphicsView*> views = scene()->views();
    Q_ASSERT(!views.isEmpty());
    QDeclarativeView *view = qobject_cast<QDeclarativeView*>(views.first());
    Q_ASSERT(view);

    Q_FOREACH (KScreen::Output *output, m_config->outputs()) {
        addOutput(view->engine(), output);
    }

    updateOutputsPlacement();
}

int QMLScreen::connectedOutputsCount() const
{
    return m_connectedOutputsCount;
}

int QMLScreen::enabledOutputsCount() const
{
    return m_enabledOutputsCount;
}

QMLOutput *QMLScreen::primaryOutput() const
{
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->output()->isPrimary()) {
            return qmlOutput;
        }
    }

    return 0;
}

void QMLScreen::qmlOutputClicked()
{
    QMLOutput *clickedOutput = qobject_cast<QMLOutput*>(sender());
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->zValue() > clickedOutput->zValue()) {
            qmlOutput->setZValue(qmlOutput->zValue() - 1);
        }
    }

    clickedOutput->setZValue(m_outputMap.count());
    clickedOutput->setFocus(true);
    Q_EMIT focusedOutputChanged(clickedOutput);
}

QSize QMLScreen::maxScreenSize() const
{
    return m_config->screen()->maxSize();
}

float QMLScreen::outputScale() const
{
    return 1.0 / 8.0;
}

void QMLScreen::outputConnectedChanged()
{
    int connectedCount = 0;

    Q_FOREACH (KScreen::Output *output, m_outputMap.keys()) {
        if (output->isConnected()) {
            ++connectedCount;
        }
    }

    if (connectedCount != m_connectedOutputsCount) {
        m_connectedOutputsCount = connectedCount;
        Q_EMIT connectedOutputsCountChanged();
    }
}

void QMLScreen::outputEnabledChanged()
{
    /* TODO: Update position of QMLOutput */
    qmlOutputMoved(m_outputMap.value(qobject_cast<KScreen::Output*>(sender())));

    int enabledCount = 0;

    Q_FOREACH (KScreen::Output *output, m_outputMap.keys()) {
        if (output->isEnabled()) {
            ++enabledCount;
        }
    }

    if (enabledCount == m_enabledOutputsCount) {
        m_enabledOutputsCount = enabledCount;
        Q_EMIT enabledOutputsCountChanged();
    }
}

void QMLScreen::outputPrimaryChanged()
{
    KScreen::Output *newPrimary = qobject_cast<KScreen::Output*>(sender());
    if (!newPrimary->isPrimary()) {
        return;
    }

    Q_FOREACH (KScreen::Output *output, m_outputMap.keys()) {
        if (output == newPrimary) {
            continue;
        }

        output->setPrimary(false);
    }

    Q_EMIT primaryOutputChanged();
}

void QMLScreen::outputPositionChanged()
{
    /* TODO: Reposition the QMLOutputs */
}

void QMLScreen::qmlOutputMoved()
{
    qmlOutputMoved(qobject_cast<QMLOutput*>(sender()));
}

void QMLScreen::qmlOutputMoved(QMLOutput *qmlOutput)
{
    updateCornerOutputs();

    if (m_leftmost) {
        m_leftmost->setOutputX(0);
    }
    if (m_topmost) {
        m_topmost->setOutputY(0);
    }

    if (qmlOutput == m_leftmost) {
        Q_FOREACH (QMLOutput *other, m_outputMap) {
            if (other == m_leftmost) {
                continue;
            }

            if (!other->output()->isConnected() || !other->output()->isEnabled()) {
                continue;
            }

            other->setOutputX(float(other->x() - m_leftmost->x()) / outputScale());
        }
    } else if (m_leftmost) {
        qmlOutput->setOutputX(float(qmlOutput->x() - m_leftmost->x()) / outputScale());
    }

    if (qmlOutput == m_topmost) {
        Q_FOREACH (QMLOutput *other, m_outputMap) {
            if (other == m_topmost) {
                continue;
            }

            if (!other->output()->isConnected() || !other->output()->isEnabled()) {
                continue;
            }

            other->setOutputY(float(other->y() - m_topmost->y()) / outputScale());
        }
    } else if (m_topmost) {
        qmlOutput->setOutputY(float(qmlOutput->y() - m_topmost->y()) / outputScale());
    }
}

void QMLScreen::viewSizeChanged()
{
    updateOutputsPlacement();
}

void QMLScreen::updateCornerOutputs()
{
    m_leftmost = 0;
    m_topmost = 0;
    m_rightmost = 0;
    m_bottommost = 0;

    Q_FOREACH (QMLOutput *output, m_outputMap) {
        if (!output->output()->isConnected() || !output->output()->isEnabled()) {
            continue;
        }

        QMLOutput *other = m_leftmost;
        if (!other || output->x() < other->x()) {
            m_leftmost = output;
        }

        if (!other || output->y() < other->y()) {
            m_topmost = output;
        }

        if (!other || output->x() + output->width() > other->x() + other->width()) {
            m_rightmost = output;
        }

        if (!other || output->y() + output->height() > other->y() + other->height()) {
            m_bottommost = output;
        }
    }
}

void QMLScreen::updateOutputsPlacement()
{
    int disabledOffsetX = width();
    QSizeF activeScreenSize;

    Q_FOREACH (QGraphicsItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
        if (!qmlOutput->output()->isConnected()) {
            continue;
        }

        if (!qmlOutput->output()->isEnabled()) {
            disabledOffsetX -= qmlOutput->width();
            qmlOutput->setPos(disabledOffsetX, 0);
            continue;
        }

        if (qmlOutput->outputX() + qmlOutput->currentOutputWidth() > activeScreenSize.width()) {
            activeScreenSize.setWidth(qmlOutput->outputX() + qmlOutput->currentOutputWidth());
        }
        if (qmlOutput->outputY() + qmlOutput->currentOutputHeight() > activeScreenSize.height()) {
            activeScreenSize.setHeight(qmlOutput->outputY() + qmlOutput->currentOutputHeight());
        }
    }

    activeScreenSize *= outputScale();

    const QPointF offset((width() - activeScreenSize.width()) / 2.0,
                         (height() - activeScreenSize.height()) / 2.0);

    Q_FOREACH (QGraphicsItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
        if (!qmlOutput->output()->isConnected() || !qmlOutput->output()->isEnabled()) {
            continue;
        }

        qmlOutput->blockSignals(true);
        qmlOutput->setPos(offset.x() + (qmlOutput->outputX() * outputScale()),
                          offset.y() + (qmlOutput->outputY() * outputScale()));
        qmlOutput->blockSignals(false);
    }
}

KScreen::Config *QMLScreen::config() const
{
    return m_config;
}


#include "qmlscreen.moc"
