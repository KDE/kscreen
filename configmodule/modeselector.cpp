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
#include <KLocalizedString>

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
    qDebug() << "mm" << output->currentMode();
    m_initialMode = output->currentMode();
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
            if (!refreshRates.count()) {
                refreshRates.append(0);
            }
            if (!refreshRates.contains((qreal)(_mode->refreshRate()))) {
                refreshRates.append((qreal)(_mode->refreshRate()));
                qSort(refreshRates.begin(), refreshRates.end());
            }
        }
    }
    m_currentModeIndex = m_modeSizes.indexOf(modeString(m_output->currentMode().data()));
    m_modeLabelMin = modeString(m_modes.first());
    m_modeLabelMax = modeString(m_modes.last());
    m_refreshLabelMin = QString::number(m_refreshRatesTable[currentmsize].at(1), 'f', 2);
    m_refreshLabelMax = QString::number(m_refreshRatesTable[currentmsize].last(), 'f', 2);

    emit modesChanged();
    emit refreshRatesChanged();
}

int ModeSelector::currentModeIndex() const
{
    return m_currentModeIndex;
}

void ModeSelector::setSelectedResolutionIndex(int index)
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
        m_refreshLabelMin = QString::number(m_refreshRatesTable[msize].at(1), 'f', 2);
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

QStringList ModeSelector::refreshRatesLabels() const
{
    QStringList rs({ i18n("Auto") });
    for (const auto rr : m_refreshRatesTable[m_selectedModeSize]) {
        if (rr > 0) { // we have already appended the Auto label
            rs << i18nc("refresh rate combo box values", "%1 Hz", QString::number(rr, 'f', 2));
        }
    }
    return rs;
}

void ModeSelector::setSelectedRefreshRate(int index)
{
    int ix = index;
    if (!m_refreshRatesTable[m_selectedModeSize].count()) {
        qCDebug(KSCREEN_KCM) << "Don't know refresh rates." << index << m_selectedModeSize << m_refreshRatesTable[m_selectedModeSize];
        return;
    }
    if (index == 0) {
        ix = m_refreshRatesTable[m_selectedModeSize].count() - 1;
    }
    if (ix < 0 || ix >= m_refreshRatesTable[m_selectedModeSize].count()) {
        qCWarning(KSCREEN_KCM) << "Invalid refresh rate:" << index << m_refreshRatesTable[m_selectedModeSize];
        return;
    }
    if (m_selectedRefreshRate != m_refreshRatesTable[m_selectedModeSize].at(ix)) {
        m_selectedRefreshRate = m_refreshRatesTable[m_selectedModeSize].at(ix);
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

int ModeSelector::preferredRefreshIndexForSizeIndex(int index)
{
    auto ms = m_modeSizes.at(index);
    auto mstring = modeString(m_initialMode.data());
    if (ms == mstring) {
        int ix = m_refreshRatesTable[mstring].indexOf(m_initialMode->refreshRate());
        if (ix >= m_refreshRatesTable[mstring].count()-1) {
            ix = 0; // auto
        }
        return ix;

    }
    return 0;
}

QStringList ModeSelector::newOutputActions() const
{
    QStringList actions;

    actions.append(i18nc("new output combo box", "Do nothing"));
    actions.append(i18nc("new output combo box", "Open this configuration module"));
    actions.append(i18nc("new output combo box", "Extend workspace"));
    actions.append(i18nc("new output combo box", "Clone outputs"));

    return actions;
}

void ModeSelector::setNewOutputAction(int index)
{
    qCDebug(KSCREEN_KCM) << "Setting new output action" << index << newOutputActions().at(index);
}

} // ns
