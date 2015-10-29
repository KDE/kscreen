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
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

#include <KLocalizedString>
#include <QComboBox>

#include <kscreen/output.h>
#include <kscreen/edid.h>

OutputConfig::OutputConfig(QWidget *parent)
    : QWidget(parent)
    , mOutput(0)
{
}

OutputConfig::OutputConfig(const KScreen::OutputPtr &output, QWidget *parent)
    : QWidget(parent)
{
    setOutput(output);
}

OutputConfig::~OutputConfig()
{
}

void OutputConfig::setTitle(const QString& title)
{
    mTitle->setText(title);
}


void OutputConfig::initUi()
{
    connect(mOutput.data(), &KScreen::Output::isConnectedChanged,
            this, [=]() {
                if (!mOutput->isConnected()) {
                    setVisible(false);
                }
            });

    connect(mOutput.data(), &KScreen::Output::isEnabledChanged,
            this, [=]() {
                mEnabled->blockSignals(true);
                mEnabled->setChecked(mOutput->isEnabled());
                mEnabled->blockSignals(false);
            });

    connect(mOutput.data(), &KScreen::Output::rotationChanged,
            this, [=]() {
                const int index = mRotation->findData(mOutput->rotation());
                mRotation->blockSignals(true);
                mRotation->setCurrentIndex(index);
                mRotation->blockSignals(false);
            });

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    mTitle = new QLabel(this);
    mTitle->setAlignment(Qt::AlignHCenter);
    vbox->addWidget(mTitle);

    setTitle(Utils::outputName(mOutput));

    QFormLayout *formLayout = new QFormLayout();
    vbox->addLayout(formLayout);

    mEnabled = new QCheckBox(i18n("Enabled"), this);
    mEnabled->setChecked(mOutput->isEnabled());
    connect(mEnabled, &QCheckBox::clicked,
            this, [=](bool checked) {
                  mOutput->setEnabled(checked);
                  qDebug() << mOutput.data() << mOutput->name() << mOutput->isEnabled();
                  Q_EMIT changed();
            });
    formLayout->addRow(i18n("Display:"), mEnabled);

    mResolution = new ResolutionSlider(mOutput, this);
    connect(mResolution, &ResolutionSlider::resolutionChanged,
            this, &OutputConfig::slotResolutionChanged);
    formLayout->addRow(i18n("Resolution:"), mResolution);

    mRotation = new QComboBox(this);
    mRotation->addItem(QIcon::fromTheme(QStringLiteral("arrow-up")), i18n("Normal"), KScreen::Output::None);
    mRotation->addItem(QIcon::fromTheme(QStringLiteral("arrow-left")), i18n("90° Clockwise"), KScreen::Output::Left);
    mRotation->addItem(QIcon::fromTheme(QStringLiteral("arrow-down")), i18n("Upside Down"), KScreen::Output::Inverted);
    mRotation->addItem(QIcon::fromTheme(QStringLiteral("arrow-right")), i18n("90° Counterclockwise"), KScreen::Output::Right);
    connect(mRotation, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OutputConfig::slotRotationChanged);
    mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));

    formLayout->addRow(i18n("Orientation:"), mRotation);

    formLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));

    CollapsableButton *advancedButton = new CollapsableButton(i18n("Advanced Settings"), this);
    advancedButton->setCollapsed(true);
    vbox->addWidget(advancedButton);

    QWidget *advancedWidget = new QWidget(this);
    int leftMargin, topMargin, rightMargin, bottomMargin;
    advancedWidget->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    advancedWidget->setContentsMargins(25, topMargin, rightMargin, bottomMargin);
    vbox->addWidget(advancedWidget);
    advancedButton->setWidget(advancedWidget);

    formLayout = new QFormLayout(advancedWidget);
    advancedWidget->setLayout(formLayout);

    mRefreshRate = new QComboBox(advancedWidget);
    mRefreshRate->addItem(i18n("Auto"), -1);
    formLayout->addRow(i18n("Refresh rate:"), mRefreshRate);
    slotResolutionChanged(mResolution->currentResolution());
    connect(mRefreshRate, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OutputConfig::slotRefreshRateChanged);

    vbox->addStretch(2);
}

void OutputConfig::setOutput(const KScreen::OutputPtr &output)
{
    mOutput = output;
    initUi();
}

KScreen::OutputPtr OutputConfig::output() const
{
    return mOutput;
}


void OutputConfig::slotResolutionChanged(const QSize &size)
{
    // Ignore disconnected outputs
    if (!size.isValid()) {
        return;
    }

    KScreen::ModePtr selectedMode;
    QList<KScreen::ModePtr> modes;
    Q_FOREACH (const KScreen::ModePtr mode, mOutput->modes()) {
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
    mRefreshRate->blockSignals(true);
    for (int i = 1; i < mRefreshRate->count(); ++i) {
        mRefreshRate->removeItem(i);
    }

    for (int i = 0; i < modes.count(); i++) {
        const KScreen::ModePtr mode = modes.at(i);
        mRefreshRate->addItem(i18n("%1 Hz", QString::fromLatin1("%1").arg(mode->refreshRate(), 0, 'f', 1)), mode->id());

        // If selected refresh rate is other then what we consider the "Auto" value
        // - that is it's not the highest resolution - then select it, otherwise
        // we stick with "Auto"
        if (mode == selectedMode && i > 1) {
            mRefreshRate->setCurrentIndex(i);
        }
    }
    mRefreshRate->blockSignals(false);

    Q_EMIT changed();
}

void OutputConfig::slotRotationChanged(int index)
{
    KScreen::Output::Rotation rotation = 
        static_cast<KScreen::Output::Rotation>(mRotation->itemData(index).toInt());
    mOutput->setRotation(rotation);

    Q_EMIT changed();
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

    Q_EMIT changed();
}
