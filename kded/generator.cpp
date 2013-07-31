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

Generator* Generator::self()
{
    if (!Generator::instance) {
        Generator::instance = new Generator();
    }
    return Generator::instance;
}

Generator::Generator()
 : QObject()
 , m_isReady(false)
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

    extendToRight(outputs);

    return fallbackIfNeeded(config);
}

KScreen::Config* Generator::fallbackIfNeeded(KScreen::Config* config)
{
    //If the ideal config can't be applied, try clonning
    if (!KScreen::Config::canBeApplied(config)) {
        delete config;
        config = displaySwitch(1);// Try to clone at our best
    }

    //If after trying to clone at our best, we fail... return current
    if (!KScreen::Config::canBeApplied(config)) {
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

    KScreen::Output* embedded, *external;
    embedded = embeddedOutput(outputs);
    outputs.remove(embedded->id());
    external = outputs.value(outputs.keys().first());

    if (iteration == 1) {
        kDebug() << "Cloning";
        KScreen::ModeList modes = embedded->modes();
        QMap<QString, QSize> embeddedModeSize;
        Q_FOREACH(KScreen::Mode* mode, modes) {
            embeddedModeSize.insert(mode->id(), mode->size());
        }

        QList<QString> embeddedKeys;
        KScreen::ModeList externalCommon;
        KScreen::ModeList externalModes = external->modes();
        Q_FOREACH(KScreen::Mode* mode, externalModes) {
            if (!embeddedModeSize.keys(mode->size()).isEmpty()) {
                externalCommon.insert(mode->id(), mode);
                embeddedKeys.append(embeddedModeSize.keys(mode->size()));
            }
        }

        KScreen::ModeList embeddedCommon;
        Q_FOREACH(const QString& key, embeddedKeys) {
            embeddedCommon.insert(key, modes[key]);
        }

        KScreen::Mode* biggestEmbedded = biggestMode(embeddedCommon);
        KScreen::Mode* biggestExternal = biggestMode(externalCommon);

        embedded->setEnabled(true);
        embedded->setPos(QPoint(0,0));
        embedded->setCurrentModeId(biggestEmbedded->id());
        external->setEnabled(true);
        external->setPos(QPoint(0,0));
        external->setCurrentModeId(biggestExternal->id());

        return config;
    }

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
    int area, total = 0;
    KScreen::Mode* biggest = 0;
    Q_FOREACH(KScreen::Mode* mode, modes) {
        area = mode->size().width() * mode->size().height();
        if (area < total) {
            continue;
        }
        if (area == total && mode->refreshRate() < biggest->refreshRate()) {
            continue;
        }
        if (area == total && mode->refreshRate() > biggest->refreshRate()) {
            biggest = mode;
            continue;
        }

        total = area;
        biggest = mode;
    }

    return biggest;
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
