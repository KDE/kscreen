/*
 * Copyright 2015  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "primaryoutputcombo.h"
#include "utils.h"

#include <KLocalizedString>

#include <KScreen/Config>
#include <KScreen/Output>


PrimaryOutputCombo::PrimaryOutputCombo(QWidget* parent)
    : QComboBox(parent)
{
    connect(this, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PrimaryOutputCombo::onCurrentIndexChanged);

    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    addItem(i18n("No Primary Output"));
}

PrimaryOutputCombo::~PrimaryOutputCombo()
{
}

void PrimaryOutputCombo::setConfig(const KScreen::ConfigPtr &config)
{
    if (mConfig) {
        mConfig->disconnect(this);
        for (const KScreen::OutputPtr &output : mConfig->outputs()) {
            output->disconnect(this);
        }
    }

    clear();
    addItem(i18n("No Primary Output"));

    if (!config) {
        return;
    }

    mConfig = config;
    connect(mConfig.data(), &KScreen::Config::outputAdded,
            this, &PrimaryOutputCombo::addOutput);
    connect(mConfig.data(), &KScreen::Config::outputRemoved,
            this, &PrimaryOutputCombo::removeOutput);
    connect(mConfig.data(), &KScreen::Config::primaryOutputChanged,
            this, &PrimaryOutputCombo::setPrimaryOutput);

    for (const KScreen::OutputPtr &output : config->outputs()) {
        addOutput(output);
    }
}

void PrimaryOutputCombo::addOutput(const KScreen::OutputPtr &output)
{
    connect(output.data(), &KScreen::Output::isConnectedChanged,
            [output, this]() {
                outputChanged(output);
            });
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            [output, this]() {
                outputChanged(output);
            });

    if (!output->isConnected() || !output->isEnabled()) {
        return;
    }

    addOutputItem(output);
}

void PrimaryOutputCombo::removeOutput(int outputId)
{
    KScreen::OutputPtr output = mConfig->output(outputId);
    if (output) {
        output->disconnect(this);
    }

    removeOutputItem(outputId);
}

KScreen::OutputPtr PrimaryOutputCombo::primaryOutput() const
{
    if (!mConfig) {
        return KScreen::OutputPtr();
    }

    const int index = currentIndex();
    // "No Primary Output" item
    if (index == 0) {
        return KScreen::OutputPtr();
    }

    return mConfig->output(itemData(index).toInt());
}

void PrimaryOutputCombo::setPrimaryOutput(const KScreen::OutputPtr &output)
{
    Q_ASSERT(mConfig);

    const int index = output ? findData(output->id()) : 0;
    if (index == -1) {
        return;
    }

    if (index == currentIndex()) {
        return;
    }

    setCurrentIndex(index);
}


void PrimaryOutputCombo::outputChanged(const KScreen::OutputPtr &output)
{
    const int index = findData(output->id());
    if (index == -1 && output->isConnected() && output->isEnabled()) {
        addOutputItem(output);
    } else if (index > 0 && (!output->isConnected() || !output->isEnabled())) {
        removeOutputItem(output->id());
    }
}

void PrimaryOutputCombo::addOutputItem(const KScreen::OutputPtr& output)
{
    addItem(Utils::outputName(output), output->id());
    if (output->isPrimary()) {
        setPrimaryOutput(output);
    }
}

void PrimaryOutputCombo::removeOutputItem(int outputId)
{
    const int index = findData(outputId);
    if (index == -1) {
        return;
    }

    if (index == currentIndex()) {
        // We'll get the actual primary update signal eventually
        // Don't emit currentIndexChanged
        const bool blocked = blockSignals(true);
        setCurrentIndex(0);
        blockSignals(blocked);
    }
    removeItem(index);
}

void PrimaryOutputCombo::onCurrentIndexChanged(int currentIndex)
{
    if (!mConfig) {
        return;
    }

    if (currentIndex == 0) {
        mConfig->setPrimaryOutput(KScreen::OutputPtr());
    } else if (currentIndex > 0) {
        mConfig->setPrimaryOutput(mConfig->output(itemData(currentIndex).toInt()));
    }
}

