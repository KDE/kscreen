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
    KScreen::OsdTest osdtest;
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(dbus);
    parser.process(app);

    if (parser.isSet(dbus)) {
        osdtest.setUseDBus(true);
    }

    osdtest.showActionSelector();

    return app.exec();
}
