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

#include "../kded/generator.h"

#include <QtTest>
#include <QtCore/QObject>

#include <kscreen/config.h>

using namespace KScreen;

class testScreenConfig : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void singleOutput();
    void laptopLidOpenAndExternal();
    void laptopLidClosedAndExternal();
    void laptopDockedLidOpenAndExternal();
    void laptopDockedLidClosedAndExternal();
    void workstationWithTwoOutputsSameSize();
};

void testScreenConfig::initTestCase()
{
    setenv("KSCREEN_BACKEND", "Fake", 1);
}

void testScreenConfig::singleOutput()
{
    //json file for the fake backend
    QByteArray path(TEST_DATA);
    path.append("/singleOutput.json");
    setenv("TEST_DATA", path, 1);

    Output* output = Generator::idealConfig()->outputs().value(1);

    QCOMPARE(output->currentMode(), 3);

}

void testScreenConfig::laptopLidOpenAndExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopAndExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator::forceLaptop = true;
    Output* laptop = Generator::idealConfig()->outputs().value(1);
    Output* external = Generator::idealConfig()->outputs().value(2);

    QCOMPARE(laptop->currentMode(), 3);
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));

    QCOMPARE(external->currentMode(), 4);
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

void testScreenConfig::laptopLidClosedAndExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopAndExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator::forceLaptop = true;
    Generator::forceLidClosed = true;
    Output* laptop = Generator::idealConfig()->outputs().value(1);
    Output* external = Generator::idealConfig()->outputs().value(2);

    QCOMPARE(laptop->isEnabled(), false);

    QCOMPARE(external->currentMode(), 4);
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void testScreenConfig::laptopDockedLidOpenAndExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopAndExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator::forceLaptop = true;
    Generator::forceLidClosed = false;
    Generator::forceDocked = true;
    Output* laptop = Generator::idealConfig()->outputs().value(1);
    Output* external = Generator::idealConfig()->outputs().value(2);

    QCOMPARE(laptop->currentMode(), 3);
    QCOMPARE(laptop->isPrimary(), false);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));

    QCOMPARE(external->currentMode(), 4);
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

void testScreenConfig::laptopDockedLidClosedAndExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopAndExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator::forceLaptop = true;
    Generator::forceLidClosed = true;
    Generator::forceDocked = true;
    Output* laptop = Generator::idealConfig()->outputs().value(1);
    Output* external = Generator::idealConfig()->outputs().value(2);

    QCOMPARE(laptop->isEnabled(), false);

    QCOMPARE(external->currentMode(), 4);
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void testScreenConfig::workstationWithTwoOutputsSameSize()
{
    QByteArray path(TEST_DATA);
    path.append("/workstaionTwoExternalSameSize.json");
    setenv("TEST_DATA", path, 1);

    Generator::forceLaptop = false;

    Output* external1 = Generator::idealConfig()->outputs().value(1);
    Output* external2 = Generator::idealConfig()->outputs().value(2);

    QCOMPARE(external1->currentMode(), 3);
    QCOMPARE(external1->isEnabled(), true);

    QCOMPARE(external2->currentMode(), 3);
    QCOMPARE(external2->isEnabled(), true);
}
QTEST_MAIN(testScreenConfig)

#include "testscreenconfig.moc"