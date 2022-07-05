/*
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QGuiApplication>

#include "osdmanager.h"

int main(int argc, char **argv)
{
    KScreen::OsdManager osdManager;
    return QGuiApplication(argc, argv).exec();
}
