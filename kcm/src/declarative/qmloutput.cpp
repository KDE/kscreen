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

#include <QStandardItem>
#include <QStandardItemModel>
#include <qquickitem.h>
#include <QRect>

const static int sMargin = 0;
const static int sSnapArea = 20;
const static int sSnapAlignArea = 6;

Q_DECLARE_METATYPE(KScreen::ModePtr)

bool operator>(const QSize &sizeA, const QSize &sizeB)
{
    return ((sizeA.width() > sizeB.width()) && (sizeA.height() > sizeB.height()));
}

QMLOutput::QMLOutput(QQuickItem *parent):
    QQuickItem(parent),
    m_screen(nullptr),
    m_cloneOf(nullptr),
    m_leftDock(nullptr),
    m_topDock(nullptr),
    m_rightDock(nullptr),
    m_bottomDock(nullptr),
    m_isCloneMode(false)
{
    connect(this, &QMLOutput::xChanged,
            this, static_cast<void(QMLOutput::*)()>(&QMLOutput::moved));
    connect(this, &QMLOutput::yChanged,
            this, static_cast<void(QMLOutput::*)()>(&QMLOutput::moved));
}

KScreen::Output* QMLOutput::output() const
{
    return m_output.data();
}

KScreen::OutputPtr QMLOutput::outputPtr() const
{
    return m_output;
}

void QMLOutput::setOutputPtr(const KScreen::OutputPtr &output)
{
    Q_ASSERT(m_output.isNull());

    m_output = output;
    Q_EMIT outputChanged();

    connect(m_output.data(), &KScreen::Output::rotationChanged,
            this, &QMLOutput::updateRootProperties);
    connect(m_output.data(), &KScreen::Output::currentModeIdChanged,
            this, &QMLOutput::currentModeIdChanged);
    connect(m_output.data(), &KScreen::Output::scaleChanged,
            this, &QMLOutput::currentModeIdChanged);
}

QMLScreen *QMLOutput::screen() const
{
    return m_screen;
}

void QMLOutput::setScreen(QMLScreen *screen)
{
    Q_ASSERT(m_screen == nullptr);

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
}

QMLOutput *QMLOutput::leftDockedTo() const
{
    return m_leftDock;
}

void QMLOutput::undockLeft()
{
    setLeftDockedTo(nullptr);
}

void QMLOutput::setTopDockedTo(QMLOutput *output)
{
    if (m_topDock == output) {
        return;
    }

    m_topDock = output;
    Q_EMIT topDockedToChanged();
}

QMLOutput *QMLOutput::topDockedTo() const
{
    return m_topDock;
}

void QMLOutput::undockTop()
{
    setTopDockedTo(nullptr);
}

void QMLOutput::setRightDockedTo(QMLOutput *output)
{
    if (m_rightDock == output) {
        return;
    }

    m_rightDock = output;
    Q_EMIT rightDockedToChanged();
}

QMLOutput *QMLOutput::rightDockedTo() const
{
    return m_rightDock;
}

void QMLOutput::undockRight()
{
    setRightDockedTo(nullptr);
}

void QMLOutput::setBottomDockedTo(QMLOutput *output)
{
    if (m_bottomDock == output) {
        return;
    }

    m_bottomDock = output;
    Q_EMIT bottomDockedToChanged();
}

QMLOutput *QMLOutput::bottomDockedTo() const
{
    return m_bottomDock;
}

void QMLOutput::undockBottom()
{
    setBottomDockedTo(nullptr);
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

    KScreen::ModePtr mode = m_output->currentMode();
    if (!mode) {
        if (m_output->isConnected()) {
            mode = bestMode();
            if (!mode) {
                return 1000;
            }
            m_output->setCurrentModeId(mode->id());
        } else {
            return 1000;
        }
    }

    return mode->size().height() / m_output->scale();
}

int QMLOutput::currentOutputWidth() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::ModePtr mode = m_output->currentMode();
    if (!mode) {
        if (m_output->isConnected()) {
            mode = bestMode();
            if (!mode) {
                return 1000;
            }
            m_output->setCurrentModeId(mode->id());
        } else {
            return 1000;
        }
    }

    return mode->size().width() / m_output->scale();
}

