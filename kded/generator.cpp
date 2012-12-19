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

    KScreen::OutputList connectedOutputs = config->connectedOutputs();

    if (connectedOutputs.count() == 1) {
        qDebug() << "Config for one output";
        KScreen::Output* output = connectedOutputs.take(connectedOutputs.keys().first());
        output->setCurrentMode(output->preferredMode());

        return config;
    }

    //If we are a laptop, go into laptop mode
    if (isLaptop()) {
        return laptop(config, connectedOutputs);
    }

    //Check if the prefered mode has the same size in all
    bool sameSize = false;
    QSize last;
    Q_FOREACH(KScreen::Output* output, connectedOutputs) {
        if (last.isEmpty()) {
            last = output->mode(output->preferredMode())->size();
            continue;
        }
        if (last != output->mode(output->preferredMode())->size()) {
            break;
        }
        sameSize = true;
    }

    if (sameSize) {
        Q_FOREACH(KScreen::Output* output, connectedOutputs) {
            output->setCurrentMode(output->preferredMode());
            output->setEnabled(true);
            output->setPos(QPoint(0,0));
        }

        return config;
    }

    qDebug() << "No ideal config found";
    return new KScreen::Config();
}

bool Generator::isLaptop()
{
    if (m_forceLaptop) {
        return true;
    }

    return m_device->isLaptop();
}

KScreen::Config* Generator::laptop(KScreen::Config* config, KScreen::OutputList& outputs)
{
    qDebug() << "Config for a laptop";

    KScreen::Output* embedded = 0;
    Q_FOREACH(KScreen::Output* output, outputs) {
        if (isEmbedded(output->name())) {
            embedded = output;
            outputs.remove(embedded->id());
            continue;
        }
    }

    if (outputs.isEmpty() || !embedded) {
        qWarning("Neither external outputs or embedded could be found");
        return KScreen::Config::current();
    }

    if (isLidClosed() && outputs.count() == 1) {
        qDebug() << "With lid closed";
        embedded->setEnabled(false);

        KScreen::Output* external = outputs.value(outputs.keys().first());
        external->setEnabled(true);
        external->setCurrentMode(external->preferredMode());
        external->setPrimary(true);

        return config;
    }

    if (isLidClosed() && outputs.count() > 1) {
        embedded->setEnabled(false);
        embedded->setPrimary(false);

        KScreen::Output* biggest = biggestOutput(outputs);
        outputs.remove(biggest->id());

        biggest->setEnabled(true);
        biggest->setPrimary(true);
        biggest->setCurrentMode(biggest->preferredMode());
        biggest->setPos(QPoint(0,0));
        QSize globalSize = biggest->mode(biggest->currentMode())->size();

        Q_FOREACH(KScreen::Output* output, outputs) {
            output->setEnabled(true);
            output->setCurrentMode(output->preferredMode());
            output->setPos(QPoint(globalSize.width(), 0));

            QSize size = output->mode(output->currentMode())->size();
            globalSize += size;
        }

        return config;
    }

    embedded->setPos(QPoint(0,0));
    embedded->setCurrentMode(embedded->preferredMode());
    embedded->setPrimary(true);
    embedded->setEnabled(true);

    QSize size = embedded->mode(embedded->preferredMode())->size();
    KScreen::Output* external = outputs.value(outputs.keys().first());
    external->setPos(QPoint(size.width(), 0));
    external->setEnabled(true);
    external->setCurrentMode(external->preferredMode());
    external->setPrimary(false);

    if (isDocked()) {
        qDebug() << "Docked";
        embedded->setPrimary(false);
        external->setPrimary(true);
    }

    return config;
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

KScreen::Config* Generator::dockedLaptop()
{
    return new KScreen::Config();
}

KScreen::Config* Generator::desktop()
{
    return new KScreen::Config();
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
