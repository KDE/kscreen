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

OutputConfig::OutputConfig(QWidget *parent)
    : QWidget(parent)
    , mOutput(0)
{
}

OutputConfig::OutputConfig(KScreen::Output *output, QWidget *parent)
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
    connect(mOutput, SIGNAL(isConnectedChanged()), SLOT(slotOutputConnectedChanged()));
    connect(mOutput, SIGNAL(isEnabledChanged()), SLOT(slotOutputEnabledChanged()));
    connect(mOutput, SIGNAL(rotationChanged()), SLOT(slotOutputRotationChanged()));

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
    connect(mEnabled, SIGNAL(clicked(bool)), SLOT(slotEnabledChanged(bool)));
    formLayout->addRow(i18n("Display:"), mEnabled);

    mResolution = new ResolutionSlider(mOutput, this);
    connect(mResolution, SIGNAL(resolutionChanged(QSize)), SLOT(slotResolutionChanged(QSize)));
    formLayout->addRow(i18n("Resolution:"), mResolution);

    mRotation = new KComboBox(this);
    connect(mRotation, SIGNAL(currentIndexChanged(int)), SLOT(slotRotationChanged(int)));
    mRotation->addItem(KIcon(QLatin1String("arrow-up")), i18n("Normal"), KScreen::Output::None);
    mRotation->addItem(KIcon(QLatin1String("arrow-left")), i18n("90° clockwise"), KScreen::Output::Left);
    mRotation->addItem(KIcon(QLatin1String("arrow-down")), i18n("Upside down"), KScreen::Output::Inverted);
    mRotation->addItem(KIcon(QLatin1String("arrow-right")), i18n("90° counterclockwise"), KScreen::Output::Right);
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

    mRefreshRate = new KComboBox(advancedWidget);
    mRefreshRate->addItem(i18n("Auto"), -1);
    formLayout->addRow(i18n("Refresh Rate:"), mRefreshRate);
    slotResolutionChanged(mResolution->currentResolution());
    connect(mRefreshRate, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotRefreshRateChanged(int)));

    vbox->addStretch(2);
}

void OutputConfig::setOutput(KScreen::Output *output)
{
    mOutput = output;
    initUi();
}

KScreen::Output* OutputConfig::output() const
{
    return mOutput;
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

    Q_EMIT changed();
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

#include "outputconfig.moc"
