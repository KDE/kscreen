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
 , m_forceLaptop(false)
 , m_forceLidClosed(false)
 , m_forceDocked(false)
{

}

void Generator::destroy()
{
    delete Generator::instance;
}

Generator::~Generator()
{

}

KScreen::Config* Generator::idealConfig()
{
    KScreen::Config* config = KScreen::Config::current();
    KScreen::OutputList outputs = config->outputs();
    KScreen::OutputList connectedOutputs;

    Q_FOREACH(KScreen::Output* output, outputs) {
        if (!output->isConnected()) {
            output->setEnabled(false);
            continue;
        }

        connectedOutputs.insert(output->id(), output);
    }

    //If we only have one screen, just select the preferred mode
    if (connectedOutputs.count() == 1) {
        qDebug() << "Config for one output";
        KScreen::Output* output = connectedOutputs.take(connectedOutputs.keys().first());
        output->setCurrentMode(output->preferredMode());

        return config;
    }

    //If we are a laptop, go into laptop mode
    if (isLaptop()) {
        return laptop();
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

    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.UPower",
                                   "/org/freedesktop/UPower",
                                   "org.freedesktop.DBus.Properties",
                                   "Get");
    QVariantList args;
    args << "org.freedesktop.UPower";
    args << "LidIsPresent";
    msg.setArguments(args);

    QDBusReply<QVariant> reply = QDBusConnection::systemBus().call(msg);
    return reply.value().toBool();
}

KScreen::Config* Generator::laptop()
{
    qDebug() << "Config for a laptop";
    KScreen::Config* config = KScreen::Config::current();
    KScreen::OutputList outputs = config->outputs();

    KScreen::Output* embedded;
    KScreen::Output* external;
    Q_FOREACH(KScreen::Output* output, outputs) {
        if (!output->isConnected()) {
            continue;
        }
        if (isEmbedded(output->name())) {
            embedded = output;
            continue;
        }
        external = output;
    }

    if (isLidClosed()) {
        qDebug() << "With lid closed";
        embedded->setEnabled(false);
        external->setEnabled(true);
        external->setCurrentMode(external->preferredMode());
        external->setPrimary(true);

        return config;
    }

    embedded->setPos(QPoint(0,0));
    embedded->setCurrentMode(embedded->preferredMode());
    embedded->setPrimary(true);
    embedded->setEnabled(true);

    QSize size = embedded->mode(embedded->preferredMode())->size();
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

    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.UPower",
                                                      "/org/freedesktop/UPower",
                                                      "org.freedesktop.DBus.Properties",
                                                      "Get");
    QVariantList args;
    args << "org.freedesktop.UPower";
    args << "LidIsClosed";
    msg.setArguments(args);

    QDBusReply<QVariant> reply = QDBusConnection::systemBus().call(msg);
    return reply.value().toBool();
}

bool Generator::isDocked()
{
    if (m_forceDocked) {
        return true;
    }

    return false;
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
