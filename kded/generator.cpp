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

#include <QtCore/QDebug>

#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusConnection>

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
 , m_device(new Device(this))
 , m_isReady(false)
 , m_forceLaptop(false)
 , m_forceLidClosed(false)
 , m_forceDocked(false)
{
    connect(m_device, SIGNAL(ready()), SIGNAL(ready()));
}

void Generator::destroy()
{
    delete Generator::instance;
}

Generator::~Generator()
{
    delete m_device;
}

KScreen::Config* Generator::idealConfig()
{
    KScreen::Config* config = KScreen::Config::current();
    disableAllDisconnectedOutputs(config->outputs());

    KScreen::OutputList outputs = config->connectedOutputs();

    if (outputs.count() == 1) {
        singleOutput(outputs);
        return config;
    }

    if (isLaptop()) {
        laptop(outputs);
        return config;
    }

    extendToRight(outputs);

    return config;
}

KScreen::Config* Generator::displaySwitch(int iteration)
{
    KScreen::Config* config = KScreen::Config::current();
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

    //Clone
    if (iteration == 1) {
        KScreen::ModeList modes = embedded->modes();
        QMap<int, QSize> embeddedModeSize;
        Q_FOREACH(KScreen::Mode* mode, modes) {
            embeddedModeSize.insert(mode->id(), mode->size());
        }

        QList<int> embeddedKeys;
        KScreen::ModeList externalCommon;
        KScreen::ModeList externalModes = external->modes();
        Q_FOREACH(KScreen::Mode* mode, externalModes) {
            if (!embeddedModeSize.keys(mode->size()).isEmpty()) {
                externalCommon.insert(mode->id(), mode);
                embeddedKeys.append(embeddedModeSize.keys(mode->size()));
            }
        }

        KScreen::ModeList embeddedCommon;
        Q_FOREACH(int key, embeddedKeys) {
            embeddedCommon.insert(key, modes[key]);
        }

        KScreen::Mode* biggestEmbedded = biggestMode(embeddedCommon);
        KScreen::Mode* biggestExternal = biggestMode(externalCommon);

        embedded->setEnabled(true);
        embedded->setPos(QPoint(0,0));
        embedded->setCurrentMode(biggestEmbedded->id());
        external->setEnabled(true);
        external->setPos(QPoint(0,0));
        external->setCurrentMode(biggestExternal->id());

        return config;
    }

    //Extend left
    if (iteration == 2) {
        external->setEnabled(true);
        external->setCurrentMode(external->preferredMode());

        QSize size = external->mode(external->currentMode())->size();
        embedded->setPos(QPoint(size.width(), 0));
        embedded->setEnabled(true);
        embedded->setCurrentMode(embedded->preferredMode());
        embedded->setPrimary(true);
        return config;
    }

    //Turn of embedded
    if (iteration == 3) {
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        external->setEnabled(true);
        external->setPrimary(true);
        external->setCurrentMode(external->preferredMode());
        return config;
    }

    //Turn off external
    if (iteration == 4) {
        embedded->setEnabled(true);
        embedded->setPrimary(true);
        embedded->setCurrentMode(embedded->preferredMode());

        external->setEnabled(false);
        external->setPrimary(false);
        return config;
    }

    //Extend right
    if (iteration == 5) {
        embedded->setPos(QPoint(0,0));
        embedded->setCurrentMode(embedded->preferredMode());
        embedded->setPrimary(true);
        embedded->setEnabled(true);

        QSize size = embedded->mode(embedded->currentMode())->size();
        external->setPos(QPoint(size.width(), 0));
        external->setEnabled(true);
        external->setCurrentMode(external->preferredMode());
        external->setPrimary(false);

        return config;
    }

    return config;
}

void Generator::singleOutput(KScreen::OutputList& outputs)
{
    qDebug() << "Config for one output";
    KScreen::Output* output = outputs.take(outputs.keys().first());
    output->setCurrentMode(output->preferredMode());
}

