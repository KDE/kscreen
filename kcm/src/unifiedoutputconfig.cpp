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

#include "unifiedoutputconfig.h"
#include "collapsablebutton.h"
#include "resolutionslider.h"
#include "utils.h"
#include "kcm_screen_debug.h"

#include <QComboBox>
#include <QIcon>
#include <KLocalizedString>

#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>

#include <kscreen/output.h>
#include <kscreen/config.h>


bool operator<(const QSize &s1, const QSize &s2)
{
    return s1.width() * s1.height() < s2.width() * s2.height();
}

template<>
bool qMapLessThanKey(const QSize &s1, const QSize &s2)
{
    return s1 < s2;
}


UnifiedOutputConfig::UnifiedOutputConfig(const KScreen::ConfigPtr &config, QWidget *parent)
    : OutputConfig(parent)
    , mConfig(config)
{
}

UnifiedOutputConfig::~UnifiedOutputConfig()
{
}

void UnifiedOutputConfig::setOutput(const KScreen::OutputPtr &output)
{
    mOutput = output;

    mClones.clear();
    Q_FOREACH (int id, mOutput->clones()) {
        mClones << mConfig->output(id);
    }
    mClones << mOutput;

    OutputConfig::setOutput(output);
}

void UnifiedOutputConfig::initUi()
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    mTitle = new QLabel(this);
    mTitle->setAlignment(Qt::AlignHCenter);
    vbox->addWidget(mTitle);

    setTitle(i18n("Unified Outputs"));

    QGridLayout *formLayout = new QGridLayout();
    vbox->addLayout(formLayout);
    vbox->addStretch(2);

    KScreen::OutputPtr fakeOutput = createFakeOutput();
    mResolution = new ResolutionSlider(fakeOutput, this);
    connect(mResolution, &ResolutionSlider::resolutionChanged,
            this, &UnifiedOutputConfig::slotResolutionChanged);
    formLayout->addWidget(new QLabel(i18n("Resolution:"), this), 1, 0);
    formLayout->addWidget(mResolution, 1, 1);
    slotResolutionChanged(mResolution->currentResolution());

    mRotation = new QComboBox(this);
    connect(mRotation, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &UnifiedOutputConfig::slotRotationChanged);
    mRotation->addItem(QIcon::fromTheme(QStringLiteral("arrow-up")), i18n("Normal"), KScreen::Output::None);
    mRotation->addItem(QIcon::fromTheme(QStringLiteral("arrow-right")), i18n("90° Clockwise"), KScreen::Output::Right);
    mRotation->addItem(QIcon::fromTheme(QStringLiteral("arrow-down")), i18n("Upside Down"), KScreen::Output::Inverted);
    mRotation->addItem(QIcon::fromTheme(QStringLiteral("arrow-left")), i18n("90° Counterclockwise"), KScreen::Output::Left);
    formLayout->addWidget(new QLabel(i18n("Orientation:"), this), 2, 0);
    formLayout->addWidget(mRotation, 2, 1);

    formLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2, 3, 1);
}

KScreen::OutputPtr UnifiedOutputConfig::createFakeOutput()
{
    // Find set of common resolutions
    QMap<QSize, int> commonSizes;
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        QList<QSize> processedSizes;
        Q_FOREACH (const KScreen::ModePtr &mode, clone->modes()) {
            // Make sure we don't count some modes multiple times because of different
            // refresh rates
            if (processedSizes.contains(mode->size())) {
                continue;
            }

            processedSizes << mode->size();

            if (commonSizes.contains(mode->size())) {
                commonSizes[mode->size()]++;
            } else {
                commonSizes.insert(mode->size(), 1);
            }
        }
    }

    KScreen::OutputPtr fakeOutput(new KScreen::Output);

    // This will give us list of resolution that are shared by all outputs
    QList<QSize> commonResults = commonSizes.keys(mClones.count());
    // If there are no common resolution, fallback to smallest preferred mode
    if (commonResults.isEmpty()) {
        QSize smallestMode;
        Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
            qCDebug(KSCREEN_KCM) << smallestMode << clone->preferredMode()->size();
            if (!smallestMode.isValid() || clone->preferredMode()->size() < smallestMode) {
                smallestMode = clone->preferredMode()->size();
            }
        }
        commonResults << smallestMode;
    }
    qSort(commonResults);

    KScreen::ModeList modes;
    Q_FOREACH (const QSize &size, commonResults) {
        KScreen::ModePtr mode(new KScreen::Mode);
        mode->setSize(size);
        mode->setId(Utils::sizeToString(size));
        mode->setName(mode->id());
        modes.insert(mode->id(), mode);
    }
    fakeOutput->setModes(modes);
    fakeOutput->setCurrentModeId(Utils::sizeToString(commonResults.last()));
    return fakeOutput;
}

void UnifiedOutputConfig::slotResolutionChanged(const QSize &size)
{
   // Ignore disconnected outputs
    if (!size.isValid()) {
        return;
    }

    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        const QString &id = findBestMode(clone, size);
        if (id.isEmpty()) {
            // FIXME: Error?
            return;
        }

        clone->setCurrentModeId(id);
    }

    Q_EMIT changed();
}

QString UnifiedOutputConfig::findBestMode(const KScreen::OutputPtr &output, const QSize &size)
{
    float refreshRate = 0;
    QString id;
    Q_FOREACH (const KScreen::ModePtr &mode, output->modes()) {
        if (mode->size() == size && mode->refreshRate() > refreshRate) {
            refreshRate = mode->refreshRate();
            id = mode->id();
        }
    }

    return id;
}
