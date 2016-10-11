/*************************************************************************************
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>                                  *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#include "osdtest.h"
#include "../../kded/osd.h"
#include "../../kded/osdmanager.h"
#include "../../kcm/src/utils.h"

#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QTimer>
#include <QRect>

#include <QLoggingCategory>


Q_LOGGING_CATEGORY(KSCREEN_KDED, "kscreen.kded")

namespace KScreen {
OsdTest::OsdTest(QObject *parent)
    : QObject(parent)
{
}

OsdTest::~OsdTest()
{
}

void OsdTest::start()
{
    QTimer::singleShot(5500, qApp, &QCoreApplication::quit);
    auto osdManager = new KScreen::OsdManager(this);
    osdManager->showOutputIdentifiers();
}


} // ns