void Generator::laptop(KScreen::OutputList& outputs)
{
    qDebug() << "Config for a laptop";

    KScreen::Output* embedded = embeddedOutput(outputs);
    outputs.remove(embedded->id());

    if (outputs.isEmpty() || !embedded) {
        qWarning("Neither external outputs or embedded could be found");
        return;
    }

    if (isLidClosed() && outputs.count() == 1) {
        qDebug() << "With lid closed";
        embedded->setEnabled(false);

        KScreen::Output* external = outputs.value(outputs.keys().first());
        external->setEnabled(true);
        external->setCurrentMode(external->preferredMode());
        external->setPrimary(true);

        return;
    }

    if (isLidClosed() && outputs.count() > 1) {
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        extendToRight(outputs);
        return;
    }

    //If lid is open, laptop screen shuold be primary
    embedded->setPos(QPoint(0,0));
    embedded->setCurrentMode(embedded->preferredMode());
    embedded->setPrimary(true);
    embedded->setEnabled(true);

    QSize globalSize = embedded->mode(embedded->preferredMode())->size();
    KScreen::Output* biggest = biggestOutput(outputs);
    outputs.remove(biggest->id());

    biggest->setPos(QPoint(globalSize.width(), 0));
    biggest->setEnabled(true);
    biggest->setCurrentMode(biggest->preferredMode());
    biggest->setPrimary(false);

    QSize size;
    globalSize += biggest->mode(biggest->currentMode())->size();
    Q_FOREACH(KScreen::Output* output, outputs) {
        output->setEnabled(true);
        output->setCurrentMode(output->preferredMode());
        output->setPos(QPoint(globalSize.width(), 0));

        size = output->mode(output->currentMode())->size();
        globalSize += size;
    }

    if (isDocked()) {
        qDebug() << "Docked";
        embedded->setPrimary(false);
        biggest->setPrimary(true);
    }

    return;
}

void Generator::extendToRight(KScreen::OutputList& outputs)
{
    KScreen::Output* biggest = biggestOutput(outputs);
    outputs.remove(biggest->id());

    biggest->setEnabled(true);
    biggest->setPrimary(true);
    biggest->setCurrentMode(biggest->preferredMode());
    biggest->setPos(QPoint(0,0));

    QSize size;
    QSize globalSize = biggest->mode(biggest->currentMode())->size();
    Q_FOREACH(KScreen::Output* output, outputs) {
        output->setEnabled(true);
        output->setPrimary(false);
        output->setCurrentMode(output->preferredMode());
        output->setPos(QPoint(globalSize.width(), 0));

        size = output->mode(output->currentMode())->size();
        globalSize += size;
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
        KScreen::Mode* mode = output->mode(output->preferredMode());
        area = mode->size().width() * mode->size().height();
        if (area < total) {
            continue;
        }

        total = area;
        biggest = output;
    }

    return biggest;
}

void Generator::disableAllDisconnectedOutputs(const KScreen::OutputList& outputs)
{
    Q_FOREACH(KScreen::Output* output, outputs) {
        if (!output->isConnected()) {
            output->setEnabled(false);
        }
    }
}

KScreen::Output* Generator::embeddedOutput(const KScreen::OutputList& outputs)
{
    Q_FOREACH(KScreen::Output* output, outputs) {
        if (!isEmbedded(output->name())) {
            continue;
        }

        return output;
    }

    return 0;
}

bool Generator::isEmbedded(const QString& name)
{
    QStringList embedded;
    embedded << "LVDS";
    embedded << "IDP";
    embedded << "EDP";

    Q_FOREACH(const QString &pre, embedded) {
        if (name.toUpper().startsWith(pre)) {
            qDebug() << "This is embedded: " << name;
            return true;
        }
    }

    return false;
}

bool Generator::isLaptop()
{
    if (m_forceLaptop) {
        return true;
    }

    return m_device->isLaptop();
}

bool Generator::isLidClosed()
{
    if (m_forceLidClosed) {
        return true;
    }

    return m_device->isLidClosed();
}

bool Generator::isDocked()
{
    if (m_forceDocked) {
        return true;
    }

    return m_device->isDocked();
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
