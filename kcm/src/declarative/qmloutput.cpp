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


#include "qmloutput.h"
#include "qmlscreen.h"

#include <kscreen/output.h>
#include <KDebug>

#include <QStandardItem>
#include <QStandardItemModel>

const static int sMargin = 0;
const static int sSnapArea = 20;
const static int sSnapAlignArea = 10;

Q_DECLARE_METATYPE(KScreen::Mode*)

bool operator>(const QSize &sizeA, const QSize &sizeB)
{
    return ((sizeA.width() > sizeB.width()) && (sizeA.height() > sizeB.height()));
}

QMLOutput::QMLOutput(QDeclarativeItem *parent):
    QDeclarativeItem(parent),
    m_output(0),
    m_screen(0),
    m_cloneOf(0),
    m_leftDock(0),
    m_topDock(0),
    m_rightDock(0),
    m_bottomDock(0)
{
    connect(this, SIGNAL(xChanged()), SLOT(moved()));
    connect(this, SIGNAL(yChanged()), SLOT(moved()));
}

QMLOutput::~QMLOutput()
{

}

KScreen::Output* QMLOutput::output() const
{
    return m_output;
}

void QMLOutput::setOutput(KScreen::Output *output)
{
    Q_ASSERT(m_output == 0);

    m_output = output;
    Q_EMIT outputChanged();

    connect(m_output, SIGNAL(rotationChanged()),
            this, SLOT(updateRootProperties()));
    connect(m_output, SIGNAL(currentModeIdChanged()),
            this, SLOT(currentModeIdChanged()));
}

QMLScreen *QMLOutput::screen() const
{
    return m_screen;
}

void QMLOutput::setScreen(QMLScreen *screen)
{
    Q_ASSERT(m_screen == 0);

    m_screen = screen;
    Q_EMIT screenChanged();
}

void QMLOutput::setLeftDockedTo(QMLOutput *output)
{
    if (m_leftDock == output) {
        return;
    }

    m_leftDock = output;
    Q_EMIT leftDockedToChanged();

    update();
}

QMLOutput *QMLOutput::leftDockedTo() const
{
    return m_leftDock;
}

void QMLOutput::undockLeft()
{
    setLeftDockedTo(0);
}

void QMLOutput::setTopDockedTo(QMLOutput *output)
{
    if (m_topDock == output) {
        return;
    }

    m_topDock = output;
    Q_EMIT topDockedToChanged();

    update();
}

QMLOutput *QMLOutput::topDockedTo() const
{
    return m_topDock;
}

void QMLOutput::undockTop()
{
    setTopDockedTo(0);
}

void QMLOutput::setRightDockedTo(QMLOutput *output)
{
    if (m_rightDock == output) {
        return;
    }

    m_rightDock = output;
    Q_EMIT rightDockedToChanged();

    update();
}

QMLOutput *QMLOutput::rightDockedTo() const
{
    return m_rightDock;
}

void QMLOutput::undockRight()
{
    setRightDockedTo(0);
}

void QMLOutput::setBottomDockedTo(QMLOutput *output)
{
    if (m_bottomDock == output) {
        return;
    }

    m_bottomDock = output;
    Q_EMIT bottomDockedToChanged();

    update();
}

QMLOutput *QMLOutput::bottomDockedTo() const
{
    return m_bottomDock;
}

void QMLOutput::undockBottom()
{
    setBottomDockedTo(0);
}

void QMLOutput::setCloneOf(QMLOutput* other)
{
    if (m_cloneOf == other) {
        return;
    }

    m_cloneOf = other;
    Q_EMIT cloneOfChanged();
}

QMLOutput* QMLOutput::cloneOf() const
{
    return m_cloneOf;
}

int QMLOutput::currentOutputHeight() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::Mode *mode = m_output->currentMode();
    if (!mode) {
        if (m_output->isConnected()) {
            mode = bestMode();
            m_output->setCurrentModeId(mode->id());
        } else {
            return 1000;
        }
    }

    return mode->size().height();
}

int QMLOutput::currentOutputWidth() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::Mode *mode = m_output->currentMode();
    if (!mode) {
        if (m_output->isConnected()) {
            mode = bestMode();
            m_output->setCurrentModeId(mode->id());
        } else {
            return 1000;
        }
    }

    return mode->size().width();
}