void QMLOutput::currentModeIdChanged()
{
    if (!m_output) {
        return;
    }

    if (isCloneMode()) {
        const float newWidth = currentOutputWidth() * m_screen->outputScale();
        setX((m_screen->width() - newWidth) / 2);
        const float newHeight = currentOutputHeight() * m_screen->outputScale();
        setY((m_screen->height() - newHeight) / 2);
    } else {
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

bool QMLOutput::isCloneMode() const
{
    return m_isCloneMode;
}

void QMLOutput::setIsCloneMode(bool isCloneMode)
{
    if (m_isCloneMode == isCloneMode) {
        return;
    }

    m_isCloneMode = isCloneMode;
    Q_EMIT isCloneModeChanged();
}

void QMLOutput::dockToNeighbours()
{
    Q_FOREACH (QMLOutput *otherQmlOutput, m_screen->outputs()) {
        if (otherQmlOutput == this) {
            continue;
        }

        if (!otherQmlOutput->output()->isConnected() || !otherQmlOutput->output()->isEnabled()) {
            continue;
        }

        const QRect geom = m_output->geometry();
        const QRect otherGeom = otherQmlOutput->output()->geometry();

        if (geom.left() - 1 == otherGeom.right()) {
            setLeftDockedTo(otherQmlOutput);
            continue;
        }
        if (geom.right() + 1 == otherGeom.left()) {
            setRightDockedTo(otherQmlOutput);
            continue;
        }
        if (geom.top() - 1 == otherGeom.bottom()) {
            setTopDockedTo(otherQmlOutput);
            continue;
        }
        if (geom.bottom() + 1 == otherGeom.top()) {
            setBottomDockedTo(otherQmlOutput);
            continue;
        }
    }
}

KScreen::ModePtr QMLOutput::bestMode() const
{
    if (!m_output) {
        return KScreen::ModePtr();
    }

    KScreen::ModeList modes = m_output->modes();
    KScreen::ModePtr bestMode;
    Q_FOREACH (const KScreen::ModePtr &mode, modes) {
        if (!bestMode || (mode->size() > bestMode->size())) {
            bestMode = mode;
        }
    }

    return bestMode;
}

bool QMLOutput::collidesWithOutput(QObject *other)
{
    QQuickItem* otherItem = qobject_cast<QQuickItem*>(other);
    return boundingRect().intersects(otherItem->boundingRect());
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
    const QList<QQuickItem*> siblings = screen()->childItems();

    // First, if we have moved, then unset the "cloneOf" flag
    setCloneOf(nullptr);

    disconnect(this, &QMLOutput::xChanged, this, static_cast<void(QMLOutput::*)()>(&QMLOutput::moved));
    disconnect(this, &QMLOutput::yChanged, this, static_cast<void(QMLOutput::*)()>(&QMLOutput::moved));
    Q_FOREACH (QQuickItem *sibling, siblings) {
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
    connect(this, &QMLOutput::xChanged, this, static_cast<void(QMLOutput::*)()>(&QMLOutput::moved));
    connect(this, &QMLOutput::yChanged, this, static_cast<void(QMLOutput::*)()>(&QMLOutput::moved));

    Q_EMIT moved(m_output->name());
}


/* Transformation of an item (rotation of the MouseArea) is only visual.
 * The coordinates and dimensions are still the same (when you rotated
 * 100x500 rectangle by 90 deg, it will still be 100x500, although
 * visually it will be 500x100).
 *
 * This method calculates the real-visual coordinates and dimensions of
 * the MouseArea and updates root item to match them. This makes snapping
 * work correctly regardless off visual rotation of the output
 */
void QMLOutput::updateRootProperties()
{
    const float transformedWidth = (m_output->isHorizontal() ? currentOutputWidth() : currentOutputHeight()) * m_screen->outputScale();
    const float transformedHeight = (m_output->isHorizontal() ? currentOutputHeight() : currentOutputWidth()) * m_screen->outputScale();

    const float transformedX = x() + (width() / 2.0) - (transformedWidth / 2.0);
    const float transformedY = y() + (height() / 2.0) - (transformedHeight / 2.0);

    setPosition(QPointF(transformedX, transformedY));
    setSize(QSizeF(transformedWidth, transformedHeight));
}
