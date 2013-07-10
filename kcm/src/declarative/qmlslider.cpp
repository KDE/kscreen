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

#include "qmlslider.h"

#include <QSlider>

#include <kscreen/output.h>
#include <kscreen/mode.h>

#include <KDebug>

QMLSlider::QMLSlider(QGraphicsItem *parent):
    QGraphicsProxyWidget(parent),
    m_output(0)
{
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setAttribute(Qt::WA_NoSystemBackground);
    m_slider->setTickPosition(QSlider::TicksAbove);
    m_slider->setTickInterval(1);

    m_slider->setMinimum(0);
    m_slider->setMaximum(0);
    m_slider->setSingleStep(1);

    connect(m_slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderMoved()));

    setWidget(m_slider);
}

QMLSlider::~QMLSlider()
{

}

KScreen::Output *QMLSlider::output() const
{
    return m_output;
}

bool cmpModeByArea(KScreen::Mode *modeA, KScreen::Mode *modeB)
{
    const QSize sizeA = modeA->size();
    const int areaA = sizeA.width() * sizeA.height();
    const QSize sizeB = modeB->size();
    const int areaB = sizeB.width() * sizeB.height();

    if (areaA == areaB) {
        if (sizeA == sizeB) {
            return modeA->refreshRate() < modeB->refreshRate();
        }

        if (sizeA.width() == sizeB.width()) {
            return sizeA.height() < sizeB.height();
        }

        return sizeA.width() < sizeB.width();
    }

    return areaA < areaB;
}

void QMLSlider::setOutput(KScreen::Output *output)
{
    Q_ASSERT(m_output == 0);

    m_output = output;
    Q_EMIT outputChanged();

    if (output) {
        m_modes = m_output->modes().values();
        qSort(m_modes.begin(), m_modes.end(), cmpModeByArea);

        m_slider->blockSignals(true);
        m_slider->setMaximum(m_modes.count() - 1);
        m_slider->setValue(m_modes.indexOf(output->currentMode()));
        m_slider->blockSignals(false);

        Q_EMIT modesChanged();
    }
}

QDeclarativeListProperty<KScreen::Mode> QMLSlider::modes()
{
    return QDeclarativeListProperty<KScreen::Mode>(this, m_modes);
}

void QMLSlider::slotSliderMoved()
{
    Q_ASSERT(m_slider->value() < m_modes.count() );

    KScreen::Mode *mode = m_modes.at(m_slider->value());

    m_output->setCurrentModeId(mode->id());
}



#include "qmlslider.moc"
