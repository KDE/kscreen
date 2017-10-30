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

#include <QGraphicsScene>

#include <QTimer>
#include <sys/socket.h>

//static void NullDeleter(KScreen::Output */*output*/) { }

QMLScreen::QMLScreen(QQuickItem *parent):
    QQuickItem(parent),
    m_config(nullptr),
    m_connectedOutputsCount(0),
    m_enabledOutputsCount(0),
    m_leftmost(nullptr),
    m_topmost(nullptr),
    m_rightmost(nullptr),
    m_bottommost(nullptr)
{
    connect(this, &QMLScreen::widthChanged, this, &QMLScreen::viewSizeChanged);
    connect(this, &QMLScreen::heightChanged, this, &QMLScreen::viewSizeChanged);
}

QMLScreen::~QMLScreen()
{
}

KScreen::ConfigPtr QMLScreen::config() const
{
    return m_config;
}

void QMLScreen::setConfig(const KScreen::ConfigPtr &config)
{
    qDeleteAll(m_outputMap);
    m_outputMap.clear();
    m_bottommost = m_leftmost = m_rightmost = m_topmost = 0;
    m_connectedOutputsCount = 0;
    m_enabledOutputsCount = 0;

    if (m_config) {
        m_config->disconnect(this);
    }

    m_config = config;
    connect(m_config.data(), &KScreen::Config::outputAdded,
            this, [this](const KScreen::OutputPtr &output) {
                addOutput(output);
                updateOutputsPlacement();
            });
    connect(m_config.data(), &KScreen::Config::outputRemoved,
            this, &QMLScreen::removeOutput);

    for (const KScreen::OutputPtr &output : m_config->outputs()) {
        addOutput(output);
    }

    updateOutputsPlacement();

    for (QMLOutput *qmlOutput : m_outputMap) {
        if (qmlOutput->output()->isConnected() && qmlOutput->output()->isEnabled()) {
            qmlOutput->dockToNeighbours();
        }
    }
}


void QMLScreen::addOutput(const KScreen::OutputPtr &output)
{
    //QQuickItem *container = findChild<QQuickItem*>(QLatin1String("outputContainer"));

    QMLOutputComponent comp(m_engine, this);
    QMLOutput *qmloutput = comp.createForOutput(output);
    if (!qmloutput) {
        qWarning() << "Failed to create QMLOutput";
        return;
    }

    m_outputMap.insert(output, qmloutput);

    qmloutput->setParentItem(this);
    qmloutput->setZ(m_outputMap.count());

    connect(output.data(), &KScreen::Output::isConnectedChanged,
            this, &QMLScreen::outputConnectedChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            this, &QMLScreen::outputEnabledChanged);
    connect(output.data(), &KScreen::Output::posChanged,
            this, &QMLScreen::outputPositionChanged);
    connect(qmloutput, &QMLOutput::yChanged,
            [this, qmloutput]() {
                qmlOutputMoved(qmloutput);
            });
    connect(qmloutput, &QMLOutput::xChanged,
            [this, qmloutput]() {
                qmlOutputMoved(qmloutput);
            });
    connect(qmloutput, SIGNAL(clicked()),
            this, SLOT(setActiveOutput()));

    qmloutput->updateRootProperties();
}

void QMLScreen::removeOutput(int outputId)
{
    for (const KScreen::OutputPtr &output : m_outputMap.keys()) {
        if (output->id() == outputId) {
            QMLOutput *qmlOutput = m_outputMap.take(output);
            qmlOutput->setParentItem(Q_NULLPTR);
            qmlOutput->setParent(Q_NULLPTR);
            qmlOutput->deleteLater();
            return;
        }
    }
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

    return nullptr;
}

QList<QMLOutput*> QMLScreen::outputs() const
{
    return m_outputMap.values();
}


void QMLScreen::setActiveOutput(QMLOutput *output)
{
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->z() > output->z()) {
            qmlOutput->setZ(qmlOutput->z() - 1);
        }
    }

    output->setZ(m_outputMap.count());
    output->setFocus(true);
    Q_EMIT focusedOutputChanged(output);
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

    Q_FOREACH (const KScreen::OutputPtr &output, m_outputMap.keys()) {
        if (output->isConnected()) {
            ++connectedCount;
        }
    }

    if (connectedCount != m_connectedOutputsCount) {
        m_connectedOutputsCount = connectedCount;
        Q_EMIT connectedOutputsCountChanged();
        updateOutputsPlacement();
    }
}

void QMLScreen::outputEnabledChanged()
{
    const KScreen::OutputPtr output(qobject_cast<KScreen::Output*>(sender()), [](void *){});
    if (output->isEnabled()) {
        updateOutputsPlacement();
    }
    int enabledCount = 0;

    Q_FOREACH (const KScreen::OutputPtr &output, m_outputMap.keys()) {
        if (output->isEnabled()) {
            ++enabledCount;
        }
    }

    if (enabledCount == m_enabledOutputsCount) {
        m_enabledOutputsCount = enabledCount;
        Q_EMIT enabledOutputsCountChanged();
    }
}

void QMLScreen::outputPositionChanged()
{
    /* TODO: Reposition the QMLOutputs */
}

void QMLScreen::qmlOutputMoved(QMLOutput *qmlOutput)
{
    if (qmlOutput->isCloneMode()) {
        return;
    }

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

    Q_FOREACH (QQuickItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
        if (!qmlOutput->output()->isConnected()) {
            continue;
        }

        if (!qmlOutput->output()->isEnabled()) {
            qmlOutput->blockSignals(true);
            disabledOffsetX -= qmlOutput->width();
            qmlOutput->setPosition(QPoint(disabledOffsetX, 0));
            qmlOutput->blockSignals(false);
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

    Q_FOREACH (QQuickItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
        if (!qmlOutput->output()->isConnected() || !qmlOutput->output()->isEnabled()) {
            continue;
        }

        qmlOutput->blockSignals(true);
        qmlOutput->setPosition(QPointF(offset.x() + (qmlOutput->outputX() * outputScale()),
                          offset.y() + (qmlOutput->outputY() * outputScale())));
        qmlOutput->blockSignals(false);
    }
}

void QMLScreen::setEngine(QQmlEngine* engine)
{
    m_engine = engine;
}
