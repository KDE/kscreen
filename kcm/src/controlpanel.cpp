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


#include "controlpanel.h"
#include "qmloutput.h"
#include "modesproxymodel.h"
#include "resolutionsortmodel.h"

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>

#include <KComboBox>
#include <KLocalizedString>
#include <kscreen/output.h>

#include <KDebug>

ControlPanel::ControlPanel(QWidget* parent, Qt::WindowFlags f):
    QWidget(parent, f),
    m_output(0)
{
    m_layout = new QGridLayout(this);

    m_outputName = new QLabel(this);
    QFont font = m_outputName->font();
    font.setPointSize(15);
    m_outputName->setFont(font);
    m_layout->addWidget(m_outputName, 0, 0, 1, 1);
    m_layout->setColumnStretch(3, 2);

    m_primary = new QCheckBox(i18n("Primary display"), this);
    m_layout->addWidget(m_primary, 1, 0, 1, 1);
    connect(m_primary, SIGNAL(toggled(bool)), SLOT(primaryChanged(bool)));

    m_enabled = new QCheckBox(i18n("Enabled"), this);
    m_layout->addWidget(m_enabled, 2, 0, 1, 1);
    connect(m_enabled, SIGNAL(toggled(bool)), SLOT(enabledChanged(bool)));

    m_resolutionsLabel = new QLabel(i18n("Resolution:"), this);
    m_layout->addWidget(m_resolutionsLabel, 3, 0, 1, 1);

    m_resolutionSortModel = new ResolutionSortModel(this);

    m_resolutions = new KComboBox(this);
    m_resolutions->setModel(m_resolutionSortModel);
    m_resolutions->setMinimumContentsLength(10);
    m_layout->addWidget(m_resolutions, 3, 1, 1, 1);
    connect(m_resolutions, SIGNAL(currentIndexChanged(int)), SLOT(resolutionChanged()));

    m_refreshRatesLabel = new QLabel(i18n("Refresh rate:"), this);
    m_layout->addWidget(m_refreshRatesLabel, 4, 0, 1, 1);

    m_modesProxyModel = new ModesProxyModel(this);

    m_refreshRates = new KComboBox(this);
    m_refreshRates->setModel(m_modesProxyModel);
    m_layout->addWidget(m_refreshRates, 4, 1, 1, 1);
    connect(m_refreshRates, SIGNAL(currentIndexChanged(int)), SLOT(refreshRateChanged()));
    connect(m_resolutions, SIGNAL(currentIndexChanged(int)), m_refreshRates, SLOT(repaint()));
}

ControlPanel::~ControlPanel()
{

}

void ControlPanel::setOutput(QMLOutput* output)
{
    m_output = 0;

    m_outputName->setText(output->output()->name());
    m_enabled->setChecked(output->output()->isEnabled());
    m_primary->setChecked(output->output()->isPrimary());
    m_resolutionSortModel->setSourceModel(output->modesModel());
    m_modesProxyModel->setSourceModel(m_resolutionSortModel);

    m_output = output;

    KScreen::Mode *currentMode = m_output->output()->mode(m_output->output()->currentMode());
    if (!currentMode) {
        return;
    }
    int index = m_resolutions->findData(currentMode->size(), QMLOutput::SizeRole);
    m_resolutions->setCurrentIndex(index);
    m_resolutionSortModel->sort(0, Qt::DescendingOrder);

    index = m_refreshRates->findData(currentMode->refreshRate(), QMLOutput::RefreshRateRole);
    m_refreshRates->setCurrentIndex(index);

    refreshRateChanged();
}

void ControlPanel::enabledChanged(bool enabled)
{
    if (!m_output) {
        return;
    }

    m_output->output()->setEnabled(enabled);
}

void ControlPanel::primaryChanged(bool primary)
{
    if (!m_output) {
        return;
    }

    m_output->output()->setPrimary(primary);
}

void ControlPanel::refreshRateChanged()
{
    if (!m_output) {
        return;
    }

    QModelIndex proxyModelIndex = m_resolutionSortModel->index(m_resolutions->currentIndex(), 0);
    QModelIndex parentIndex = m_resolutionSortModel->mapToSource(proxyModelIndex);
    QModelIndex modelIndex = m_refreshRates->model()->index(m_refreshRates->currentIndex(), 0, parentIndex);

    int modeId = m_refreshRates->model()->data(modelIndex, QMLOutput::ModeIdRole).toInt();
    if (modeId == -1) {
        QModelIndex proxyModelIndex = m_resolutionSortModel->index(m_resolutions->currentIndex(), 0);
        QModelIndex parentIndex = m_resolutionSortModel->mapToSource(proxyModelIndex);
        modeId = m_output->modesModel()->index(0, 0, parentIndex).data(QMLOutput::ModeIdRole).toInt();
    }

    /* Shoudl not happen - invalid value */
    if (modeId == 0) {
        return;
    }

    m_output->output()->setCurrentMode(modeId);
}

void ControlPanel::resolutionChanged()
{
    m_modesProxyModel->setSourceModelCurrentRow(m_resolutions->currentIndex());

    refreshRateChanged();
}
