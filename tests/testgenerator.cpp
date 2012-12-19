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
    void laptopLidOpenAndTwoExternal();
    void laptopLidClosedAndExternal();
    void laptopLidClosedAndThreeExternal();
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

    Output* output = Generator::self()->idealConfig()->outputs().value(1);

    QCOMPARE(output->currentMode(), 3);

}

void testScreenConfig::laptopLidOpenAndExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopAndExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);

    QCOMPARE(laptop->currentMode(), 3);
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));

    QCOMPARE(external->currentMode(), 4);
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

void testScreenConfig::laptopLidOpenAndTwoExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopLidOpenAndTwoExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* hdmi1 = config->outputs().value(2);
    Output* hdmi2 = config->outputs().value(3);

    QCOMPARE(laptop->currentMode(), 3);
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));

    QCOMPARE(hdmi1->currentMode(), 4);
    QCOMPARE(hdmi1->isPrimary(), false);
    QCOMPARE(hdmi1->isEnabled(), true);
    QCOMPARE(hdmi1->pos(), QPoint(hdmi2->pos().x() + hdmi2->mode(hdmi2->currentMode())->size().width(), 0));

    QCOMPARE(hdmi2->currentMode(), 4);
    QCOMPARE(hdmi2->isPrimary(), false);
    QCOMPARE(hdmi2->isEnabled(), true);
    QCOMPARE(hdmi2->pos(), QPoint(1280, 0));

}

void testScreenConfig::laptopLidClosedAndExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopAndExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);
    generator->setForceLidClosed(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);

    QCOMPARE(laptop->isEnabled(), false);

    QCOMPARE(external->currentMode(), 4);
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void testScreenConfig::laptopLidClosedAndThreeExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopLidClosedAndThreeExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);
    generator->setForceLidClosed(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* hdmi1 = config->outputs().value(2);
    Output* hdmi2 = config->outputs().value(3);
    Output* primary = config->outputs().value(4);

    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(laptop->isPrimary(), false);

    QCOMPARE(hdmi1->isEnabled(), true);
    QCOMPARE(hdmi1->isPrimary(), false);
    QCOMPARE(hdmi1->currentMode(), 4);
    QCOMPARE(hdmi1->pos(), QPoint(primary->mode(primary->currentMode())->size().width(), 0));

    QCOMPARE(hdmi2->isEnabled(), true);
    QCOMPARE(hdmi2->isPrimary(), false);
    QCOMPARE(hdmi2->currentMode(), 3);
    QCOMPARE(hdmi2->pos(), QPoint(hdmi1->pos().x() + hdmi1->mode(hdmi1->currentMode())->size().width(), 0));

    QCOMPARE(primary->isEnabled(), true);
    QCOMPARE(primary->isPrimary(), true);
    QCOMPARE(primary->currentMode(), 4);
    QCOMPARE(primary->pos(), QPoint(0,0));
}

void testScreenConfig::laptopDockedLidOpenAndExternal()
{
    QByteArray path(TEST_DATA);
    path.append("/laptopAndExternal.json");
    setenv("TEST_DATA", path, 1);

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);
    generator->setForceLidClosed(false);
    generator->setForceDocked(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);

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

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);
    generator->setForceLidClosed(true);
    generator->setForceDocked(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);

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

    Generator* generator = Generator::self();
    generator->setForceLaptop(false);

    Config* config = generator->idealConfig();
    Output* external1 = config->outputs().value(1);
    Output* external2 = config->outputs().value(2);

    QCOMPARE(external1->currentMode(), 3);
    QCOMPARE(external1->isEnabled(), true);

    QCOMPARE(external2->currentMode(), 3);
    QCOMPARE(external2->isEnabled(), true);
}
QTEST_MAIN(testScreenConfig)

#include "testscreenconfig.moc"