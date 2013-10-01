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

#include "outputconfig.h"
#include "resolutionslider.h"
#include "collapsablebutton.h"
#include "utils.h"

#include <QtCore/QStringBuilder>
#include <QtGui/QFormLayout>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QGroupBox>

#include <KLocalizedString>
#include <KComboBox>
#include <KIcon>

#include <kscreen/output.h>
#include <kscreen/edid.h>

OutputConfig::OutputConfig(KScreen::Output *output, QWidget *parent)
    : QWidget(parent)
    , mOutput(output)
{
    connect(output, SIGNAL(isConnectedChanged()), SLOT(slotOutputConnectedChanged()));
    connect(output, SIGNAL(isEnabledChanged()), SLOT(slotOutputEnabledChanged()));
    connect(output, SIGNAL(rotationChanged()), SLOT(slotOutputRotationChanged()));

    QGridLayout *layout = new QGridLayout(this);

    mLabel = new CollapsableButton(Utils::outputName(output), this);
    layout->addWidget(mLabel, 0, 0, 1, 2);

    layout->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum), 1, 0);

    mControlsWidget = new QGroupBox(this);
    mLabel->setWidget(mControlsWidget);
    layout->addWidget(mControlsWidget, 1, 1);

    QGridLayout *formLayout = new QGridLayout(mControlsWidget);

    mEnabled = new QCheckBox(i18n("Enabled"), mControlsWidget);
    mEnabled->setChecked(output->isEnabled());
    connect(mEnabled, SIGNAL(clicked(bool)), SLOT(slotEnabledChanged(bool)));
    formLayout->addWidget(new QLabel(i18n("Display:"), this), 0, 0);
    formLayout->addWidget(mEnabled, 0, 1);

    mResolution = new ResolutionSlider(output, mControlsWidget);
    connect(mResolution, SIGNAL(resolutionChanged(QSize)), SLOT(slotResolutionChanged(QSize)));
    formLayout->addWidget(new QLabel(i18n("Resolution:"), this), 1, 0);
    formLayout->addWidget(mResolution, 1, 1);

    mRotation = new KComboBox(mControlsWidget);
    connect(mRotation, SIGNAL(currentIndexChanged(int)), SLOT(slotRotationChanged(int)));
    mRotation->addItem(KIcon(QLatin1String("arrow-up")), i18n("Normal"), KScreen::Output::None);
    mRotation->addItem(KIcon(QLatin1String("arrow-left")), i18n("90° clockwise"), KScreen::Output::Left);
    mRotation->addItem(KIcon(QLatin1String("arrow-down")), i18n("Upside down"), KScreen::Output::Inverted);
    mRotation->addItem(KIcon(QLatin1String("arrow-right")), i18n("90° counterclockwise"), KScreen::Output::Right);
    formLayout->addWidget(new QLabel(i18n("Orientation:"), this), 2, 0);
    formLayout->addWidget(mRotation, 2, 1);

    formLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2, 3, 1);

    CollapsableButton *advanced = new CollapsableButton(i18n("Advanced Settings"), this);
    advanced->setCollapsed(true);
    formLayout->addWidget(advanced, 3, 0, 1, 2);

    QWidget *advancedWidget = new QWidget(mControlsWidget);
    formLayout->addWidget(advancedWidget, 4, 0, 1, 2);
    advanced->setWidget(advancedWidget);
    formLayout = new QGridLayout(advancedWidget);
    advancedWidget->setLayout(formLayout);

    mRefreshRate = new KComboBox(advancedWidget);
    mRefreshRate->addItem(i18n("Auto"), -1);
    formLayout->addWidget(new QLabel(i18n("Refresh Rate:"), this), 0, 0);
    formLayout->addWidget(mRefreshRate, 0, 1);
    slotResolutionChanged(mResolution->currentResolution());
    connect(mRefreshRate, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotRefreshRateChanged(int)));

    if (!output->isEnabled()) {
        collapse();
    }

    if (!output->isConnected()) {
        hide();
    }
}

OutputConfig::~OutputConfig()
{
}

KScreen::Output* OutputConfig::output() const
{
    return mOutput;
}

bool OutputConfig::isExpanded() const
{
    return mControlsWidget->isVisible();
}

void OutputConfig::collapse()
{
    mLabel->setCollapsed(true);
}

void OutputConfig::expand()
{
    mLabel->setCollapsed(false);
}

void OutputConfig::toggle()
{
    mLabel->toggle();
}

void OutputConfig::slotOutputConnectedChanged()
{
    setVisible(mOutput->isConnected());
}

void OutputConfig::slotOutputEnabledChanged()
{
    mEnabled->setChecked(mOutput->isEnabled());
}

void OutputConfig::slotOutputRotationChanged()
{
    const int index = mRotation->findData(mOutput->rotation());
    mRotation->setCurrentIndex(index);
}

void OutputConfig::slotEnabledChanged(bool checked)
{
    mOutput->setEnabled(checked);
}

void OutputConfig::slotResolutionChanged(const QSize &size)
{
    // Ignore disconnected outputs
    if (!size.isValid()) {
        return;
    }

    KScreen::Mode* selectedMode = 0;
    QList<KScreen::Mode*> modes;
    Q_FOREACH (KScreen::Mode *mode, mOutput->modes()) {
        if (mode->size() == size) {
            modes << mode;
            if (!selectedMode || selectedMode->refreshRate() < mode->refreshRate()) {
                selectedMode = mode;
            }
        }
    }

    Q_ASSERT(selectedMode);
    mOutput->setCurrentModeId(selectedMode->id());

    // Don't remove the first "Auto" item - prevents ugly flicker of the combobox
    // when changing resolution
    for (int i = 1; i < mRefreshRate->count(); ++i) {
        mRefreshRate->removeItem(i);
    }

    for (int i = 0; i < modes.count(); i++) {
        KScreen::Mode *mode = modes.at(i);
        mRefreshRate->addItem(QString::fromLatin1("%1 Hz").arg(mode->refreshRate(), 0, 'f', 1), mode->id());

        // If selected refresh rate is other then what we consider the "Auto" value
        // - that is it's not the highest resolution - then select it, otherwise
        // we stick with "Auto"
        if (mode == selectedMode && i > 1) {
            mRefreshRate->setCurrentIndex(i);
        }
    }
}

void OutputConfig::slotRotationChanged(int index)
{
    KScreen::Output::Rotation rotation = 
        static_cast<KScreen::Output::Rotation>(mRotation->itemData(index).toInt());
    mOutput->setRotation(rotation);
}

void OutputConfig::slotRefreshRateChanged(int index)
{
    QString modeId;
    if (index == 0) {
        // Item 0 is "Auto" - "Auto" is equal to highest refresh rate (at least
        // that's how I understand it, and since the combobox is sorted in descending
        // order, we just pick the second item from top
        modeId = mRefreshRate->itemData(1).toString();
    } else {
        modeId = mRefreshRate->itemData(index).toString();
    }
    mOutput->setCurrentModeId(modeId);
}

#include "outputconfig.moc"
