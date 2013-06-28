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

QMLSlider::QMLSlider(QGraphicsItem *parent):
    QGraphicsProxyWidget(parent),
    m_output(0)
{
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setAttribute(Qt::WA_NoSystemBackground);
    m_slider->setTickPosition(QSlider::TicksAbove);

    setWidget(m_slider);
}

QMLSlider::~QMLSlider()
{

}

KScreen::Output *QMLSlider::output() const
{
    return m_output;
}

void QMLSlider::setOutput(KScreen::Output *output)
{
    Q_ASSERT(m_output == 0);

    m_output = output;
    Q_EMIT outputChanged();
}


#include "qmlslider.moc"
