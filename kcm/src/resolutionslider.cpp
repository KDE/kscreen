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

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>

#include <KLocalizedString>

#include <kscreen/output.h>

static bool sizeLessThan(const QSize &sizeA, const QSize &sizeB)
{
    return sizeA.width() * sizeA.height() < sizeB.width() * sizeB.height();
}

ResolutionSlider::ResolutionSlider(KScreen::Output *output, QWidget *parent)
    : QWidget(parent)
    , mOutput(output)
{
    connect(output, SIGNAL(currentModeIdChanged()),
            this, SLOT(slotOutputModeChanged()));

    QGridLayout *layout = new QGridLayout(this);

    mSmallestLabel = new QLabel(this);
    layout->addWidget(mSmallestLabel, 0, 0);

    mSlider = new QSlider(Qt::Horizontal, this);
    mSlider->setTickInterval(1);
    mSlider->setTickPosition(QSlider::TicksBelow);
    mSlider->setSingleStep(1);
    mSlider->setPageStep(1);
    layout->addWidget(mSlider, 0, 1);
    connect(mSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSlideValueChanged(int)));

    mBiggestLabel = new QLabel(this);
    layout->addWidget(mBiggestLabel, 0, 2);

    mCurrentLabel = new QLabel(this);
    mCurrentLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(mCurrentLabel, 1, 0, 1, 3);

    Q_FOREACH (KScreen::Mode* mode, output->modes()) {
        if (mModes.contains(mode->size())) {
            continue;
        }

        mModes << mode->size();
    }

    qSort(mModes.begin(), mModes.end(), sizeLessThan);

    if (mModes.size() < 2) {
        mSmallestLabel->hide();
        mBiggestLabel->hide();
        mSlider->hide();
        mCurrentLabel->setAlignment(Qt::AlignLeft);
        if (mModes.size() == 0) {
            mCurrentLabel->setText(i18n("No available resolutions"));
        } else {
            mCurrentLabel->setText(Utils::sizeToString(mModes.first()));
        }

        return;
    }

    mSmallestLabel->setText(Utils::sizeToString(mModes.first()));
    mBiggestLabel->setText(Utils::sizeToString(mModes.last()));
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
    slotOutputModeChanged();
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

    return mModes.at(mSlider->value());
}

void ResolutionSlider::slotOutputModeChanged()
{
    if (!mOutput->currentMode()) {
        return;
    }

    mSlider->setValue(mModes.indexOf(mOutput->currentMode()->size()));
}

void ResolutionSlider::slotSlideValueChanged(int value)
{
    const QSize &size = mModes.at(value);
    mCurrentLabel->setText(Utils::sizeToString(size));

    Q_EMIT resolutionChanged(size);
}


#include "resolutionslider.moc"
