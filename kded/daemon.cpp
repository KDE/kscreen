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

#include "daemon.h"
#include "serializer.h"
#include "generator.h"

#include <kdemacros.h>
#include <KPluginFactory>

#include <kscreen/config.h>
#include <kscreen/configmonitor.h>

K_PLUGIN_FACTORY(KScreenDaemonFactory, registerPlugin<KScreenDaemon>();)
K_EXPORT_PLUGIN(KScreenDaemonFactory("kscreen", "kscreen"))

KScreenDaemon::KScreenDaemon(QObject* parent, const QList< QVariant >& ) : KDEDModule(parent)
{
    connect(Generator::self(), SIGNAL(ready()), SLOT(init()));
}

KScreenDaemon::~KScreenDaemon()
{
    Generator::destroy();
}

void KScreenDaemon::init()
{
    applyConfig();
    monitorForChanges();
}

void KScreenDaemon::applyConfig()
{
    KScreen::Config* config = 0;
    if (Serializer::configExists()) {
        config = Serializer::config(Serializer::currentId());
    } else {
        config = Generator::self()->idealConfig();
    }

    KScreen::Config::setConfig(config);
}

void KScreenDaemon::monitorForChanges()
{
    KScreen::Config* config = KScreen::Config::current();
    KScreen::ConfigMonitor::instance()->addConfig(config);

    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(KScreen::Output* output, outputs) {
        connect(output, SIGNAL(isConnectedChanged()), SLOT(applyConfig()));
    }
}