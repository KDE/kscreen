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
#include <QtCore/QObject>

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
    KScreen::ScreenPtr screen = KScreen::ScreenPtr::create();
    screen->setCurrentSize(QSize(1920, 1080));
    screen->setMaxSize(QSize(32768, 32768));
    screen->setMinSize(QSize(8, 8));

    QList<QSize> sizes({ QSize(1920, 1080), QSize(640, 480), QSize(1024, 768), QSize(1280, 1024), QSize(1920, 1280) });
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
    QByteArray data = QByteArray::fromBase64("AP///////wAQrBbwTExLQQ4WAQOANCB46h7Frk80sSYOUFSlSwCBgKlA0QBxTwEBAQEBAQEBKDyAoHCwI0AwIDYABkQhAAAaAAAA/wBGNTI1TTI0NUFLTEwKAAAA/ABERUxMIFUyNDEwCiAgAAAA/QA4TB5REQAKICAgICAgAToCAynxUJAFBAMCBxYBHxITFCAVEQYjCQcHZwMMABAAOC2DAQAA4wUDAQI6gBhxOC1AWCxFAAZEIQAAHgEdgBhxHBYgWCwlAAZEIQAAngEdAHJR0B4gbihVAAZEIQAAHowK0Iog4C0QED6WAAZEIQAAGAAAAAAAAAAAAAAAAAAAPg==");

    KScreen::OutputPtr output1 = KScreen::OutputPtr::create();
    output1->setId(1);
    output1->setEdid(data);
    qDebug() << "HASH:" << output1->edid()->hash();
    qDebug() << "OutputID: " << Serializer::outputId(output1);
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
    output5->setName(QStringLiteral("DVI-0"));
    output5->setPos(QPoint(0, 0));
    output5->setConnected(true);
    output5->setEnabled(false);
    output5->setModes(modes);

    KScreen::OutputPtr output6 = KScreen::OutputPtr::create();
    output6->setId(6);
    output6->setEdid(data);
    output6->setName(QStringLiteral("DVI-1"));
    output6->setPos(QPoint(0, 0));
    output6->setConnected(true);
    output6->setEnabled(false);
    output6->setModes(modes);

    KScreen::ConfigPtr config = KScreen::ConfigPtr::create();
    config->setScreen(screen);
    config->addOutput(output1);
    config->addOutput(output2);
    config->addOutput(output3);
    config->addOutput(output4);
    config->addOutput(output5);
    config->addOutput(output6);

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
        qDebug() << " ** " << output->pos() << output->name();
        QVERIFY(output->name() != Serializer::outputId(output));
        QCOMPARE(positions[output->name()], output->pos());
        QCOMPARE(output->currentMode()->size(), QSize(1920, 1080));
        QVERIFY(output->isEnabled());
    }
    qDebug() << " sc " << config->screen()->currentSize();

    QCOMPARE(config->screen()->currentSize(), QSize(5940, 2160));
}

QTEST_MAIN(TestSerializer)

#include "serializertest.moc"