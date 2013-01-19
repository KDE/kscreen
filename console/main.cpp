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

#include <stdlib.h>
#include <iostream>

#include <KApplication>
#include <KAboutData>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KAboutData>

#include "loop.h"

using namespace std;

int main (int argc, char *argv[])
{
    KAboutData aboutData("kscreen-console", "kscreen-console", ki18n("KScreen Console"), "1.0", ki18n("KScreen Console"),
    KAboutData::License_GPL, ki18n("(c) 2012 KScreen Team"));

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Maintainer"), "afiestas@kde.org",
        "http://www.afiestas.org/");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("commands", ki18n("Show available commands"));
    options.add("+[arg(s)]", ki18n("Arguments for command"));

    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("commands")) {
        cout << "Commands: " << endl;
        cout << "    bug \t <Show information needed for a bug report>" << endl;
        cout << "    config \t <Show kscreen config files>" << endl;
        cout << "    outputs \t <Show Output information>" << endl;
        cout << "    monitor \t <Monitors for changes>" << endl;
        return 1;
    }
    KApplication app;

    setenv("KSCREEN_BACKEND", "XRandR", 1);

    Loop *loop = new Loop(0);

    QString command;
    if (args->count() > 0) {
        command = args->arg(0);
    }

    if (command.isEmpty()) {
        loop->printConfig();
        loop->monitorAndPrint();
        return app.exec();
    }

    if (command == "outputs") {
        loop->printConfig();
        return 1;
    }

    return -1;
}
