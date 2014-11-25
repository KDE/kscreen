/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "generator.h"
#include "device.h"
#include "debug.h"

#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QLoggingCategory>

#include <kscreen/config.h>

Generator* Generator::instance = 0;

bool operator<(const QSize &s1, const QSize &s2)
{
    return s1.width() * s1.height() < s2.width() * s2.height();
}

Generator* Generator::self()
{
    if (!Generator::instance) {
        Generator::instance = new Generator();
    }
    return Generator::instance;
}

Generator::Generator()
 : QObject()
 , m_forceLaptop(false)
 , m_forceLidClosed(false)
 , m_forceNotLaptop(false)
 , m_forceDocked(false)
{
    connect(Device::self(), SIGNAL(ready()), SIGNAL(ready()));
}

void Generator::destroy()
{
    delete Generator::instance;
    Generator::instance = 0;
}

Generator::~Generator()
{
}

void Generator::setCurrentConfig(const KScreen::ConfigPtr &currentConfig)
{
    m_currentConfig = currentConfig;
}


KScreen::ConfigPtr Generator::idealConfig(const KScreen::ConfigPtr &currentConfig)
{
    Q_ASSERT(currentConfig);

//     KDebug::Block idealBlock("Ideal Config");
    KScreen::ConfigPtr config = currentConfig->clone();

    disableAllDisconnectedOutputs(config->outputs());

    KScreen::OutputList outputs = config->connectedOutputs();

    qCDebug(KSCREEN_KDED) << "Connected outputs: " << outputs.count();

    if (outputs.isEmpty()) {
        return config;
    }

    if (outputs.count() == 1) {
        singleOutput(outputs);
        return config;
    }

    if (isLaptop()) {
        laptop(outputs);
        return fallbackIfNeeded(config);
    }

    qCDebug(KSCREEN_KDED) << "Extend to Right";
    extendToRight(outputs);

    return fallbackIfNeeded(config);
}

KScreen::ConfigPtr Generator::fallbackIfNeeded(const KScreen::ConfigPtr &config)
{
    qCDebug(KSCREEN_KDED) << "fallbackIfNeeded()";

    KScreen::ConfigPtr newConfig;

    //If the ideal config can't be applied, try clonning
    if (!KScreen::Config::canBeApplied(config)) {
        if (isLaptop()) {
            newConfig = displaySwitch(1);// Try to clone at our best
        } else {
            newConfig = config;
            KScreen::OutputList outputList = config->connectedOutputs();
            outputList.value(outputList.keys().first())->setPrimary(true);
            cloneScreens(outputList);
        }
    } else {
        newConfig = config;
    }

    //If after trying to clone at our best, we fail... return current
    if (!KScreen::Config::canBeApplied(newConfig)) {
        qCDebug(KSCREEN_KDED) << "Config cannot be applied";
        newConfig = config;
    }

    return config;
}

KScreen::ConfigPtr Generator::displaySwitch(int iteration)
{
//     KDebug::Block switchBlock("Display Switch");
    KScreen::ConfigPtr config = m_currentConfig;
    Q_ASSERT(config);


    KScreen::OutputList outputs = config->connectedOutputs();

    if (outputs.count() < 2) {
        singleOutput(outputs);
        return config;
    }

    if (outputs.count() > 2) {
        extendToRight(outputs);
        return config;
    }

    if (iteration == 1) {
        qCDebug(KSCREEN_KDED) << "Cloning";
        embeddedOutput(outputs)->setPrimary(true);
        cloneScreens(outputs);

        return config;
    }

    KScreen::OutputPtr embedded, external;
    embedded = embeddedOutput(outputs);
    outputs.remove(embedded->id());
    external = outputs.value(outputs.keys().first());


    if (iteration == 2) {
        qCDebug(KSCREEN_KDED) << "Extend to left";
        external->setEnabled(true);
        external->setPos(QPoint(0,0));
        external->setCurrentModeId(external->preferredModeId());

        QSize size = external->currentMode()->size();
        embedded->setPos(QPoint(size.width(), 0));
        embedded->setEnabled(true);
        embedded->setCurrentModeId(embedded->preferredModeId());
        embedded->setPrimary(true);
        return config;
    }

    if (iteration == 3) {
        qCDebug(KSCREEN_KDED) << "Turn of embedded (laptop)";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        external->setEnabled(true);
        external->setPrimary(true);
        external->setCurrentModeId(external->preferredModeId());
        return config;
    }

    if (iteration == 4) {
        qCDebug(KSCREEN_KDED) << "Turn off external screen";
        embedded->setEnabled(true);
        embedded->setPrimary(true);
        embedded->setPos(QPoint(0,0));
        embedded->setCurrentModeId(embedded->preferredModeId());

        external->setEnabled(false);
        external->setPrimary(false);
        return config;
    }

    if (iteration == 5) {
        qCDebug(KSCREEN_KDED) << "Extend to the right";
        embedded->setPos(QPoint(0,0));
        embedded->setCurrentModeId(embedded->preferredModeId());
        embedded->setPrimary(true);
        embedded->setEnabled(true);

        QSize size = embedded->currentMode()->size();
        external->setPos(QPoint(size.width(), 0));
        external->setEnabled(true);
        external->setCurrentModeId(external->preferredModeId());
        external->setPrimary(false);

        return config;
    }

    return config;
}

