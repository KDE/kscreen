/*
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QGuiApplication>

#include <KLocalizedString>

#include "osdmanager.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setQuitOnLastWindowClosed(false);
    KLocalizedString::setApplicationDomain("kscreen_osd");
    KScreen::OsdManager osdManager;
    return app.exec();
}
