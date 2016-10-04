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
}

QQmlListProperty<KScreen::Mode> ModeSelector::modes()
{
    QQmlListProperty<KScreen::Mode> lst(this, m_modes);
    return lst;
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
    m_modes.clear();
    m_modeSizes.clear();
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
            QList<qreal> &refreshRates = m_modesTable[msize];
            refreshRates.append((qreal)(_mode->refreshRate()));
            qSort(refreshRates.begin(), refreshRates.end());
        }
    }
    qCDebug(KSCREEN_KCM) << "Bazinga!" << m_modes.count();
    emit modesChanged();
    emit refreshRatesChanged();
}

void ModeSelector::setSelectedSize(int index)
{
    if (index >= m_modeSizes.count()) {
        qCWarning(KSCREEN_KCM) << "Invalid size index:" << index << m_modeSizes;
        return;
    }
    const auto msize = m_modeSizes.at(index);
    //qCDebug(KSCREEN_KCM) << "MODES:" << m_modesTable.keys();

    if (m_selectedModeSize != msize) {
        m_selectedModeSize = msize;
    //     qCDebug(KSCREEN_KCM) << "refreshrates:" << m_modesTable[m_selectedModeSize];
        m_selectedRefreshRate = 0;
        updateSelectedMode();
        qCDebug(KSCREEN_KCM) << "EMIT RC!! selected size is now:" << index << msize << selectedMode()->size() << m_modesTable[m_selectedModeSize];
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
    return m_modesTable[m_selectedModeSize];
}

void ModeSelector::setSelectedRefreshRate(int index)
{

    qCDebug(KSCREEN_KCM) << " select refresh " << index << m_selectedModeSize << m_modesTable[m_selectedModeSize];
    if (index >= m_modesTable[m_selectedModeSize].count()) {
        qCWarning(KSCREEN_KCM) << "Invalid refresh rate:" << index << m_modesTable[m_selectedModeSize];
        return;
    }
    qCDebug(KSCREEN_KCM) << "selected refresh rate is now:" << m_modesTable[m_selectedModeSize].at(index);
    if (m_selectedRefreshRate != m_modesTable[m_selectedModeSize].at(index)) {
        m_selectedRefreshRate = m_modesTable[m_selectedModeSize].at(index);
        updateSelectedMode();
    }
}

void ModeSelector::updateSelectedMode()
{
    for (auto mode : m_modes) {
        if (modeString(mode) == m_selectedModeSize) {
            if (m_selectedRefreshRate == 0) {
                m_selectedModeId = mode->id();
            } else if (m_selectedRefreshRate == mode->refreshRate()) {
                m_selectedModeId = mode->id();
            }
        }
    }
    qCDebug(KSCREEN_KCM) << "Selected is now:" << m_selectedModeId << m_output->mode(m_selectedModeId);
    emit selectedModeChanged();
    m_output->setCurrentModeId(m_selectedModeId);

}

} // ns