uint qHash(const QSize &size)
{
    return size.width() * size.height();
}

void Generator::cloneScreens(KScreen::OutputList& outputs)
{
    if (outputs.isEmpty()) {
        qCWarning(KSCREEN_KDED) << "output list to clone is empty";
        return;
    }

    QSet<QSize> commonSizes;
    const QSize maxSize  = m_currentConfig->screen()->maxSize();

    QList<QSet<QSize> >modes;
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        QSet<QSize> modeSizes;
        const KScreen::ModeList modes = output->modes();
        Q_FOREACH(const KScreen::ModePtr &mode, modes) {
            const QSize size = mode->size();
            if (size.width() > maxSize.width() || size.height() > maxSize.height()) {
                continue;
            }
            modeSizes.insert(mode->size());
        }

        //If we have nothing to compare against
        if (commonSizes.isEmpty()) {
            commonSizes = modeSizes;
            continue;
        }

        commonSizes.intersect(modeSizes);
    }

    qCDebug(KSCREEN_KDED) << "Common sizes: " << commonSizes;
    //fallback to biggestMode if no commonSizes have been found
    if (commonSizes.isEmpty()) {
        Q_FOREACH(KScreen::OutputPtr output, outputs) {
            output->setEnabled(true);
            output->setPos(QPoint(0, 0));
            output->setCurrentModeId(biggestMode(output->modes())->id());
        }
        return;
    }


    //At this point, we know we have common sizes, let's get the biggest on
    QList<QSize> commonSizeList = commonSizes.toList();
    qSort(commonSizeList.begin(), commonSizeList.end());
    QSize biggestSize = commonSizeList.last();

    //Finally, look for the mode with biggestSize and biggest refreshRate and set it
    qCDebug(KSCREEN_KDED) << "Biggest Size: " << biggestSize;
    KScreen::ModePtr bestMode;
    Q_FOREACH(KScreen::OutputPtr output, outputs) {
        bestMode = bestModeForSize(output->modes(), biggestSize);
        output->setEnabled(true);
        output->setPos(QPoint(0, 0));
        output->setCurrentModeId(bestMode->id());
    }
}

void Generator::singleOutput(KScreen::OutputList &outputs)
{
    Q_ASSERT(!outputs.isEmpty());

    KScreen::OutputPtr output = outputs.take(outputs.keys().first());
    Q_ASSERT(output);

    output->setCurrentModeId(output->preferredModeId());
    output->setEnabled(true);
    output->setPrimary(true);
    output->setPos(QPoint(0,0));
}

void Generator::laptop(KScreen::OutputList &outputs)
{
    Q_ASSERT(!outputs.isEmpty());

//     KDebug::Block laptopBlock("Laptop config");

    KScreen::OutputPtr embedded = embeddedOutput(outputs);
    /* Apparently older laptops use "VGA-*" as embedded output ID, so embeddedOutput()
     * will fail, because it looks only for modern "LVDS", "EDP", etc. If we
     * fail to detect which output is embedded, just use the one  with the lowest
     * ID. It's a wild guess, but I think it's highly probable that it will work.
     * See bug #318907 for further reference. -- dvratil
     */
    if (!embedded) {
        QList<int> keys = outputs.keys();
        qSort(keys);
        embedded = outputs.value(keys.first());
    }
    outputs.remove(embedded->id());

    if (outputs.isEmpty()) {
        qCWarning(KSCREEN_KDED) << "No external outputs found, going for singleOutput()";
        outputs.insert(embedded->id(), embedded);
        return singleOutput(outputs);
    }

    if (isLidClosed() && outputs.count() == 1) {
        qCDebug(KSCREEN_KDED) << "With lid closed";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        KScreen::OutputPtr external = outputs.value(outputs.keys().first());
        external->setEnabled(true);
        external->setPrimary(true);
        external->setCurrentModeId(external->preferredModeId());
        external->setPos(QPoint(0, 0));

        return;
    }

    if (isLidClosed() && outputs.count() > 1) {
        qCDebug(KSCREEN_KDED) << "Lid is closed, and more than one output";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        extendToRight(outputs);
        return;
    }

    qCDebug(KSCREEN_KDED) << "Lid is open";
    //If lid is open, laptop screen shuold be primary
    embedded->setPos(QPoint(0,0));
    embedded->setCurrentModeId(embedded->preferredModeId());
    embedded->setPrimary(true);
    embedded->setEnabled(true);

    int globalWidth;
    if (embedded->isHorizontal()) {
        globalWidth = embedded->preferredMode()->size().width();
    } else {
        globalWidth = embedded->preferredMode()->size().height();
    }
    KScreen::OutputPtr biggest = biggestOutput(outputs);
    outputs.remove(biggest->id());

    biggest->setPos(QPoint(globalWidth, 0));
    biggest->setEnabled(true);
    biggest->setCurrentModeId(biggest->preferredModeId());
    biggest->setPrimary(false);

    if (biggest->isHorizontal()) {
        globalWidth += biggest->currentMode()->size().width();
    } else {
        globalWidth += biggest->currentMode()->size().height();
    }
    Q_FOREACH(KScreen::OutputPtr output, outputs) {
        output->setEnabled(true);
        output->setCurrentModeId(output->preferredModeId());
        output->setPos(QPoint(globalWidth, 0));
        output->setPrimary(false);

        if (output->isHorizontal()) {
            globalWidth += output->currentMode()->size().width();
        } else {
            globalWidth += output->currentMode()->size().height();
        }
    }

    if (isDocked()) {
        qCDebug(KSCREEN_KDED) << "Docked";
        embedded->setPrimary(false);
        biggest->setPrimary(true);
    }
}