void QMLOutput::currentModeIdChanged()
{
    if (!m_output) {
        return;
    }

    if (m_rightDock) {
        QMLOutput *rightDock = m_rightDock;
        float newWidth = currentOutputWidth() * m_screen->outputScale();
        setX(rightDock->x() - newWidth);
        setRightDockedTo(rightDock);
    }

    if (m_bottomDock) {
        QMLOutput *bottomDock = m_bottomDock;
        float newHeight = currentOutputHeight() * m_screen->outputScale();
        setY(bottomDock->y() - newHeight);
        setBottomDockedTo(bottomDock);
    }

    Q_EMIT currentOutputSizeChanged();
}


int QMLOutput::outputX() const
{
    return m_output->pos().x();
}

void QMLOutput::setOutputX(int x)
{
    if (m_output->pos().rx() == x) {
        return;
    }

    QPoint pos = m_output->pos();
    pos.setX(x);
    m_output->setPos(pos);
    Q_EMIT outputXChanged();
}

int QMLOutput::outputY() const
{
    return m_output->pos().y();
}

void QMLOutput::setOutputY(int y)
{
    if (m_output->pos().ry() == y) {
        return;
    }

    QPoint pos = m_output->pos();
    pos.setY(y);
    m_output->setPos(pos);
    Q_EMIT outputYChanged();
}

KScreen::Mode* QMLOutput::bestMode() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::ModeList modes = m_output->modes();
    KScreen::Mode *bestMode = 0;
    Q_FOREACH (KScreen::Mode *mode, modes) {
        if (!bestMode || (mode->size() > bestMode->size())) {
            bestMode = mode;
        }
    }

    return bestMode;
}

bool QMLOutput::collidesWithOutput(QObject *other)
{
    return QGraphicsItem::collidesWithItem(qobject_cast<QGraphicsItem*>(other), Qt::IntersectsItemShape);
}

bool QMLOutput::maybeSnapTo(QMLOutput *other)
{
    qreal centerX = x() + (width() / 2.0);
    qreal centerY = y() + (height() / 2.0);

    const qreal x2 = other->x();
    const qreal y2 = other->y();
    const qreal height2 = other->height();
    const qreal width2 = other->width();
    const qreal centerX2 = x2 + (width2 / 2.0);
    const qreal centerY2 = y2 + (height2 / 2.0);

    /* left of other */
    if ((x() + width() > x2 - sSnapArea) && (x() + width() < x2 + sSnapArea) &&
        (y() + height() > y2) && (y() < y2 + height2))
    {
        setX(x2 - width() + sMargin);
        centerX = x() + (width() / 2.0);
        setRightDockedTo(other);
        other->setLeftDockedTo(this);
        //output.cloneOf = null;

        /* output is snapped to other on left and their
         * upper sides are aligned */
        if ((y() < y2 + sSnapAlignArea) && (y() > y2 - sSnapAlignArea)) {
            setY(y2);
            return true;
        }

        /* output is snapped to other on left and they
         * are centered */
        if ((centerY < centerY2 + sSnapAlignArea) && (centerY > centerY2 - sSnapAlignArea)) {
            setY(centerY2 - (height() / 2.0));
            return true;
        }

        /* output is snapped to other on left and their
         * bottom sides are aligned */
        if ((y() + height() < y2 + height2 + sSnapAlignArea) && 
            (y() + height() > y2 + height2 - sSnapAlignArea))
        {
            setY(y2 + height2 - height());
            return true;
        }

        return true;
    }

    /* output is right of other */
    if ((x() > x2 + width2 - sSnapArea) && (x() < x2 + width2 + sSnapArea) &&
        (y() + height() > y2) && (y() < y2 + height2))
    {
        setX(x2 + width2 - sMargin);
        centerX = x() + (width() / 2.0);
        setLeftDockedTo(other);
        other->setRightDockedTo(this);
        //output.cloneOf = null;

        /* output is snapped to other on right and their
         * upper sides are aligned */
        if ((y() < y2 + sSnapAlignArea) && (y() > y2 - sSnapAlignArea)) {
            setY(y2);
            return true;
        }

        /* output is snapped to other on right and they
         * are centered */
        if ((centerY < centerY2 + sSnapAlignArea) && (centerY > centerY2 - sSnapAlignArea)) {
            setY(centerY2 - (height() / 2.0));
            return true;
        }

        /* output is snapped to other on right and their
         * bottom sides are aligned */
        if ((y() + height() < y2 + height2 + sSnapAlignArea) &&
            (y() + height() > y2 + height2 - sSnapAlignArea))
        {
            setY(y2 + height2 - height());
            return true;
        }

        return true;
    }

    /* output is above other */
    if ((y() + height() > y2 - sSnapArea) && (y() + height() < y2 + sSnapArea) &&
        (x() + width() > x2) && (x() < x2 + width2))
    {
        setY(y2 - height() + sMargin);
        centerY = y() + (height() / 2.0);
        setBottomDockedTo(other);
        other->setTopDockedTo(this);
        //output.cloneOf = null;

        /* output is snapped to other on top and their
         * left sides are aligned */
        if ((x() < x2 + sSnapAlignArea) && (x() > x2 - sSnapAlignArea)) {
            setX(x2);
            return true;
        }

        /* output is snapped to other on top and they
         * are centered */
        if ((centerX < centerX2 + sSnapAlignArea) && (centerX > centerX2 - sSnapAlignArea)) {
            setX(centerX2 - (width() / 2.0));
            return true;
        }

        /* output is snapped to other on top and their
         * right sides are aligned */
        if ((x() + width() < x2 + width2 + sSnapAlignArea) &&
            (x() + width() > x2 + width2 - sSnapAlignArea))
        {
            setX(x2 + width2 - width());
            return true;
        }

        return true;
    }

    /* output is below other */
    if ((y() > y2 + height2 - sSnapArea) && (y() < y2 + height2 + sSnapArea) &&
        (x() + width() > x2) && (x() < x2 + width2))
    {
        setY(y2 + height2 - sMargin);
        centerY = y() + (height() / 2.0);
        setTopDockedTo(other);
        other->setBottomDockedTo(this);
        //output.cloneOf = null;

        /* output is snapped to other on bottom and their
         * left sides are aligned */
        if ((x() < x2 + sSnapAlignArea) && (x() > x2 - sSnapAlignArea)) {
            setX(x2);
            return true;
        }

        /* output is snapped to other on bottom and they
         * are centered */
        if ((centerX < centerX2 + sSnapAlignArea) && (centerX > centerX2 - sSnapAlignArea)) {
            setX(centerX2 - (width() / 2.0));
            return true;
        }

        /* output is snapped to other on bottom and their
         * right sides are aligned */
        if ((x() + width() < x2 + width2 + sSnapAlignArea) &&
            (x() + width() > x2 + width2 - sSnapAlignArea))
        {
            setX(x2 + width2 - width());
            return true;
        }

        return true;
    }

    return false;
}

