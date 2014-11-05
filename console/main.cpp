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

#include <QProcess>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <KAboutData>
#include <KLocalizedString>

#include "console.h"

using namespace std;

void showCommands()
{
    QTextStream(stdout) << "Commands: " << endl;
    QTextStream(stdout) << "    bug \t <Show information needed for a bug report>" << endl;
    QTextStream(stdout) << "    config \t <Show kscreen config files>" << endl;
    QTextStream(stdout) << "    outputs \t <Show Output information>" << endl;
    QTextStream(stdout) << "    monitor \t <Monitors for changes>" << endl;
}
int main (int argc, char *argv[])
{
    dup2(1, 2);

    QGuiApplication app(argc, argv);
    KAboutData aboutData("kscreen-console", i18n("KScreen Console"), "1.0", i18n("KScreen Console"),
    KAboutLicense::GPL, i18n("(c) 2012 KScreen Team"));
    KAboutData::setApplicationData(aboutData);

    aboutData.addAuthor(i18n("Alejandro Fiestas Olivares"), i18n("Maintainer"), "afiestas@kde.org",
        "http://www.afiestas.org/");

    QCommandLineParser parser;
    parser.addPositionalArgument("command", i18n("Command to execute (commands will list the available commands)"));
    parser.addPositionalArgument("[args...]", i18n("Arguments for the specified command"));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.isSet("commands")) {
        showCommands();
        return 1;
    }

    Console *console = new Console(0);

    QString command;
    if (!parser.positionalArguments().isEmpty()) {
        command = parser.positionalArguments().first();
    }

    if (command.isEmpty()) {
        console->printConfig();
        console->monitorAndPrint();
        return app.exec();
    }

    if (command == "monitor") {
        QTextStream(stdout) << "Remember to enable KSRandR or KSRandR11 in kdebugdialog" << endl;
        //Print config so that we have some pivot data
        console->printConfig();
        console->monitor();
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
        QTextStream(stdout) << QStringLiteral("\n========================xrandr --verbose==========================\n");
        QProcess proc;
        proc.setProcessChannelMode(QProcess::MergedChannels);
        proc.start("xrandr", QStringList("--verbose"));
        proc.waitForFinished();
        QTextStream(stdout) << proc.readAll().data();
        QTextStream(stdout) << QStringLiteral("\n========================Outputs===================================\n");
        console->printConfig();
        QTextStream(stdout) << QStringLiteral("\n========================Configurations============================\n");
        console->printSerializations();
        return 1;
    }
    showCommands();
    return -1;
}
