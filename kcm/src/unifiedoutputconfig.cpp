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

#include <KComboBox>
#include <KIcon>
#include <KLocalizedString>
#include <KDebug>

#include <QtGui/QGridLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>

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


UnifiedOutputConfig::UnifiedOutputConfig(KScreen::Config *config, QWidget *parent)
    : OutputConfig(parent)
    , mConfig(config)
{
}

UnifiedOutputConfig::~UnifiedOutputConfig()
{
}

void UnifiedOutputConfig::setOutput(KScreen::Output *output)
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
    setTitle(i18n("Unified Outputs"));

    QVBoxLayout *vbox = new QVBoxLayout(this);
    QGridLayout *formLayout = new QGridLayout();
    vbox->addLayout(formLayout);
    vbox->addStretch(2);

    KScreen::Output *fakeOutput = createFakeOutput();
    mResolution = new ResolutionSlider(fakeOutput, this);
    connect(mResolution, SIGNAL(resolutionChanged(QSize)), SLOT(slotResolutionChanged(QSize)));
    formLayout->addWidget(new QLabel(i18n("Resolution:"), this), 1, 0);
    formLayout->addWidget(mResolution, 1, 1);
    slotResolutionChanged(mResolution->currentResolution());

    mRotation = new KComboBox(this);
    connect(mRotation, SIGNAL(currentIndexChanged(int)), SLOT(slotRotationChanged(int)));
    mRotation->addItem(KIcon(QLatin1String("arrow-up")), i18n("Normal"), KScreen::Output::None);
    mRotation->addItem(KIcon(QLatin1String("arrow-left")), i18n("90° clockwise"), KScreen::Output::Left);
    mRotation->addItem(KIcon(QLatin1String("arrow-down")), i18n("Upside down"), KScreen::Output::Inverted);
    mRotation->addItem(KIcon(QLatin1String("arrow-right")), i18n("90° counterclockwise"), KScreen::Output::Right);
    formLayout->addWidget(new QLabel(i18n("Orientation:"), this), 2, 0);
    formLayout->addWidget(mRotation, 2, 1);

    formLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2, 3, 1);
}

KScreen::Output *UnifiedOutputConfig::createFakeOutput()
{
    // Find set of common resolutions
    QMap<QSize, int> commonSizes;
    Q_FOREACH (KScreen::Output *clone, mClones) {
        QList<QSize> processedSizes;
        Q_FOREACH (KScreen::Mode *mode, clone->modes()) {
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

    KScreen::Output *fakeOutput = new KScreen::Output(this);

    // This will give us list of resolution that are shared by all outputs
    QList<QSize> commonResults = commonSizes.keys(mClones.count());
    // If there are no common resolution, fallback to smallest preferred mode
    if (commonResults.isEmpty()) {
        QSize smallestMode;
        Q_FOREACH (KScreen::Output *clone, mClones) {
            qDebug() << smallestMode << clone->preferredMode()->size();
            if (!smallestMode.isValid() || clone->preferredMode()->size() < smallestMode) {
                smallestMode = clone->preferredMode()->size();
            }
        }
        commonResults << smallestMode;
    }
    qSort(commonResults);

    KScreen::ModeList modes;
    Q_FOREACH (const QSize &size, commonResults) {
        KScreen::Mode *mode = new KScreen::Mode(fakeOutput);
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

    Q_FOREACH (KScreen::Output *clone, mClones) {
        const QString &id = findBestMode(clone, size);
        if (id.isEmpty()) {
            // FIXME: Error?
            return;
        }

        clone->setCurrentModeId(id);
    }

    Q_EMIT changed();
}

QString UnifiedOutputConfig::findBestMode(const KScreen::Output *output, const QSize &size)
{
    float refreshRate = 0;
    QString id;
    Q_FOREACH (KScreen::Mode *mode, output->modes()) {
        if (mode->size() == size && mode->refreshRate() > refreshRate) {
            id = mode->id();
        }
    }

    return id;
}
