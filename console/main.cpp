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
#include <QDateTime>
#include <KAboutData>
#include <KLocalizedString>

#include <kscreen/getconfigoperation.h>
#include <kscreen/config.h>

#include "console.h"

using namespace std;

void configReceived(KScreen::ConfigOperation *op)
{

    const KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();

    const QString command = op->property("command").toString();
    const qint64 msecs = QDateTime::currentMSecsSinceEpoch() - op->property("start").toLongLong();
    qDebug() << "Received config. Took" << msecs << "milliseconds";

    Console *console = new Console(config);

    if (command.isEmpty()) {
        console->printConfig();
        console->monitorAndPrint();
    } else if (command == QLatin1String("monitor")) {
        QTextStream(stdout) << "Remember to enable KSRandR or KSRandR11 in kdebugdialog" << endl;
        //Print config so that we have some pivot data
        console->printConfig();
        console->monitor();
        //Do nothing, enable backend output to see debug
    } else if (command == QLatin1String("outputs")) {
        console->printConfig();
        qApp->quit();
    } else if (command == QLatin1String("config")) {
        console->printSerializations();
        qApp->quit();
    } else if (command == QLatin1String("bug")) {
        QTextStream(stdout) << QStringLiteral("\n========================xrandr --verbose==========================\n");
        QProcess proc;
        proc.setProcessChannelMode(QProcess::MergedChannels);
        proc.start(QStringLiteral("xrandr"), QStringList(QStringLiteral("--verbose")));
        proc.waitForFinished();
        QTextStream(stdout) << proc.readAll().data();
        QTextStream(stdout) << QStringLiteral("\n========================Outputs===================================\n");
        console->printConfig();
        QTextStream(stdout) << QStringLiteral("\n========================Configurations============================\n");
        console->printSerializations();
        qApp->quit();
    } else if (command == QLatin1String("json")) {
        console->printJSONConfig();
        qApp->quit();
    } else {

        qApp->quit();
    }
}


int main (int argc, char *argv[])
{
    dup2(1, 2);

    QGuiApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("kscreen-console"), i18n("KScreen Console"), QStringLiteral("1.0"), i18n("KScreen Console"),
    KAboutLicense::GPL, i18n("(c) 2012 KScreen Team"));
    KAboutData::setApplicationData(aboutData);

    aboutData.addAuthor(i18n("Alejandro Fiestas Olivares"), i18n("Maintainer"), QStringLiteral("afiestas@kde.org"),
        QStringLiteral("http://www.afiestas.org/"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        i18n("KScreen Console is a CLI tool to query KScreen status\n\n"
             "Commands:\n"
             "  bug             Show information needed for a bug report\n"
             "  config          Show KScreen config files\n"
             "  outputs         Show output information\n"
             "  monitor         Monitor for changes\n"
             "  json            Show current KScreen config"));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("command"), i18n("Command to execute"),
                                 QStringLiteral("bug|config|outputs|monitor|json"));
    parser.addPositionalArgument(QStringLiteral("[args...]"), i18n("Arguments for the specified command"));

    parser.process(app);

    QString command;
    if (!parser.positionalArguments().isEmpty()) {
        command = parser.positionalArguments().first();
    }

    qDebug() << "START: Requesting Config";

    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation();
    op->setProperty("command", command);
    op->setProperty("start", QDateTime::currentMSecsSinceEpoch());
    QObject::connect(op, &KScreen::GetConfigOperation::finished,
                     [&](KScreen::ConfigOperation *op) {
                          configReceived(op);
                      });

    app.exec();
}
