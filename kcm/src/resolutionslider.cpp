/*
 * Copyright 2013  Daniel Vrátil <dvratil@redhat.com>
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

#include "resolutionslider.h"
#include "utils.h"

#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QComboBox>

#include <KLocalizedString>

#include <kscreen/output.h>

static bool sizeLessThan(const QSize &sizeA, const QSize &sizeB)
{
    return sizeA.width() * sizeA.height() < sizeB.width() * sizeB.height();
}

ResolutionSlider::ResolutionSlider(const KScreen::OutputPtr &output, QWidget *parent)
    : QWidget(parent)
    , mOutput(output)
    , mSmallestLabel(nullptr)
    , mBiggestLabel(nullptr)
    , mCurrentLabel(nullptr)
    , mSlider(nullptr)
    , mComboBox(nullptr)
{
    connect(output.data(), &KScreen::Output::currentModeIdChanged,
            this, &ResolutionSlider::slotOutputModeChanged);

    Q_FOREACH (const KScreen::ModePtr &mode, output->modes()) {
        if (mModes.contains(mode->size())) {
            continue;
        }

        mModes << mode->size();
    }
    qSort(mModes.begin(), mModes.end(), sizeLessThan);

    QGridLayout *layout = new QGridLayout(this);

    if (mModes.count() > 15) {
        std::reverse(mModes.begin(), mModes.end());
        mComboBox = new QComboBox(this);
        mComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        mComboBox->setEditable(false);
        Q_FOREACH (const QSize &size, mModes) {
            mComboBox->addItem(Utils::sizeToString(size));
            if ((output->currentMode() && (output->currentMode()->size() == size))
                || (output->preferredMode() && (output->preferredMode()->size() == size))) {
                    mComboBox->setCurrentIndex(mComboBox->count() - 1);
            }
        }
        layout->addWidget(mComboBox, 0, 0, 1, 1);
        connect(mComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &ResolutionSlider::slotValueChanged);
    } else {
        mCurrentLabel = new QLabel(this);
        mCurrentLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(mCurrentLabel, 1, 0, 1, 3);

        if (mModes.count() == 0) {
            mCurrentLabel->setText(i18n("No available resolutions"));
        } else if (mModes.count() == 1) {
            mCurrentLabel->setText(Utils::sizeToString(mModes.first()));
        } else {
            mSlider = new QSlider(Qt::Horizontal, this);
            mSlider->setTickInterval(1);
            mSlider->setTickPosition(QSlider::TicksBelow);
            mSlider->setSingleStep(1);
            mSlider->setPageStep(1);
            mSlider->setMinimum(0);
            mSlider->setMaximum(mModes.size() - 1);
            mSlider->setSingleStep(1);
            if (output->currentMode()) {
                mSlider->setValue(mModes.indexOf(output->currentMode()->size()));
            } else if (output->preferredMode()) {
                mSlider->setValue(mModes.indexOf(output->preferredMode()->size()));
            } else {
                mSlider->setValue(mSlider->maximum());
            }
            layout->addWidget(mSlider, 0, 1);
            connect(mSlider, &QSlider::valueChanged,
                    this, &ResolutionSlider::slotValueChanged);

            mSmallestLabel = new QLabel(this);
            mSmallestLabel->setText(Utils::sizeToString(mModes.first()));
            layout->addWidget(mSmallestLabel, 0, 0);
            mBiggestLabel = new QLabel(this);
            mBiggestLabel->setText(Utils::sizeToString(mModes.last()));
            layout->addWidget(mBiggestLabel, 0, 2);

            mCurrentLabel->setText(Utils::sizeToString(mModes.at(mSlider->value())));
        }
    }
}

ResolutionSlider::~ResolutionSlider()
{
}

QSize ResolutionSlider::currentResolution() const
{
    if (mModes.isEmpty()) {
        return QSize();
    }

    if (mModes.size() < 2) {
        return mModes.first();
    }

    if (mSlider) {
        return mModes.at(mSlider->value());
    } else {
        const int i = mComboBox->currentIndex();
        return i > -1 ? mModes.at(i) : QSize();
    }
}

void ResolutionSlider::slotOutputModeChanged()
{
    if (!mOutput->currentMode()) {
        return;
    }

    if (mSlider) {
        mSlider->blockSignals(true);
        mSlider->setValue(mModes.indexOf(mOutput->currentMode()->size()));
        mSlider->blockSignals(false);
    } else if (mComboBox) {
        mComboBox->blockSignals(true);
        mComboBox->setCurrentIndex(mModes.indexOf(mOutput->currentMode()->size()));
        mComboBox->blockSignals(false);
    }
}

void ResolutionSlider::slotValueChanged(int value)
{
    const QSize &size = mModes.at(value);
    if (mCurrentLabel) {
        mCurrentLabel->setText(Utils::sizeToString(size));
    }

    Q_EMIT resolutionChanged(size);
}
