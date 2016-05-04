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

QTEST_MAIN(TestSerializer)

#include "serializertest.moc"