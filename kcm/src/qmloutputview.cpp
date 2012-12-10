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


#include "qmloutputview.h"
#include "qmloutputcomponent.h"
#include "qmloutput.h"

#include <QDeclarativeEngine>
#include <qdeclarativeexpression.h>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <KStandardDirs>
#include <KDebug>
#include <QGraphicsScene>

#include <kscreen/output.h>

Q_DECLARE_METATYPE(QMLOutput*);

QMLOutputView::QMLOutputView():
    QDeclarativeItem(),
    m_activeOutput(0)
{
    connect(this, SIGNAL(heightChanged()), SLOT(viewSizeChanged()));
    connect(this, SIGNAL(widthChanged()), SLOT(viewSizeChanged()));
}

QMLOutputView::~QMLOutputView()
{
}

QList<QMLOutput*> QMLOutputView::outputs() const
{
    return m_outputs;
}

QMLOutput* QMLOutputView::activeOutput() const
{
    return m_activeOutput;
}


void QMLOutputView::addOutput(QDeclarativeEngine *engine, KScreen::Output* output)
{
    QMLOutputComponent outputComponent(engine);

    QMLOutput *instance = dynamic_cast<QMLOutput*>(outputComponent.createForOutput(output));
    if (!instance) {
        kWarning() << "Failed to add output" << output->name();
        return;
    }

    instance->setParentItem(this);

    /* Root refers to the root object. We need it in order to set drag range */
    instance->setProperty("viewport", this->property("root"));
    connect(instance, SIGNAL(moved(bool)), this, SLOT(outputMoved(bool)));
    connect(instance, SIGNAL(clicked()), this, SLOT(outputClicked()));
    connect(instance, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(instance, SIGNAL(primaryTriggered()), SLOT(primaryTriggered()));

    m_outputs << instance;
    instance->setProperty("z", m_outputs.count());

    if (!output->isConnected()) {
        return;
    }

    viewSizeChanged(true);

    Q_EMIT outputsChanged();
}

void QMLOutputView::viewSizeChanged()
{
    viewSizeChanged(false);
}


void QMLOutputView::viewSizeChanged(bool initialPlacement)
{
    int disabledOffset = width();

    /* Make a rectangle around all the outputs, center it and then adjust
    * position of all the outputs to be in the middle of the view */
    QRect rect;
    QList< QMLOutput * > positionedOutputs;

    Q_FOREACH (QMLOutput *qmloutput, m_outputs) {
        if (!qmloutput->output()->isConnected()) {
            qmloutput->setX(0);
            qmloutput->setY(0);
            continue;
        }

        if ((initialPlacement && !qmloutput->output()->isEnabled()) ||
            (!qmloutput->output()->isEnabled() && !qmloutput->property("moved").isValid())){
            disabledOffset -= qmloutput->width();
            qmloutput->setX(disabledOffset);
            qmloutput->setY(0);
            continue;
        }

        qmloutput->setX(qmloutput->output()->pos().x() * qmloutput->displayScale());
        qmloutput->setY(qmloutput->output()->pos().y() * qmloutput->displayScale());

        if (qmloutput->x() < rect.left()) {
            rect.setX(qmloutput->x());
        }

        if (qmloutput->x() + qmloutput->width() > rect.right()) {
            rect.setWidth(qmloutput->x() + qmloutput->width());
        }

        if (qmloutput->y() < rect.top()) {
            rect.setY(qmloutput->y());
        }

        if (qmloutput->y() + qmloutput->height() > rect.bottom()) {
            rect.setHeight(qmloutput->y() + qmloutput->height());
        }

        positionedOutputs << qmloutput;
    }

    int offsetX = rect.left() + ((width() - rect.width()) / 2);
    int offsetY = rect.top() + ((height() - rect.height()) / 2);


    Q_FOREACH (QMLOutput *qmloutput, positionedOutputs) {
        qmloutput->setX(offsetX + (qmloutput->output()->pos().x() * qmloutput->displayScale()));
        qmloutput->setY(offsetY + (qmloutput->output()->pos().y() * qmloutput->displayScale()));
    }
}


QMLOutput* QMLOutputView::getPrimaryOutput() const
{
    Q_FOREACH (QMLOutput *output, m_outputs) {
        if (output->output()->isPrimary()) {
            return output;
        }
    }

    return 0;
}

void QMLOutputView::outputClicked()
{
    for (int i = 0; i < m_outputs.count(); i++) {
        QMLOutput *output = m_outputs.at(i);

        /* Find clicked child and move it above all it's siblings */
        if (output == sender()) {
            int z = output->property("z").toInt();
            for (int j = 0; j < m_outputs.count(); j++) {
                int otherZ = m_outputs.at(j)->property("z").toInt();
                if (otherZ > z) {
                    m_outputs.at(j)->setProperty("z", otherZ - 1);
                }
            }
            output->setProperty("z", m_outputs.length());
            output->setProperty("focus", true);
            m_activeOutput = output;
            emit activeOutputChanged(m_activeOutput);

            break;
        }
    }
}


void QMLOutputView::outputMoved(bool snap)
{
    QMLOutput *output = dynamic_cast<QMLOutput*>(sender());
    output->setProperty("moved", true);

    int x = output->x();
    int y = output->y();
    int width = output->width();
    int height = output->height();

    /* FIXME: The size of the active snapping area should depend on size of
    * the output */

    if (snap) {
        Q_FOREACH (QMLOutput *otherOutput, m_outputs) {
            if (otherOutput == output) {
                continue;
            }

            if (!otherOutput->output()->isConnected() || !otherOutput->output()->isEnabled()) {
                continue;
            }

            int x2 = otherOutput->x();
            int y2 = otherOutput->y();
            int height2 = otherOutput->height();
            int width2 = otherOutput->width();
            int centerX = x + (width / 2);
            int centerY = y + (height / 2);
            int centerX2 = x2 + (width2 / 2);
            int centerY2 = y2 + (height2 / 2);

            /* @output is left of @otherOutput */
            if ((x + width > x2 - 30) && (x + width < x2 + 30) &&
                (y + height > y2) && (y < y2 + height2)) {

                output->setX(x2 - width);
                x = output->x();
                centerX = x + (width / 2);
                output->setCloneOf(0);

                /* @output is snapped to @otherOutput on left and their
                * upper sides are aligned */
                if ((x + width == x2) && (y < y2 + 5) && (y > y2 - 5)) {
                    output->setY(y2);
                    break;
                }

                /* @output is snapped to @otherOutput on left and they
                * are centered */
                if ((x + width == x2) && (centerY < centerY2 + 5) && (centerY > centerY2 - 5)) {
                    output->setY(centerY2 - (height / 2));
                    break;
                }

                /* @output is snapped to @otherOutput on left and their
                * bottom sides are aligned */
                if ((x + width == x2) && (y + height < y2 + height2 + 5) && (y + height > y2 + height2 - 5)) {
                    output->setY(y2 + height2 - height);
                    break;
                }
            }


            /* @output is right of @otherOutput */
            if ((x > x2 + width2 - 30) && (x < x2 + width2 + 30) &&
                (y + height > y2) && (y < y2 + height2)) {

                output->setX(x2 + width2);
                x = output->x();
                centerX = x + (width / 2);
                output->setCloneOf(0);

                /* @output is snapped to @otherOutput on right and their
                * upper sides are aligned */
                if ((x == x2 + width2) && (y < y2 + 5) && (y > y2 - 5)) {
                    output->setY(y2);
                    break;
                }

                /* @output is snapped to @otherOutput on right and they
                * are centered */
                if ((x == x2 + width2) && (centerY < centerY2 + 5) && (centerY > centerY2 - 5)) {
                    output->setY(centerY2 - (height / 2));
                    break;
                }

                /* @output is snapped to @otherOutput on right and their
                * bottom sides are aligned */
                if ((x == x2 + width2) && (y + height < y2 + height2 + 5) && (y + height > y2 + height2 -5)) {
                    output->setY(y2 + height2 - height);
                    break;
                }
            }


            /* @output is above @otherOutput */
            if ((y + height > y2 - 30) && (y + height < y2 + 30) &&
                (x + width > x2) && (x < x2 + width2)) {

                output->setY(y2 - height);
                y = output->y();
                centerY = y + (height / 2);
                output->setCloneOf(0);

                /* @output is snapped to @otherOutput on top and their
                * left sides are aligned */
                if ((y + height == y2) && (x < x2 + 5) && (x > x2 - 5)) {
                    output->setX(x2);
                    break;
                }

                /* @output is snapped to @otherOutput on top and they
                * are centered */
                if ((y + height == y2) && (centerX < centerX2 + 5) && (centerX > centerX2 - 5)) {
                    output->setX(centerX2 - (width / 2));
                    break;
                }

                /* @output is snapped to @otherOutput on top and their
                * right sides are aligned */
                if ((y + height == y2) && (x + width < x2 + width2 + 5) && (x + width > x2 + width2 - 5)) {
                    output->setX(x2 + width2 - width);
                    break;
                }
            }


            /* @output is below @otherOutput */
            if ((y > y2 + height2 - 30) && (y < y2 + height2 + 30) &&
                (x + width > x2) && (x < x2 + width2)) {

                output->setY(y2 + height2);
                y = output->y();
                centerY = y + (height / 2);
                output->setCloneOf(0);

                /* @output is snapped to @otherOutput on bottom and their
                * left sides are aligned */
                if ((y == y2 + height2) && (x < x2 + 5) && (x > x2 - 5)) {
                    output->setX(x2);
                    break;
                }

                /* @output is snapped to @otherOutput on bottom and they
                * are centered */
                if ((y == y2 + height2) && (centerX < centerX2 + 5) && (centerX > centerX2 - 5)) {
                    output->setX(centerX2 - (width / 2));
                    break;
                }

                /* @output is snapped to @otherOutput on bottom and their
                * right sides are aligned */
                if ((y == y2 + height2) && (x + width < x2 + width2 + 5) && (x + width > x2 + width2 - 5)) {
                    output->setX(x2 + width2 - width);
                    break;
                }
            }


            /* @output is to be clone of @otherOutput (left top corners
            * are aligned */
            if ((x > x2) && (x < x2 + 10) &&
                (y > y2) && (y < y2 + 10)) {

                output->setY(y2);
                output->setX(x2);

                /* Find the most common cloned output and set this
                * monitor to be clone of it as well */
                QMLOutput *cloned = otherOutput->cloneOf();
                if (cloned == 0) {
                    output->setCloneOf(otherOutput);
                } else {
                    while (cloned) {
                        if (!cloned->cloneOf() && (cloned != output)) {
                            output->setCloneOf(cloned);
                            break;
                        }

                        cloned = cloned->cloneOf();
                    }
                }

                break;
            }

            /* If the item did not match any of the conditions
            * above then it's not a clone either :) */
            output->setCloneOf(0);
        }
    }

    if (output->cloneOf() != 0) {
        QList<int> clones = output->cloneOf()->output()->clones();
        if (!clones.contains(output->output()->id())) {
            clones << output->output()->id();
        }

        /* Reset position of the cloned screen and current screen and
        * don't care about any further positioning */
        output->output()->setPos(QPoint(0, 0));
        output->cloneOf()->output()->setPos(QPoint(0, 0));

        return;
    }

    /* Left-most and top-most outputs. Other outputs are positioned
    *relatively to these */
    QMLOutput *topMostOutput = 0;
    QMLOutput *leftMostOutput = 0;

    Q_FOREACH (QMLOutput *otherOutput, m_outputs) {
        if (!otherOutput->output()->isConnected() || !otherOutput->output()->isEnabled()) {
            continue;
        }

        if (!leftMostOutput || (otherOutput->x() < leftMostOutput->x())) {
            leftMostOutput = otherOutput;
        }

        if (!topMostOutput || (otherOutput->y() < topMostOutput->y())) {
            topMostOutput = otherOutput;
        }
    }

    if (leftMostOutput != 0) {
        QPoint pos = leftMostOutput->output()->pos();
        pos.setX(0);
        leftMostOutput->output()->setPos(pos);
    }

    if (topMostOutput != 0) {
        QPoint pos = topMostOutput->output()->pos();
        pos.setY(0);
        topMostOutput->output()->setPos(pos);
    }

    /* If the leftmost output is currently being moved, then reposition
    * all output relatively to it, otherwise reposition the current output
    * relatively to the leftmost output */
    if (output == leftMostOutput) {
        Q_FOREACH (QMLOutput *otherOutput, m_outputs) {
            if (otherOutput == leftMostOutput) {
                continue;
            }

            if (!otherOutput->output()->isConnected() ||
                !otherOutput->output()->isEnabled()) {
                continue;
            }

            int x = otherOutput->x() - leftMostOutput->x();

            QPoint pos = otherOutput->output()->pos();
            pos.setX(x / otherOutput->displayScale());
            otherOutput->output()->setPos(pos);
        }
    } else if (leftMostOutput != 0) {
        int x = output->x() - leftMostOutput->x();

        QPoint pos = output->output()->pos();
        pos.setX(x / output->displayScale());
        output->output()->setPos(pos);
    }

    /* If the topmost output is currently being moved, then reposition
    * all outputs relatively to it, otherwise reposition the current output
    * relatively to the topmost output */
    if (output == topMostOutput) {
        Q_FOREACH (QMLOutput *otherOutput, m_outputs) {
            if (otherOutput == topMostOutput) {
                continue;
            }

            if (!otherOutput->output()->isConnected() ||
                !otherOutput->output()->isEnabled()) {
                continue;
            }

            int y = otherOutput->y() - topMostOutput->y();

            QPoint pos = otherOutput->output()->pos();
            pos.setY(y / otherOutput->displayScale());
            otherOutput->output()->setPos(pos);
        }
    } else if (topMostOutput != 0) {
        int y = output->y() - topMostOutput->y();

        QPoint pos = output->output()->pos();
        pos.setY(y / output->displayScale());
        output->output()->setPos(pos);
    }

    /*
    kDebug() << "Left most:" << leftMostOutput->output()->name()
            << "Top most:" << topMostOutput->output()->name();
    Q_FOREACH (QMLOutput *otherOutput, m_outputs) {
            if (!otherOutput->output()->isConnected() ||
                !otherOutput->output()->isEnabled()) {
                    continue;
            }

            kDebug() << otherOutput->output()->name() << otherOutput->output()->pos();
    }
    */
}

void QMLOutputView::primaryTriggered()
{
    QMLOutput *newPrimary = dynamic_cast<QMLOutput*>(sender());

    /* Unset primary flag on all other outputs */
    Q_FOREACH(QMLOutput *qmlOutput, m_outputs) {
        if (qmlOutput != newPrimary) {
            qmlOutput->output()->setPrimary(false);
        }
    }
}


QDeclarativeContext* QMLOutputView::context() const
{
    QList< QGraphicsView* > views;
    QDeclarativeView *view;

    views = scene()->views();
    if (views.count() == 0) {
        kWarning() << "This view is not in any scene!";
        return 0;
    }

    view = dynamic_cast< QDeclarativeView* >(views.at(0));
    return view->rootContext();
}
