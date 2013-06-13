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

    private:
        void loadConfig(const QByteArray &fileName);

private Q_SLOTS:
    void initTestCase();
    void singleOutput();
    void laptopLidOpenAndExternal();
    void laptopLidOpenAndTwoExternal();
    void laptopLidClosedAndExternal();
    void laptopLidClosedAndThreeExternal();
    void laptopDockedLidOpenAndExternal();
    void laptopDockedLidClosedAndExternal();
    void workstationWithoutScreens();
    void workstationWithNoConnectedScreens();
    void workstationTwoExternalSameSize();
    void workstationTwoExternalDiferentSize();

    void switchDisplayTwoScreens();
};

void testScreenConfig::loadConfig(const QByteArray& fileName)
{
    QByteArray path(TEST_DATA);
    path.append("/" + fileName);
    setenv("TEST_DATA", path, 1);
}

void testScreenConfig::initTestCase()
{
    setenv("KSCREEN_BACKEND", "Fake", 1);
}

void testScreenConfig::singleOutput()
{
    loadConfig("singleOutput.json");

    Output* output = Generator::self()->idealConfig()->outputs().value(1);
    QCOMPARE(output->currentModeId(), QLatin1String("3"));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);
    QCOMPARE(output->pos(), QPoint(0,0));

}

void testScreenConfig::laptopLidOpenAndExternal()
{
    loadConfig("laptopAndExternal.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);

    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));

    QCOMPARE(external->currentModeId(), QLatin1String("4"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

void testScreenConfig::laptopLidOpenAndTwoExternal()
{
    loadConfig("laptopLidOpenAndTwoExternal.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* hdmi1 = config->outputs().value(2);
    Output* hdmi2 = config->outputs().value(3);

    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));

    QCOMPARE(hdmi1->currentModeId(), QLatin1String("4"));
    QCOMPARE(hdmi1->isPrimary(), false);
    QCOMPARE(hdmi1->isEnabled(), true);
    QCOMPARE(hdmi1->pos(), QPoint(hdmi2->pos().x() + hdmi2->currentMode()->size().width(), 0));

    QCOMPARE(hdmi2->currentModeId(), QLatin1String("4"));
    QCOMPARE(hdmi2->isPrimary(), false);
    QCOMPARE(hdmi2->isEnabled(), true);
    QCOMPARE(hdmi2->pos(), QPoint(1280, 0));

}

void testScreenConfig::laptopLidClosedAndExternal()
{
    loadConfig("laptopAndExternal.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);
    generator->setForceLidClosed(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);

    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(laptop->isPrimary(), false);

    QCOMPARE(external->currentModeId(), QLatin1String("4"));
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void testScreenConfig::laptopLidClosedAndThreeExternal()
{
    loadConfig("laptopLidClosedAndThreeExternal.json");

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
    QCOMPARE(hdmi1->currentModeId(), QLatin1String("4"));
    QCOMPARE(hdmi1->pos(), QPoint(primary->currentMode()->size().width(), 0));

    QCOMPARE(hdmi2->isEnabled(), true);
    QCOMPARE(hdmi2->isPrimary(), false);
    QCOMPARE(hdmi2->currentModeId(), QLatin1String("3"));
    QCOMPARE(hdmi2->pos(), QPoint(hdmi1->pos().x() + hdmi1->currentMode()->size().width(), 0));

    QCOMPARE(primary->isEnabled(), true);
    QCOMPARE(primary->isPrimary(), true);
    QCOMPARE(primary->currentModeId(), QLatin1String("4"));
    QCOMPARE(primary->pos(), QPoint(0,0));
}

void testScreenConfig::laptopDockedLidOpenAndExternal()
{
    loadConfig("laptopAndExternal.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);
    generator->setForceLidClosed(false);
    generator->setForceDocked(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);

    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), false);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));

    QCOMPARE(external->currentModeId(), QLatin1String("4"));
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

void testScreenConfig::laptopDockedLidClosedAndExternal()
{
    loadConfig("laptopAndExternal.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);
    generator->setForceLidClosed(true);
    generator->setForceDocked(true);

    Config* config = generator->idealConfig();
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);

    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(laptop->isPrimary(), false);

    QCOMPARE(external->currentModeId(), QLatin1String("4"));
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void testScreenConfig::workstationWithoutScreens()
{
    loadConfig("workstationWithoutScreens.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(false);

    Config* config = generator->idealConfig();

    QVERIFY(config->outputs().isEmpty());
}

void testScreenConfig::workstationWithNoConnectedScreens()
{
    loadConfig("workstationWithNoConnectedScreens.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(false);

    Config* config = generator->idealConfig();

    Output* external1 = config->output(1);
    Output* external2 = config->output(2);

    QCOMPARE(external1->isEnabled(), false);
    QCOMPARE(external2->isEnabled(), false);
}

void testScreenConfig::workstationTwoExternalSameSize()
{
    loadConfig("workstaionTwoExternalSameSize.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(false);

    Config* config = generator->idealConfig();
    Output* external1 = config->output(1);
    Output* external2 = config->output(2);

    QCOMPARE(external1->isPrimary(), true);
    QCOMPARE(external1->isEnabled(), true);
    QCOMPARE(external1->currentModeId(), QLatin1String("3"));
    QCOMPARE(external1->pos(), QPoint(0 ,0));

    QCOMPARE(external2->isPrimary(), false);
    QCOMPARE(external2->isEnabled(), true);
    QCOMPARE(external2->currentModeId(), QLatin1String("3"));
    QCOMPARE(external2->pos(), QPoint(external1->currentMode()->size().width() ,0));
}

void testScreenConfig::workstationTwoExternalDiferentSize()
{

    loadConfig("workstationTwoExternalDiferentSize.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(false);

    Config* config = generator->idealConfig();
    Output* external1 = config->output(1);
    Output* external2 = config->output(2);

    QCOMPARE(external1->isPrimary(), false);
    QCOMPARE(external1->isEnabled(), true);
    QCOMPARE(external1->currentModeId(), QLatin1String("3"));
    QCOMPARE(external1->pos(), QPoint(external2->currentMode()->size().width() ,0));

    QCOMPARE(external2->isPrimary(), true);
    QCOMPARE(external2->isEnabled(), true);
    QCOMPARE(external2->currentModeId(), QLatin1String("4"));
}

void testScreenConfig::switchDisplayTwoScreens()
{
    loadConfig("switchDisplayTwoScreens.json");

    Generator* generator = Generator::self();
    generator->setForceLaptop(true);
    generator->setForceDocked(false);
    generator->setForceLidClosed(false);

    //Clone all
    Config* config = generator->displaySwitch(1);
    Output* laptop = config->outputs().value(1);
    Output* external = config->outputs().value(2);
    QCOMPARE(laptop->currentModeId(), QLatin1String("2"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(external->currentModeId(), QLatin1String("3"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));

    //Extend to left
    config = generator->displaySwitch(2);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);
    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(1920, 0));
    QCOMPARE(external->currentModeId(), QLatin1String("5"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));

    //Disable embedded,. enable external
    config = generator->displaySwitch(3);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);;
    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(external->currentModeId(), QLatin1String("5"));
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));

    //Enable embedded, disable external
    config = generator->displaySwitch(4);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);;
    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));;
    QCOMPARE(external->isEnabled(), false);

    //Extend to right
    config = generator->displaySwitch(5);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);
    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(external->currentModeId(),QLatin1String("5"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

QTEST_MAIN(testScreenConfig)

#include "testscreenconfig.moc"
