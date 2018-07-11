/*************************************************************************************
 *  Copyright 2014-2016 by Sebastian Kügler <sebas@kde.org>                          *
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

#include "osdtest.h"

#include <QGuiApplication>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QCommandLineOption dbus = QCommandLineOption(QStringList() << QStringLiteral("d") << QStringLiteral("dbus"),
                                                  QStringLiteral("Call over dbus"));
    QCommandLineOption outputid = QCommandLineOption(QStringList() << QStringLiteral("o") << QStringLiteral("outputidentifiers"),
                                                  QStringLiteral("Show output identifier"));
    QCommandLineOption icon = QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("icon"),
                                                  QStringLiteral("Icon to use for OSD"), QStringLiteral("preferences-desktop-display-randr"));
    QCommandLineOption message = QCommandLineOption(QStringList() << QStringLiteral("m") << QStringLiteral("message"),
                                                  QStringLiteral("Icon to use for OSD"), QStringLiteral("OSD Test"));
    QCommandLineOption selector = QCommandLineOption({ QStringLiteral("s"), QStringLiteral("selector") },
                                                  QStringLiteral("Show new screen action selector"));
    KScreen::OsdTest osdtest;
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(dbus);
    parser.addOption(outputid);
    parser.addOption(icon);
    parser.addOption(message);
    parser.addOption(selector);
    parser.process(app);



    if (parser.isSet(dbus)) {
        osdtest.setUseDBus(true);
    }
    if (parser.isSet(outputid)) {
        osdtest.showOutputIdentifiers();
    } else if (parser.isSet(selector)) {
        osdtest.showActionSelector();
    } else {
        osdtest.showGenericOsd(parser.value(icon), parser.value(message));
    }
    if (parser.isSet(outputid)) {
    }

    return app.exec();
}
