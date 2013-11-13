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

#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusConnection>

#include <kdebug.h>
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

KScreen::Config* Generator::idealConfig()
{
    KDebug::Block idealBlock("Ideal Config");
    KScreen::Config* config = KScreen::Config::current();
    if (!config) {
        kDebug() << "Unable get current config";
        return 0;
    }

    disableAllDisconnectedOutputs(config->outputs());

    KScreen::OutputList outputs = config->connectedOutputs();

    kDebug() << "Connected outputs: " << outputs.count();

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

    kDebug() << "Extend to Right";
    extendToRight(outputs);

    return fallbackIfNeeded(config);
}

KScreen::Config* Generator::fallbackIfNeeded(KScreen::Config* config)
{
    kDebug();
    //If the ideal config can't be applied, try clonning
    if (!KScreen::Config::canBeApplied(config)) {
        delete config;
        if (Device::self()->isLaptop()) {
            config = displaySwitch(1);// Try to clone at our best
        } else {
            config = KScreen::Config::current();
            KScreen::OutputList outputList = config->connectedOutputs();
            outputList.value(outputList.keys().first())->setPrimary(true);
            cloneScreens(outputList);
        }
    }

    //If after trying to clone at our best, we fail... return current
    if (!KScreen::Config::canBeApplied(config)) {
        kDebug() << "Can't be applied";
        delete config;
        config = KScreen::Config::current();
    }

    return config;
}

KScreen::Config* Generator::displaySwitch(int iteration)
{
    KDebug::Block switchBlock("Display Switch");
    KScreen::Config* config = KScreen::Config::current();
    if (!config) {
        kDebug() << "Unable to get current config";
        return 0;
    }

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
        kDebug() << "Cloning";
        embeddedOutput(outputs)->setPrimary(true);
        cloneScreens(outputs);

        return config;
    }

    KScreen::Output* embedded, *external;
    embedded = embeddedOutput(outputs);
    outputs.remove(embedded->id());
    external = outputs.value(outputs.keys().first());


    if (iteration == 2) {
        kDebug() << "Extend to left";
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
        kDebug() << "Turn of embedded (laptop)";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        external->setEnabled(true);
        external->setPrimary(true);
        external->setCurrentModeId(external->preferredModeId());
        return config;
    }

    if (iteration == 4) {
        kDebug() << "Turn off external screen";
        embedded->setEnabled(true);
        embedded->setPrimary(true);
        embedded->setPos(QPoint(0,0));
        embedded->setCurrentModeId(embedded->preferredModeId());

        external->setEnabled(false);
        external->setPrimary(false);
        return config;
    }

    if (iteration == 5) {
        kDebug() << "Extend to the right";
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
        kWarning() << "output list to clone is empty";
        return;
    }

    QSet<QSize> commonSizes;
    const QSize maxSize  = KScreen::Config::current()->screen()->maxSize();

    QList<QSet<QSize> >modes;
    Q_FOREACH(KScreen::Output *output, outputs) {
        QSet<QSize> modeSizes;
        KScreen::ModeList modes = output->modes();
        Q_FOREACH(KScreen::Mode *mode, modes) {
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

    kDebug() << "Common sizes: " << commonSizes;
    //fallback to biggestMode if no commonSizes have been found
    if (commonSizes.isEmpty()) {
        Q_FOREACH(KScreen::Output *output, outputs) {
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
    kDebug() << "Biggest Size: " << biggestSize;
    KScreen::Mode* bestMode;
    Q_FOREACH(KScreen::Output *output, outputs) {
        bestMode = bestModeForSize(output->modes(), biggestSize);
        output->setEnabled(true);
        output->setPos(QPoint(0, 0));
        output->setCurrentModeId(bestMode->id());
    }
}

void Generator::singleOutput(KScreen::OutputList& outputs)
{
    Q_ASSERT(!outputs.isEmpty());

    KScreen::Output* output = outputs.take(outputs.keys().first());
    Q_ASSERT(output);

    output->setCurrentModeId(output->preferredModeId());
    output->setEnabled(true);
    output->setPrimary(true);
    output->setPos(QPoint(0,0));
}

void Generator::laptop(KScreen::OutputList& outputs)
{
    Q_ASSERT(!outputs.isEmpty());

    KDebug::Block laptopBlock("Laptop config");

    KScreen::Output* embedded = embeddedOutput(outputs);
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
        kWarning() << "No external outputs found, going for singleOutput()";
        outputs.insert(embedded->id(), embedded);
        return singleOutput(outputs);
    }

    if (isLidClosed() && outputs.count() == 1) {
        kDebug() << "With lid closed";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        KScreen::Output* external = outputs.value(outputs.keys().first());
        external->setEnabled(true);
        external->setPrimary(true);
        external->setCurrentModeId(external->preferredModeId());
        external->setPos(QPoint(0, 0));

        return;
    }

    if (isLidClosed() && outputs.count() > 1) {
        kDebug() << "Lid is closed, and more than one output";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        extendToRight(outputs);
        return;
    }

    kDebug() << "Lid is open";
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
    KScreen::Output* biggest = biggestOutput(outputs);
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
    Q_FOREACH(KScreen::Output* output, outputs) {
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
        kDebug() << "Docked";
        embedded->setPrimary(false);
        biggest->setPrimary(true);
    }
}

void Generator::extendToRight(KScreen::OutputList& outputs)
{
    Q_ASSERT(!outputs.isEmpty());

    kDebug() << "Extending to the right";
    KScreen::Output* biggest = biggestOutput(outputs);
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

    Q_FOREACH(KScreen::Output* output, outputs) {
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

KScreen::Mode* Generator::biggestMode(const KScreen::ModeList& modes)
{
    int modeArea, biggestArea = 0;
    KScreen::Mode* biggestMode = 0;
    Q_FOREACH(KScreen::Mode* mode, modes) {
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

KScreen::Mode* Generator::bestModeForSize(const KScreen::ModeList& modes, const QSize &size)
{
    KScreen::Mode *bestMode = 0;
    Q_FOREACH(KScreen::Mode *mode, modes) {
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

KScreen::Output* Generator::biggestOutput(const KScreen::OutputList &outputs)
{
    int area, total = 0;
    KScreen::Output* biggest = 0;
    Q_FOREACH(KScreen::Output* output, outputs) {
        KScreen::Mode* mode = output->preferredMode();
        area = mode->size().width() * mode->size().height();
        if (area <= total) {
            continue;
        }

        total = area;
        biggest = output;
    }

    return biggest;
}

void Generator::disableAllDisconnectedOutputs(const KScreen::OutputList& outputs)
{
    KDebug::Block disableBlock("Disabling disconnected screens");
    Q_FOREACH(KScreen::Output* output, outputs) {
        if (!output->isConnected()) {
            kDebug() << output->name() << " Disabled";
            output->setEnabled(false);
            output->setPrimary(false);
        }
    }
}

KScreen::Output* Generator::embeddedOutput(const KScreen::OutputList& outputs)
{
    Q_FOREACH(KScreen::Output* output, outputs) {
        if (output->type() != KScreen::Output::Panel) {
            continue;
        }

        return output;
    }

    return 0;
}

bool Generator::isLaptop()
{
    if (m_forceLaptop) {
        return true;
    }

    return Device::self()->isLaptop();
}

bool Generator::isLidClosed()
{
    if (m_forceLidClosed) {
        return true;
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

#include "generator.moc"
