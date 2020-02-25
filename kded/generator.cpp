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
#include "kscreen_daemon_debug.h"
#include <QRect>

#include <kscreen/config.h>

#if defined(QT_NO_DEBUG)
#define ASSERT_OUTPUTS(outputs)
#else
#define ASSERT_OUTPUTS(outputs) \
    while(true) { \
        Q_ASSERT(!outputs.isEmpty()); \
        Q_FOREACH (const KScreen::OutputPtr &output, outputs) { \
            Q_ASSERT(output); \
            Q_ASSERT(output->isConnected()); \
        } break; \
    }
#endif

Generator* Generator::instance = nullptr;

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
    connect(Device::self(), &Device::ready, this, &Generator::ready);
}

void Generator::destroy()
{
    delete Generator::instance;
    Generator::instance = nullptr;
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

    KScreen::OutputList connectedOutputs = config->connectedOutputs();
    qCDebug(KSCREEN_KDED) << "Connected outputs: " << connectedOutputs.count();

    if (connectedOutputs.isEmpty()) {
        return config;
    }

    //the scale will generally be independent no matter where the output is
    //scale will affect geometry, so do this first
    if (config->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling)) {
        for(auto output: qAsConst(connectedOutputs)) {
            output->setScale(bestScaleForOutput(output));
        }
    }

    if (connectedOutputs.count() == 1) {
        singleOutput(connectedOutputs);
        return config;
    }

    if (isLaptop()) {
        laptop(connectedOutputs);
        return fallbackIfNeeded(config);
    }

    qCDebug(KSCREEN_KDED) << "Extend to Right";
    extendToRight(connectedOutputs);

    return fallbackIfNeeded(config);
}

