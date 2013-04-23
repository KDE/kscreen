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

#include <unistd.h>

#include <QtCore/QDebug>
#include <QtCore/QProcess>

#include <KApplication>
#include <KAboutData>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KAboutData>

#include "console.h"

using namespace std;

void showCommands()
{
    qDebug() << "Commands: " << endl;
    qDebug() << "    bug \t <Show information needed for a bug report>" << endl;
    qDebug() << "    config \t <Show kscreen config files>" << endl;
    qDebug() << "    outputs \t <Show Output information>" << endl;
    qDebug() << "    monitor \t <Monitors for changes>" << endl;
}
int main (int argc, char *argv[])
{
    dup2(1, 2);

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
        showCommands();
        return 1;
    }
    KApplication app;

    Console *console = new Console(0);

    QString command;
    if (args->count() > 0) {
        command = args->arg(0);
    }

    if (command.isEmpty()) {
        console->printConfig();
        console->monitorAndPrint();
        return app.exec();
    }

    if (command == "monitor") {
        qDebug() << "Remember to enable KSRandR or KSRandR11 in kdebugdialog";
        //Print config so that we have some pivot data
        console->printConfig();
        //Do nothing, enable backend output to see debug
        return app.exec();
    }

    if (command == "outputs") {
        console->printConfig();
        return 1;
    }

    if (command == "config") {
        console->printSerializations();
        return 1;
    }
    if (command == "bug") {
        qDebug() << endl << "========================xrandr --verbose==========================" << endl;
        QProcess proc;
        proc.setProcessChannelMode(QProcess::MergedChannels);
        proc.start("xrandr", QStringList("--verbose"));
        proc.waitForFinished();
        qDebug() << proc.readAll().data();
        qDebug() << endl << "========================Outputs===================================" << endl;
        console->printConfig();
        qDebug() << endl << "========================Configurations============================" << endl;
        console->printSerializations();
        return 1;
    }
    showCommands();
    return -1;
}