void QMLOutput::moved()
{
    const QList<QGraphicsItem*> siblings = screen()->childItems();

    disconnect(this, SIGNAL(xChanged()), this, SLOT(moved()));
    disconnect(this, SIGNAL(yChanged()), this, SLOT(moved()));
    Q_FOREACH (QGraphicsItem *sibling, siblings) {
        QMLOutput *otherOutput = qobject_cast<QMLOutput*>(sibling);
        if (!otherOutput || otherOutput == this) {
            continue;
        }

        if (!maybeSnapTo(otherOutput)) {
            if (m_leftDock == otherOutput) {
                m_leftDock->undockRight();
                undockLeft();
            }
            if (m_topDock == otherOutput) {
                m_topDock->undockBottom();
                undockTop();
            }
            if (m_rightDock == otherOutput) {
                m_rightDock->undockLeft();
                undockRight();
            }
            if (m_bottomDock == otherOutput) {
                m_bottomDock->undockTop();
                undockBottom();
            }
        }
    }
    connect(this, SIGNAL(xChanged()), SLOT(moved()));
    connect(this, SIGNAL(yChanged()), SLOT(moved()));

    Q_EMIT moved(m_output->name());
}


/* Transformation of an item (rotation of the MouseArea) is only visual.
 * The coordinates and dimensions are still the same (when you rotated
 * 100x500 rectangle by 90 deg, it will still be 100x500, although
 * visually it will be 500x100).
 *
 * This method calculates the real-visual coordinates and dimensions of
 * the MouseArea and updates root item to match them. This makes snapping
 * works correctly regardless on visual rotation of the output
 */
void QMLOutput::updateRootProperties()
{
    const int transformedWidth = (m_output->isHorizontal() ? currentOutputWidth() : currentOutputHeight()) * m_screen->outputScale();
    const int transformedHeight = (m_output->isHorizontal() ? currentOutputHeight() : currentOutputWidth()) * m_screen->outputScale();

    const int transformedX = x() + (width() / 2) - (transformedWidth / 2);
    const int transformedY = y() + (height() / 2) - (transformedHeight / 2);

    setPos(transformedX, transformedY);
    setSize(QSize(transformedWidth, transformedHeight));
}
