/*
    <one line to give the library's name and an idea of what it does.>
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

#include<kscreen/output.h>
#include <KDebug>

QMLOutput::QMLOutput():
    QDeclarativeItem(),
    m_output(0),
    m_cloneOf(0)
{
}

QMLOutput::~QMLOutput()
{

}

bool modeSizeLessThan(const /*KScreen::*/Mode* mode1, const /*KScreen::*/Mode* mode2)
{
    if (mode1->size().width() < mode2->size().width()) {
        return true;
    }

    if (mode1->size().width() == mode2->size().width()) {
        return mode1->size().height() < mode2->size().height();
    }

    return false;
}

void QMLOutput::setOutput(/*KScreen::*/Output* output)
{
    m_output = output;
    m_modes = m_output->modes().values();

    qSort(m_modes.begin(), m_modes.end(), modeSizeLessThan);

    connect(output, SIGNAL(clonesChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(currentModeChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(isEnabledChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(isPrimaryChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(outputChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(posChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(rotationChanged()), SIGNAL(changed()));

    Q_EMIT outputChanged();
}

/*KScreen::*/Output* QMLOutput::output() const
{
    return m_output;
}

void QMLOutput::setCloneOf(QMLOutput* other)
{
    m_cloneOf = other;

    Q_EMIT cloneOfChanged();
}

QMLOutput* QMLOutput::cloneOf() const
{
    return m_cloneOf;
}

QDeclarativeListProperty </*KScreen::*/Mode > QMLOutput::modes()
{
    return QDeclarativeListProperty </*KScreen::*/Mode > (this, m_modes);
}

QList<QVariant> QMLOutput::getRefreshRatesForResolution(const QString& res)
{
    QList<float> rates;

    Q_FOREACH(/*KScreen::*/Mode * mode, m_modes) {
        if (mode->name() == res) {
            rates << mode->refreshRate();
        }
    }
    qSort(rates.begin(), rates.end(), qGreater<float>());

    QList<QVariant> result;
    Q_FOREACH(float rate, rates) {
        result << rate;
    }
    return result;
}


QStringList QMLOutput::getResolutions() const
{
    QStringList resolutions;

    Q_FOREACH (/*KScreen::*/Mode * mode, m_modes) {
        /* The list is sorted ascendingly */
        resolutions.prepend(mode->name());
    }

    resolutions.removeDuplicates();

    return resolutions;
}

void QMLOutput::setMode(const QString& resolution, const float& refreshRate)
{
    float rr = refreshRate;

    if (refreshRate == 0.0f) {
        /* Don't use getRefreshRatesForResolution(), we need it unsorted */
        Q_FOREACH(/*KScreen::*/Mode * mode, m_output->modes()) {
            if (mode->name() == resolution) {
                rr = mode->refreshRate();
                break;
            }
        }
    }

    ModeList modes = m_output->modes();
    QHashIterator<int, Mode*> iter(m_output->modes());
    while (iter.hasNext()) {
        iter.next();

        if (iter.value()->name() != resolution) {
            continue;
        }

        if (iter.value()->refreshRate() == rr) {
            m_output->setCurrentMode(iter.key());
            return;
        }
    }
}