KScreen::ConfigPtr Generator::fallbackIfNeeded(const KScreen::ConfigPtr &config)
{
    qCDebug(KSCREEN_KDED) << "fallbackIfNeeded()";

    KScreen::ConfigPtr newConfig;

    //If the ideal config can't be applied, try clonning
    if (!KScreen::Config::canBeApplied(config)) {
        if (isLaptop()) {
            newConfig = displaySwitch(Generator::Clone); // Try to clone at our best
        } else {
            newConfig = config;
            KScreen::OutputList connectedOutputs = config->connectedOutputs();
            if (connectedOutputs.isEmpty()) {
                return config;
            }
            connectedOutputs.value(connectedOutputs.keys().first())->setPrimary(true);
            cloneScreens(connectedOutputs);
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

KScreen::ConfigPtr Generator::displaySwitch(DisplaySwitchAction action)
{
//     KDebug::Block switchBlock("Display Switch");
    KScreen::ConfigPtr config = m_currentConfig;
    Q_ASSERT(config);


    KScreen::OutputList connectedOutputs = config->connectedOutputs();

    //the scale will generally be independent no matter where the output is
    //scale will affect geometry, so do this first
    if (config->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling)) {
        for(auto output: qAsConst(connectedOutputs)) {
            output->setScale(bestScaleForOutput(output));
        }
    }

    // There's not much else we can do with only one output
    if (connectedOutputs.count() < 2) {
        singleOutput(connectedOutputs);
        return config;
    }

    // We cannot try all possible combinations with two and more outputs
    if (connectedOutputs.count() > 2) {
        extendToRight(connectedOutputs);
        return config;
    }

    KScreen::OutputPtr embedded, external;
    embedded = embeddedOutput(connectedOutputs);
    // If we don't have an embedded output (desktop with two external screens
    // for instance), then pretend one of them is embedded
    if (!embedded) {
        embedded = connectedOutputs.value(connectedOutputs.keys().first());
    }
    // Just to be sure
    if (embedded->modes().isEmpty()) {
        return config;
    }

    if (action == Generator::Clone) {
        qCDebug(KSCREEN_KDED) << "Cloning";
        embedded->setPrimary(true);
        cloneScreens(connectedOutputs);
        return config;
    }

    connectedOutputs.remove(embedded->id());
    external = connectedOutputs.value(connectedOutputs.keys().first());
    // Just to be sure
    if (external->modes().isEmpty()) {
        return config;
    }

    switch (action) {
    case Generator::ExtendToLeft: {
        qCDebug(KSCREEN_KDED) << "Extend to left";
        external->setPos(QPoint(0,0));
        external->setEnabled(true);
        const KScreen::ModePtr extMode = bestModeForOutput(external);
        Q_ASSERT(extMode);
        external->setCurrentModeId(extMode->id());

        Q_ASSERT(external->currentMode()); // we must have a mode now
        const QSize size = external->geometry().size();
        embedded->setPos(QPoint(size.width(), 0));
        embedded->setEnabled(true);
        embedded->setPrimary(true);
        const KScreen::ModePtr embeddedMode = bestModeForOutput(embedded);
        Q_ASSERT(embeddedMode);
        embedded->setCurrentModeId(embeddedMode->id());

        return config;
    }
    case Generator::TurnOffEmbedded: {
        qCDebug(KSCREEN_KDED) << "Turn off embedded (laptop)";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        external->setEnabled(true);
        external->setPrimary(true);
        const KScreen::ModePtr extMode = bestModeForOutput(external);
        Q_ASSERT(extMode);
        external->setCurrentModeId(extMode->id());
        return config;
    }
    case Generator::TurnOffExternal: {
        qCDebug(KSCREEN_KDED) << "Turn off external screen";
        embedded->setPos(QPoint(0,0));
        embedded->setEnabled(true);
        embedded->setPrimary(true);
        const KScreen::ModePtr embeddedMode = bestModeForOutput(embedded);
        Q_ASSERT(embeddedMode);
        embedded->setCurrentModeId(embeddedMode->id());

        external->setEnabled(false);
        external->setPrimary(false);
        return config;
    }
    case Generator::ExtendToRight: {
        qCDebug(KSCREEN_KDED) << "Extend to the right";
        embedded->setPos(QPoint(0,0));
        embedded->setEnabled(true);
        embedded->setPrimary(true);
        const KScreen::ModePtr embeddedMode = bestModeForOutput(embedded);
        Q_ASSERT(embeddedMode);
        embedded->setCurrentModeId(embeddedMode->id());


        Q_ASSERT(embedded->currentMode()); // we must have a mode now
        const QSize size = embedded->geometry().size();
        external->setPos(QPoint(size.width(), 0));
        external->setEnabled(true);
        external->setPrimary(false);
        const KScreen::ModePtr extMode = bestModeForOutput(external);
        Q_ASSERT(extMode);
        external->setCurrentModeId(extMode->id());

        return config;
    }
    case Generator::None: // just return config
    case Generator::Clone: // handled above
        break;
    } // switch

    return config;
}

uint qHash(const QSize &size)
{
    return size.width() * size.height();
}

void Generator::cloneScreens(KScreen::OutputList &connectedOutputs)
{
    ASSERT_OUTPUTS(connectedOutputs);
    if (connectedOutputs.isEmpty()) {
        return;
    }

    QSet<QSize> commonSizes;
    const QSize maxScreenSize  = m_currentConfig->screen()->maxSize();

    Q_FOREACH(const KScreen::OutputPtr &output, connectedOutputs) {
        QSet<QSize> modeSizes;
        Q_FOREACH(const KScreen::ModePtr &mode, output->modes()) {
            const QSize size = mode->size();
            if (size.width() > maxScreenSize.width() || size.height() > maxScreenSize.height()) {
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
        Q_FOREACH(KScreen::OutputPtr output, connectedOutputs) {
            if (output->modes().isEmpty()) {
                continue;
            }
            output->setEnabled(true);
            output->setPos(QPoint(0, 0));
            const KScreen::ModePtr mode = biggestMode(output->modes());
            Q_ASSERT(mode);
            output->setCurrentModeId(mode->id());
        }
        return;
    }


    //At this point, we know we have common sizes, let's get the biggest on
    QList<QSize> commonSizeList = commonSizes.values();
    std::sort(commonSizeList.begin(), commonSizeList.end());
    const QSize biggestSize = commonSizeList.last();

    //Finally, look for the mode with biggestSize and biggest refreshRate and set it
    qCDebug(KSCREEN_KDED) << "Biggest Size: " << biggestSize;
    KScreen::ModePtr bestMode;
    Q_FOREACH(KScreen::OutputPtr output, connectedOutputs) {
        if (output->modes().isEmpty()) {
            continue;
        }
        bestMode = bestModeForSize(output->modes(), biggestSize);
        Q_ASSERT(bestMode); // we resolved this mode previously, so it better works
        output->setEnabled(true);
        output->setPos(QPoint(0, 0));
        output->setCurrentModeId(bestMode->id());
    }
}

void Generator::singleOutput(KScreen::OutputList &connectedOutputs)
{
    ASSERT_OUTPUTS(connectedOutputs);
    if (connectedOutputs.isEmpty()) {
        return;
    }

    KScreen::OutputPtr output = connectedOutputs.take(connectedOutputs.keys().first());
    if (output->modes().isEmpty()) {
        return;
    }

    const KScreen::ModePtr bestMode = bestModeForOutput(output);
    Q_ASSERT(bestMode);
    output->setCurrentModeId(bestMode->id());
    output->setEnabled(true);
    output->setPrimary(true);
    output->setPos(QPoint(0,0));
}

void Generator::laptop(KScreen::OutputList &connectedOutputs)
{
    ASSERT_OUTPUTS(connectedOutputs)
    if (connectedOutputs.isEmpty()) {
        return;
    }

//     KDebug::Block laptopBlock("Laptop config");

    KScreen::OutputPtr embedded = embeddedOutput(connectedOutputs);
    /* Apparently older laptops use "VGA-*" as embedded output ID, so embeddedOutput()
     * will fail, because it looks only for modern "LVDS", "EDP", etc. If we
     * fail to detect which output is embedded, just use the one  with the lowest
     * ID. It's a wild guess, but I think it's highly probable that it will work.
     * See bug #318907 for further reference. -- dvratil
     */
    if (!embedded) {
        QList<int> keys = connectedOutputs.keys();
        std::sort(keys.begin(), keys.end());
        embedded = connectedOutputs.value(keys.first());
    }
    connectedOutputs.remove(embedded->id());

    if (connectedOutputs.isEmpty() || embedded->modes().isEmpty()) {
        qCWarning(KSCREEN_KDED) << "No external outputs found, going for singleOutput()";
        connectedOutputs.insert(embedded->id(), embedded);
        return singleOutput(connectedOutputs);
    }

    if (isLidClosed() && connectedOutputs.count() == 1) {
        qCDebug(KSCREEN_KDED) << "With lid closed";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        KScreen::OutputPtr external = connectedOutputs.value(connectedOutputs.keys().first());
        if (external->modes().isEmpty()) {
            return;
        }
        external->setEnabled(true);
        external->setPrimary(true);
        const KScreen::ModePtr bestMode = bestModeForOutput(external);
        Q_ASSERT(bestMode);
        external->setCurrentModeId(bestMode->id());
        external->setPos(QPoint(0, 0));

        return;
    }

    if (isLidClosed() && connectedOutputs.count() > 1) {
        qCDebug(KSCREEN_KDED) << "Lid is closed, and more than one output";
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        extendToRight(connectedOutputs);
        return;
    }

    qCDebug(KSCREEN_KDED) << "Lid is open";
    //If lid is open, laptop screen should be primary
    embedded->setPos(QPoint(0,0));
    embedded->setPrimary(true);
    embedded->setEnabled(true);
    const KScreen::ModePtr embeddedMode = bestModeForOutput(embedded);
    Q_ASSERT(embeddedMode);
    embedded->setCurrentModeId(embeddedMode->id());

    int globalWidth = embedded->geometry().width();
    KScreen::OutputPtr biggest = biggestOutput(connectedOutputs);
    Q_ASSERT(biggest);
    connectedOutputs.remove(biggest->id());

    biggest->setPos(QPoint(globalWidth, 0));
    biggest->setEnabled(true);
    biggest->setPrimary(false);
    const KScreen::ModePtr mode = bestModeForOutput(biggest);
    biggest->setCurrentModeId(mode->id());

    globalWidth += biggest->geometry().width();
    Q_FOREACH(KScreen::OutputPtr output, connectedOutputs) {
        output->setEnabled(true);
        output->setPrimary(false);
        output->setPos(QPoint(globalWidth, 0));
        const KScreen::ModePtr mode = bestModeForOutput(output);
        Q_ASSERT(mode);
        output->setCurrentModeId(mode->id());

        globalWidth += output->geometry().width();
    }

    if (isDocked()) {
        qCDebug(KSCREEN_KDED) << "Docked";
        embedded->setPrimary(false);
        biggest->setPrimary(true);
    }
}

void Generator::extendToRight(KScreen::OutputList &connectedOutputs)
{
    ASSERT_OUTPUTS(connectedOutputs);
    if (connectedOutputs.isEmpty()) {
        return;
    }

    qCDebug(KSCREEN_KDED) << "Extending to the right";
    KScreen::OutputPtr biggest = biggestOutput(connectedOutputs);
    Q_ASSERT(biggest);

    connectedOutputs.remove(biggest->id());

    biggest->setEnabled(true);
    biggest->setPrimary(true);
    biggest->setPos(QPoint(0,0));
    const KScreen::ModePtr mode = bestModeForOutput(biggest);
    Q_ASSERT(mode);
    biggest->setCurrentModeId(mode->id());

    int globalWidth = biggest->geometry().width();

    Q_FOREACH(KScreen::OutputPtr output, connectedOutputs) {
        output->setEnabled(true);
        output->setPrimary(false);
        output->setPos(QPoint(globalWidth, 0));
        const KScreen::ModePtr mode = bestModeForOutput(output);
        Q_ASSERT(mode);
        output->setCurrentModeId(mode->id());

        globalWidth += output->geometry().width();
    }
}

KScreen::ModePtr Generator::biggestMode(const KScreen::ModeList &modes)
{
    Q_ASSERT(!modes.isEmpty());

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

qreal Generator::bestScaleForOutput(const KScreen::OutputPtr &output) {
    //if we have no physical size, we can't determine the DPI properly. Fallback to scale 1
    if (output->sizeMm().height() <= 0) {
        return 1.0;
    }
    const auto mode = bestModeForOutput(output);
    const qreal dpi = mode->size().height() / (output->sizeMm().height() / 25.4);

    //if reported DPI is closer to two times normal DPI, followed by a sanity check of having the sort of vertical resolution
    //you'd find in a high res screen
    if (dpi > 96 * 1.5 && mode->size().height() >= 1440) {
        return 2.0;
    }
    return 1.0;
}

KScreen::ModePtr Generator::bestModeForOutput(const KScreen::OutputPtr &output)
{
    if (KScreen::ModePtr outputMode = output->preferredMode()) {
        return outputMode;
    }

    return biggestMode(output->modes());
}


KScreen::OutputPtr Generator::biggestOutput(const KScreen::OutputList &outputs)
{
    ASSERT_OUTPUTS(outputs)

    int area, total = 0;
    KScreen::OutputPtr biggest;
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        const KScreen::ModePtr mode = bestModeForOutput(output);
        if (!mode) {
            continue;
        }
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

bool Generator::isLaptop() const
{
    if (m_forceLaptop) {
        return true;
    }
    if (m_forceNotLaptop) {
        return false;
    }

    return Device::self()->isLaptop();
}

bool Generator::isLidClosed() const
{
    if (m_forceLidClosed) {
        return true;
    }
    if (m_forceNotLaptop) {
        return false;
    }

    return Device::self()->isLidClosed();
}

bool Generator::isDocked() const
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