void Generator::extendToRight(KScreen::OutputList &outputs)
{
    Q_ASSERT(!outputs.isEmpty());

    qCDebug(KSCREEN_KDED) << "Extending to the right";
    KScreen::OutputPtr biggest = biggestOutput(outputs);
    Q_ASSERT(biggest);

    outputs.remove(biggest->id());

    biggest->setEnabled(true);
    biggest->setPrimary(true);
    biggest->setCurrentModeId(biggest->preferredModeId());
    biggest->setPos(QPoint(0,0));

    int globalWidth;
    if (biggest->isHorizontal()) {
        globalWidth = biggest->currentMode()->size().width();
    } else {
        globalWidth = biggest->currentMode()->size().height();
    }

    Q_FOREACH(KScreen::OutputPtr output, outputs) {
        output->setEnabled(true);
        output->setPrimary(false);
        output->setCurrentModeId(output->preferredModeId());
        output->setPos(QPoint(globalWidth, 0));

        if (output->isHorizontal()) {
            globalWidth += output->currentMode()->size().width();
        } else {
            globalWidth += output->currentMode()->size().height();
        }
    }
}

KScreen::ModePtr Generator::biggestMode(const KScreen::ModeList &modes)
{
    int modeArea, biggestArea = 0;
    KScreen::ModePtr biggestMode;
    Q_FOREACH(const KScreen::ModePtr &mode, modes) {
        modeArea = mode->size().width() * mode->size().height();
        if (modeArea < biggestArea) {
            continue;
        }
        if (modeArea == biggestArea && mode->refreshRate() < biggestMode->refreshRate()) {
            continue;
        }
        if (modeArea == biggestArea && mode->refreshRate() > biggestMode->refreshRate()) {
            biggestMode = mode;
            continue;
        }

        biggestArea = modeArea;
        biggestMode = mode;
    }

    return biggestMode;
}

KScreen::ModePtr Generator::bestModeForSize(const KScreen::ModeList &modes, const QSize &size)
{
    KScreen::ModePtr bestMode;
    Q_FOREACH(const KScreen::ModePtr &mode, modes) {
        if (mode->size() != size) {
            continue;
        }

        if (!bestMode) {
            bestMode = mode;
            continue;
        }

        if (mode->refreshRate() > bestMode->refreshRate()) {
            bestMode = mode;
        }
    }

    return bestMode;
}

KScreen::OutputPtr Generator::biggestOutput(const KScreen::OutputList &outputs)
{
    int area, total = 0;
    KScreen::OutputPtr biggest;
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        const KScreen::ModePtr mode = output->preferredMode();
        area = mode->size().width() * mode->size().height();
        if (area <= total) {
            continue;
        }

        total = area;
        biggest = output;
    }

    return biggest;
}

void Generator::disableAllDisconnectedOutputs(const KScreen::OutputList &outputs)
{
//     KDebug::Block disableBlock("Disabling disconnected screens");
    Q_FOREACH(KScreen::OutputPtr output, outputs) {
        if (!output->isConnected()) {
            qCDebug(KSCREEN_KDED) << output->name() << " Disabled";
            output->setEnabled(false);
            output->setPrimary(false);
        }
    }
}

KScreen::OutputPtr Generator::embeddedOutput(const KScreen::OutputList &outputs)
{
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (output->type() != KScreen::Output::Panel) {
            continue;
        }

        return output;
    }

    return KScreen::OutputPtr();
}

bool Generator::isLaptop()
{
    if (m_forceLaptop) {
        return true;
    }
    if (m_forceNotLaptop) {
        return false;
    }

    return Device::self()->isLaptop();
}

bool Generator::isLidClosed()
{
    if (m_forceLidClosed) {
        return true;
    }
    if (m_forceNotLaptop) {
        return false;
    }

    return Device::self()->isLidClosed();
}

bool Generator::isDocked()
{
    if (m_forceDocked) {
        return true;
    }

    return Device::self()->isDocked();
}

void Generator::setForceLaptop(bool force)
{
    m_forceLaptop = force;
}

void Generator::setForceLidClosed(bool force)
{
    m_forceLidClosed = force;
}

void Generator::setForceDocked(bool force)
{
    m_forceDocked = force;
}

void Generator::setForceNotLaptop(bool force)
{
    m_forceNotLaptop = force;
}

