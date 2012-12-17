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

bool Generator::forceLaptop = false;
bool Generator::forceLidClosed = false;
bool Generator::forceDocked = false;

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
    if (Generator::isLaptop()) {
        return Generator::laptop();
    }

    qDebug() << "No ideal config found";
    return new KScreen::Config();
}

bool Generator::isLaptop()
{
    if (Generator::forceLaptop) {
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
        if (Generator::isEmbedded(output->name())) {
            embedded = output;
            continue;
        }
        external = output;
    }

    if (Generator::isLidClosed()) {
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

    if (Generator::isDocked()) {
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
    if (Generator::forceLidClosed) {
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
    if (Generator::forceDocked) {
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