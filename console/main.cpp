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

    KApplication app;

    setenv("KSCREEN_BACKEND", "XRandR", 1);

    Loop *loop = new Loop(0);
    loop->printConfig();
    app.exec();
}
