/*
    Copyright 2016 Sebastian Kügler <sebas@kde.org>

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


#include "modeselector.h"
#include "debug_p.h"
#include "qmloutput.h"

#include <kscreen/output.h>

Q_DECLARE_METATYPE(KScreen::ModePtr)

namespace KScreen {

ModeSelector::ModeSelector(QObject* parent):
    QObject(parent)
    , m_output(nullptr)
{
    qRegisterMetaType<KScreen::ModeList>("KScreenModeList");
}

ModeSelector::~ModeSelector()
{
}

void ModeSelector::setOutputPtr(const KScreen::OutputPtr &output)
{
    m_output = output;
    updateModes();
    emit outputChanged();
}

KScreen::Output* ModeSelector::output() const
{
    return m_output.data();
}

QQmlListProperty<KScreen::Mode> ModeSelector::modes()
{
    QQmlListProperty<KScreen::Mode> lst(this, m_modes);
    return lst;
}

QString ModeSelector::modeLabelMin() const
{
    return m_modeLabelMin;
}

QString ModeSelector::modeLabelMax() const
{
    return m_modeLabelMax;
}

QString ModeSelector::refreshLabelMin() const
{
    return m_refreshLabelMin;
}

QString ModeSelector::refreshLabelMax() const
{
    return m_refreshLabelMax;
}

QStringList ModeSelector::modeSizes()
{
    return m_modeSizes;
}

bool modeLessThan(const KScreen::Mode* left, const KScreen::Mode* right)
{
    const auto sl = left->size().width() * left->size().height();
    const auto sr = right->size().width() * right->size().height();
    if (sl == sr) {
        return left->refreshRate() < right->refreshRate();
    }
    return sl < sr;
}

QString modeString(const KScreen::Mode *mode) {
    return QString("%1x%2").arg(QString::number(mode->size().width()), QString::number(mode->size().height()));
}

void ModeSelector::updateModes()
{
    QString currentmsize;
    m_modes.clear();
    m_modeSizes.clear();
    //m_selectedModeSize.clear();
    //m_selectedRefreshRate = 0;
    if (m_output) {
        for (auto _md : m_output->modes()) {
            m_modes << _md.data();
        }

        qSort(m_modes.begin(), m_modes.end(), modeLessThan);
        for (auto _mode : m_modes) {
            const auto msize = modeString(_mode);
            if (!m_modeSizes.contains(msize)) {
                m_modeSizes << msize;
            }
            if (m_output->currentModeId() == _mode->id()) {
                currentmsize = msize;
            }
            QList<qreal> &refreshRates = m_refreshRatesTable[msize];
            if (!refreshRates.contains((qreal)(_mode->refreshRate()))) {
                refreshRates.append((qreal)(_mode->refreshRate()));
                qSort(refreshRates.begin(), refreshRates.end());
            }
        }
    }
    m_currentModeIndex = m_modeSizes.indexOf(modeString(m_output->currentMode().data()));
    m_modeLabelMin = modeString(m_modes.first());
    m_modeLabelMax = modeString(m_modes.last());
    m_refreshLabelMin = QString::number(m_refreshRatesTable[currentmsize].first(), 'f', 2);
    m_refreshLabelMax = QString::number(m_refreshRatesTable[currentmsize].last(), 'f', 2);

    emit modesChanged();
    emit refreshRatesChanged();
}

int ModeSelector::currentModeIndex() const
{
    return m_currentModeIndex;
}

void ModeSelector::setSelectedSize(int index)
{
    if (index < 0 || !m_modeSizes.count() || index >= m_modeSizes.count()) {
        qCWarning(KSCREEN_KCM) << "Invalid size index:" << index << m_modeSizes;
        return;
    }
    const auto msize = m_modeSizes.at(index);
    if (m_selectedModeSize != msize) {
        m_selectedModeSize = msize;
        m_selectedRefreshRate = -1; // magic value, means "auto"
        updateSelectedMode();
        m_refreshLabelMin = QString::number(m_refreshRatesTable[msize].first(), 'f', 2);
        m_refreshLabelMax = QString::number(m_refreshRatesTable[msize].last(), 'f', 2);
        emit refreshRatesChanged();
    }
}

KScreen::Mode* ModeSelector::selectedMode() const
{
    if (m_output) {
        return m_output->mode(m_selectedModeId).data();
    } else {
        return nullptr;
    }
}

QList<qreal> ModeSelector::refreshRates()
{
    return m_refreshRatesTable[m_selectedModeSize];
}

void ModeSelector::setSelectedRefreshRate(int index)
{
    if (index == -1 || index >= m_refreshRatesTable[m_selectedModeSize].count()) {
        qCWarning(KSCREEN_KCM) << "Invalid refresh rate:" << index << m_refreshRatesTable[m_selectedModeSize];
        return;
    }
    if (m_selectedRefreshRate != m_refreshRatesTable[m_selectedModeSize].at(index)) {
        m_selectedRefreshRate = m_refreshRatesTable[m_selectedModeSize].at(index);
        updateSelectedMode();
    }
}

void ModeSelector::updateSelectedMode()
{
    QString selected;
    for (auto mode : m_modes) {
        if (modeString(mode) == m_selectedModeSize) {
            if (m_selectedRefreshRate <= 0) {
                selected = mode->id();
            } else if (m_selectedRefreshRate == mode->refreshRate()) {
                selected = mode->id();
            }
        }
    }
    if (selected == m_selectedModeId) {
        return;
    }
    m_selectedModeId = selected;
    qCDebug(KSCREEN_KCM) << "Selected Mode is now:" << m_selectedModeId << m_output->mode(m_selectedModeId);
    emit selectedModeChanged();
    m_output->setCurrentModeId(m_selectedModeId);
}

} // ns
