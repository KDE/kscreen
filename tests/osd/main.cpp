/*
    SPDX-FileCopyrightText: 2014-2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osdtest.h"

#include <QCommandLineParser>
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QCommandLineOption dbus = QCommandLineOption(QStringList() << QStringLiteral("d") << QStringLiteral("dbus"), QStringLiteral("Call over dbus"));
    QCommandLineOption outputid =
        QCommandLineOption(QStringList() << QStringLiteral("o") << QStringLiteral("outputidentifiers"), QStringLiteral("Show output identifier"));
    QCommandLineOption icon = QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("icon"),
                                                 QStringLiteral("Icon to use for OSD"),
                                                 QStringLiteral("preferences-desktop-display-randr"));
    QCommandLineOption message = QCommandLineOption(QStringList() << QStringLiteral("m") << QStringLiteral("message"),
                                                    QStringLiteral("Icon to use for OSD"),
                                                    QStringLiteral("OSD Test"));
    QCommandLineOption selector = QCommandLineOption({QStringLiteral("s"), QStringLiteral("selector")}, QStringLiteral("Show new screen action selector"));
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
    if (parser.isSet(outputid)) { }

    return app.exec();
}
