/*************************************************************************************
 *  Copyright (C) 2015 by Daniel Vrátil <dvratil@redhat.com>                         *
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

#include "../../kded/serializer.h"

#include <QtTest>
#include <QObject>

#include <KScreen/Config>
#include <KScreen/EDID>
#include <KScreen/Screen>
#include <KScreen/Mode>
#include <KScreen/Output>

using namespace KScreen;

class TestSerializer : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testSimpleConfig();
    void testTwoScreenConfig();
    void testRotatedScreenConfig();
    void testDisabledScreenConfig();
    void testConfig404();
    void testCorruptConfig();
    void testCorruptEmptyConfig();
    void testCorruptUselessConfig();
    void testNullConfig();
    void testIdenticalOutputs();
    void testMoveConfig();
    void testFixedConfig();

private:
    KScreen::ConfigPtr createConfig(bool output1Connected, bool output2Conected);
};

ConfigPtr TestSerializer::createConfig(bool output1Connected, bool output2Connected)
{
    KScreen::ScreenPtr screen = KScreen::ScreenPtr::create();
    screen->setCurrentSize(QSize(1920, 1080));
    screen->setMaxSize(QSize(32768, 32768));
    screen->setMinSize(QSize(8, 8));

    QList<QSize> sizes({ QSize(320, 240), QSize(640, 480), QSize(1024, 768), QSize(1280, 1024), QSize(1920, 1280) });
    KScreen::ModeList modes;
    for (int i = 0; i < sizes.count(); ++i) {
        const QSize &size = sizes[i];
        KScreen::ModePtr mode = KScreen::ModePtr::create();
        mode->setId(QStringLiteral("MODE-%1").arg(i));
        mode->setName(QStringLiteral("%1x%2").arg(size.width()).arg(size.height()));
        mode->setSize(size);
        mode->setRefreshRate(60.0);
        modes.insert(mode->id(), mode);
    }

    KScreen::OutputPtr output1 = KScreen::OutputPtr::create();
    output1->setId(1);
    output1->setName(QStringLiteral("OUTPUT-1"));
    output1->setPos(QPoint(0, 0));
    output1->setConnected(output1Connected);
    output1->setEnabled(output1Connected);
    if (output1Connected) {
        output1->setModes(modes);
    }

    KScreen::OutputPtr output2 = KScreen::OutputPtr::create();
    output2->setId(2);
    output2->setName(QStringLiteral("OUTPUT-2"));
    output2->setPos(QPoint(0, 0));
    output2->setConnected(output2Connected);
    if (output2Connected) {
        output2->setModes(modes);
    }

    KScreen::ConfigPtr config = KScreen::ConfigPtr::create();
    config->setScreen(screen);
    config->addOutput(output1);
    config->addOutput(output2);

    return config;
}

void TestSerializer::initTestCase()
{
    qputenv("KSCREEN_LOGGING", "false");
    Serializer::setConfigPath(QStringLiteral(TEST_DATA "/serializerdata/"));
}

void TestSerializer::testSimpleConfig()
{
    KScreen::ConfigPtr config = createConfig(true, false);
    config = Serializer::config(config, QStringLiteral("simpleConfig.json"));
    QVERIFY(config);

    QCOMPARE(config->connectedOutputs().count(), 1);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-4"));
    QCOMPARE(output->currentMode()->size(), QSize(1920, 1280));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->isPrimary(), true);

    auto screen = config->screen();
    QCOMPARE(screen->currentSize(), QSize(1920, 1280));
}

void TestSerializer::testTwoScreenConfig()
{
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("twoScreenConfig.json"));
    QVERIFY(config);

    QCOMPARE(config->connectedOutputs().count(), 2);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-4"));
    QCOMPARE(output->currentMode()->size(), QSize(1920, 1280));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->isPrimary(), true);

    output = config->connectedOutputs().last();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-3"));
    QCOMPARE(output->currentMode()->size(), QSize(1280, 1024));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(1920, 0));
    QCOMPARE(output->isPrimary(), false);

    auto screen = config->screen();
    QCOMPARE(screen->currentSize(), QSize(3200, 1280));
}

void TestSerializer::testRotatedScreenConfig()
{
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("rotatedScreenConfig.json"));
    QVERIFY(config);

    QCOMPARE(config->connectedOutputs().count(), 2);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-4"));
    QCOMPARE(output->currentMode()->size(), QSize(1920, 1280));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->isPrimary(), true);

    output = config->connectedOutputs().last();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-3"));
    QCOMPARE(output->currentMode()->size(), QSize(1280, 1024));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::Left);
    QCOMPARE(output->pos(), QPoint(1920, 0));
    QCOMPARE(output->isPrimary(), false);

    auto screen = config->screen();
    QCOMPARE(screen->currentSize(), QSize(2944, 1280));
}

void TestSerializer::testDisabledScreenConfig()
{
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("disabledScreenConfig.json"));
    QVERIFY(config);

    QCOMPARE(config->connectedOutputs().count(), 2);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-4"));
    QCOMPARE(output->currentMode()->size(), QSize(1920, 1280));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->isPrimary(), true);

    output = config->connectedOutputs().last();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output->isEnabled(), false);

    auto screen = config->screen();
    QCOMPARE(screen->currentSize(), QSize(1920, 1280));
}

void TestSerializer::testConfig404()
{
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("filenotfoundConfig.json"));
    QVERIFY(!config);
    QVERIFY(config.isNull());
}

void TestSerializer::testCorruptConfig()
{
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("corruptConfig.json"));
    QVERIFY(config);
    QCOMPARE(config->outputs().count(), 2);
    QVERIFY(config->isValid());
}

void TestSerializer::testCorruptEmptyConfig()
{
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("corruptEmptyConfig.json"));
    QVERIFY(config);
    QCOMPARE(config->outputs().count(), 2);
    QVERIFY(config->isValid());
}

void TestSerializer::testCorruptUselessConfig()
{
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("corruptUselessConfig.json"));
    QVERIFY(config);
    QCOMPARE(config->outputs().count(), 2);
    QVERIFY(config->isValid());
}

void TestSerializer::testNullConfig()
{
    KScreen::ConfigPtr nullConfig;
    QVERIFY(!nullConfig);

    // Null configs have empty configIds
    QVERIFY(Serializer::configId(nullConfig).isEmpty());

    // Load config from a file not found results in a nullptr
    KScreen::ConfigPtr config = createConfig(true, true);
    QVERIFY(!Serializer::config(config, QString()));

    // Wrong config file name should fail to save
    QCOMPARE(Serializer::saveConfig(config, QString()), false);
}

void TestSerializer::testIdenticalOutputs()
{
    // Test configuration of a video wall with 6 identical outputs connected
    // this is the autotest for https://bugs.kde.org/show_bug.cgi?id=325277
    KScreen::ScreenPtr screen = KScreen::ScreenPtr::create();
    screen->setCurrentSize(QSize(1920, 1080));
    screen->setMaxSize(QSize(32768, 32768));
    screen->setMinSize(QSize(8, 8));

    QList<QSize> sizes({ QSize(640, 480), QSize(1024, 768), QSize(1920, 1080), QSize(1280, 1024), QSize(1920, 1280) });
    KScreen::ModeList modes;
    for (int i = 0; i < sizes.count(); ++i) {
        const QSize &size = sizes[i];
        KScreen::ModePtr mode = KScreen::ModePtr::create();
        mode->setId(QStringLiteral("MODE-%1").arg(i));
        mode->setName(QStringLiteral("%1x%2").arg(size.width()).arg(size.height()));
        mode->setSize(size);
        mode->setRefreshRate(60.0);
        modes.insert(mode->id(), mode);
    }
    // This one is important, the output id in the config file is a hash of it
    QByteArray data = QByteArray::fromBase64("AP///////wAQrBbwTExLQQ4WAQOANCB46h7Frk80sSYOUFSlSwCBgKlA0QBxTwEBAQEBAQEBKDyAoHCwI0AwIDYABkQhAAAaAAAA/wBGNTI1TTI0NUFLTEwKAAAA/ABERUxMIFUyNDEwCiAgAAAA/QA4TB5REQAKICAgICAgAToCAynxUJAFBAMCBxYBHxITFCAVEQYjCQcHZwMMABAAOC2DAQAA4wUDAQI6gBhxOC1AWCxFAAZEIQAAHgEdgBhxHBYgWCwlAAZEIQAAngEdAHJR0B4gbihVAAZEIQAAHowK0Iog4C0QED6WAAZEIQAAGAAAAAAAAAAAAAAAAAAAPg==");

    // When setting up the outputs, make sure they're not added in alphabetical order
    // or in the same order of the config file, as that makes the tests accidentally pass

    KScreen::OutputPtr output1 = KScreen::OutputPtr::create();
    output1->setId(1);
    output1->setEdid(data);
    output1->setName(QStringLiteral("DisplayPort-0"));
    output1->setPos(QPoint(0, 0));
    output1->setConnected(true);
    output1->setEnabled(false);
    output1->setModes(modes);

    KScreen::OutputPtr output2 = KScreen::OutputPtr::create();
    output2->setId(2);
    output2->setEdid(data);
    output2->setName(QStringLiteral("DisplayPort-1"));
    output2->setPos(QPoint(0, 0));
    output2->setConnected(true);
    output2->setEnabled(false);
    output2->setModes(modes);

    KScreen::OutputPtr output3 = KScreen::OutputPtr::create();
    output3->setId(3);
    output3->setEdid(data);
    output3->setName(QStringLiteral("DisplayPort-2"));
    output3->setPos(QPoint(0, 0));
    output3->setConnected(true);
    output3->setEnabled(false);
    output3->setModes(modes);

    KScreen::OutputPtr output6 = KScreen::OutputPtr::create();
    output6->setId(6);
    output6->setEdid(data);
    output6->setName(QStringLiteral("DVI-0"));
    output6->setPos(QPoint(0, 0));
    output6->setConnected(true);
    output6->setEnabled(false);
    output6->setModes(modes);

    KScreen::OutputPtr output4 = KScreen::OutputPtr::create();
    output4->setId(4);
    output4->setEdid(data);
    output4->setName(QStringLiteral("DisplayPort-3"));
    output4->setPos(QPoint(0, 0));
    output4->setConnected(true);
    output4->setEnabled(false);
    output4->setModes(modes);

    KScreen::OutputPtr output5 = KScreen::OutputPtr::create();
    output5->setId(5);
    output5->setEdid(data);
    output5->setName(QStringLiteral("DVI-1"));
    output5->setPos(QPoint(0, 0));
    output5->setConnected(true);
    output5->setEnabled(false);
    output5->setModes(modes);

    KScreen::ConfigPtr config = KScreen::ConfigPtr::create();
    config->setScreen(screen);
    config->addOutput(output6);
    config->addOutput(output2);
    config->addOutput(output5);
    config->addOutput(output4);
    config->addOutput(output3);
    config->addOutput(output1);

    QHash<QString, QPoint> positions;
    positions["DisplayPort-0"] = QPoint(0, 1080);
    positions["DisplayPort-1"] = QPoint(2100, 30);
    positions["DisplayPort-2"] = QPoint(2100, 1080);
    positions["DisplayPort-3"] = QPoint(4020, 0);
    positions["DVI-0"] = QPoint(4020, 1080);
    positions["DVI-1"] = QPoint(0, 0);

    config = Serializer::config(config, QStringLiteral("outputgrid_2x3.json"));
    QVERIFY(config);

    QCOMPARE(config->connectedOutputs().count(), 6);
    Q_FOREACH (auto output, config->connectedOutputs()) {
        QVERIFY(positions.keys().contains(output->name()));
        QVERIFY(output->name() != Serializer::outputId(output));
        QCOMPARE(positions[output->name()], output->pos());
        QCOMPARE(output->currentMode()->size(), QSize(1920, 1080));
        QCOMPARE(output->currentMode()->refreshRate(), 60.0);
        QVERIFY(output->isEnabled());
    }
    QCOMPARE(config->screen()->currentSize(), QSize(5940, 2160));
}

void TestSerializer::testMoveConfig()
{
    // Test if restoring a config using Serializer::moveConfig(src, dest) works
    // https://bugs.kde.org/show_bug.cgi?id=353029

    // Load a dualhead config
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("twoScreenConfig.json"));
    QVERIFY(config);

    // Make sure we don't write into TEST_DATA
    QStandardPaths::setTestModeEnabled(true);
    Serializer::setConfigPath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % QStringLiteral("/kscreen/"));

    // Basic assumptions for the remainder of our tests, this is the situation where the lid is opened
    QCOMPARE(config->connectedOutputs().count(), 2);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);

    auto output2 = config->connectedOutputs().last();
    QCOMPARE(output2->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output2->isEnabled(), true);
    QCOMPARE(output2->isPrimary(), false);

    // we fake the lid being closed, first save our current config to _lidOpened
    Serializer::saveConfig(config, "0xdeadbeef_lidOpened");

    // ... then switch off the panel, set primary to the other output
    output->setEnabled(false);
    output->setPrimary(false);
    output2->setPrimary(true);

    // save config as the current one, this is the config we don't want restored, and which we'll overwrite
    Serializer::saveConfig(config, "0xdeadbeef");

    QCOMPARE(output->isEnabled(), false);
    QCOMPARE(output->isPrimary(), false);
    QCOMPARE(output2->isPrimary(), true);

    // Check if both files exist
    QFile openCfg(Serializer::configFileName("0xdeadbeef_lidOpened"));
    QFile closedCfg(Serializer::configFileName("0xdeadbeef"));
    QVERIFY(openCfg.exists());
    QVERIFY(closedCfg.exists());

    // Switcheroo...
    QVERIFY(Serializer::moveConfig("0xdeadbeef_lidOpened", "0xdeadbeef"));

    // Check actual files, src should be gone, dest must exist
    QVERIFY(!openCfg.exists());
    QVERIFY(closedCfg.exists());

    // Now load the resulting config and make sure the laptop panel is enabled and primary again
    config = Serializer::config(config, "0xdeadbeef");

    output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);

    output2 = config->connectedOutputs().last();
    QCOMPARE(output2->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output2->isEnabled(), true);
    QCOMPARE(output2->isPrimary(), false);

    // Make sure we don't screw up when there's no _lidOpened config
    QVERIFY(!Serializer::moveConfig("0xdeadbeef_lidOpened", "0xdeadbeef"));

    config = Serializer::config(config, "0xdeadbeef");

    output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);

    output2 = config->connectedOutputs().last();
    QCOMPARE(output2->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output2->isEnabled(), true);
    QCOMPARE(output2->isPrimary(), false);

    Serializer::setConfigPath(QStringLiteral(TEST_DATA "/serializerdata/"));
}

void TestSerializer::testFixedConfig()
{
    // Load a dualhead config
    KScreen::ConfigPtr config = createConfig(true, true);
    config = Serializer::config(config, QStringLiteral("twoScreenConfig.json"));
    QVERIFY(config);

    // Make sure we don't write into TEST_DATA
    QStandardPaths::setTestModeEnabled(true);
    Serializer::setConfigPath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % QStringLiteral("/kscreen/"));
    // save config as the current one, this is the config we don't want restored, and which we'll overwrite
    Serializer::saveConfig(config, Serializer::sFixedConfig);

    // Check if both files exist
    QFile fixedCfg(Serializer::configFileName(Serializer::sFixedConfig));
    QVERIFY(fixedCfg.exists());
}


QTEST_MAIN(TestSerializer)

#include "serializertest.moc"
