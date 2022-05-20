/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../../kded/generator.h"
#include "../../kded/output.h"

#include <QObject>
#include <QtTest>

#include <kscreen/backendmanager_p.h>
#include <kscreen/config.h>
#include <kscreen/getconfigoperation.h>

using namespace KScreen;

class testScreenConfig : public QObject
{
    Q_OBJECT

private:
    KScreen::ConfigPtr loadConfig(const QByteArray &fileName);

    void switchDisplayTwoScreensNoCommonMode();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
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
    void workstationFallbackMode();
    void workstationTwoExternalDiferentSize();
    void switchDisplayTwoScreens();
    void switchDisplayTwoScreensOneRotated();
    void globalOutputData();
    void outputPreset();
};

KScreen::ConfigPtr testScreenConfig::loadConfig(const QByteArray &fileName)
{
    KScreen::BackendManager::instance()->shutdownBackend();

    QByteArray path(TEST_DATA "configs/" + fileName);
    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" + path);
    qDebug() << path;

    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation;
    if (!op->exec()) {
        qWarning() << op->errorString();
        return ConfigPtr();
    }
    return op->config();
}

void testScreenConfig::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    qputenv("KSCREEN_LOGGING", "false");
    setenv("KSCREEN_BACKEND", "Fake", 1);
}

void testScreenConfig::cleanupTestCase()
{
    KScreen::BackendManager::instance()->shutdownBackend();
}

void testScreenConfig::singleOutput()
{
    const ConfigPtr currentConfig = loadConfig("singleOutput.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr output = config->outputs().value(1);

    QCOMPARE(output->currentModeId(), QLatin1String("3"));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);
    QCOMPARE(output->pos(), QPoint(0, 0));
}

void testScreenConfig::laptopLidOpenAndExternal()
{
    const ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr external = config->outputs().value(2);

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
    const ConfigPtr currentConfig = loadConfig("laptopLidOpenAndTwoExternal.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr hdmi1 = config->outputs().value(2);
    OutputPtr hdmi2 = config->outputs().value(3);

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
    const ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(true);
    generator->setForceLidClosed(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr external = config->outputs().value(2);

    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(laptop->isPrimary(), false);

    QCOMPARE(external->currentModeId(), QLatin1String("4"));
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void testScreenConfig::laptopLidClosedAndThreeExternal()
{
    const ConfigPtr currentConfig = loadConfig("laptopLidClosedAndThreeExternal.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(true);
    generator->setForceLidClosed(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr hdmi1 = config->outputs().value(2);
    OutputPtr hdmi2 = config->outputs().value(3);
    OutputPtr primary = config->outputs().value(4);

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
    QCOMPARE(primary->pos(), QPoint(0, 0));
}

void testScreenConfig::laptopDockedLidOpenAndExternal()
{
    const ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(true);
    generator->setForceLidClosed(false);
    generator->setForceDocked(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr external = config->outputs().value(2);

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
    const ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(true);
    generator->setForceLidClosed(true);
    generator->setForceDocked(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr external = config->outputs().value(2);

    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(laptop->isPrimary(), false);

    QCOMPARE(external->currentModeId(), QLatin1String("4"));
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void testScreenConfig::workstationWithoutScreens()
{
    const ConfigPtr currentConfig = loadConfig("workstationWithoutScreens.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(false);
    generator->setForceNotLaptop(true);

    ConfigPtr config = generator->idealConfig(currentConfig);

    QVERIFY(config->outputs().isEmpty());
}

void testScreenConfig::workstationWithNoConnectedScreens()
{
    const ConfigPtr currentConfig = loadConfig("workstationWithNoConnectedScreens.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(false);
    generator->setForceNotLaptop(true);

    ConfigPtr config = generator->idealConfig(currentConfig);

    OutputPtr external1 = config->output(1);
    OutputPtr external2 = config->output(2);

    QCOMPARE(external1->isEnabled(), false);
    QCOMPARE(external2->isEnabled(), false);
}

void testScreenConfig::workstationTwoExternalSameSize()
{
    const ConfigPtr currentConfig = loadConfig("workstaionTwoExternalSameSize.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(false);
    generator->setForceNotLaptop(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr external1 = config->output(1);
    OutputPtr external2 = config->output(2);

    QCOMPARE(external1->isPrimary(), true);
    QCOMPARE(external1->isEnabled(), true);
    QCOMPARE(external1->currentModeId(), QLatin1String("3"));
    QCOMPARE(external1->pos(), QPoint(0, 0));

    QCOMPARE(external2->isPrimary(), false);
    QCOMPARE(external2->isEnabled(), true);
    QCOMPARE(external2->currentModeId(), QLatin1String("3"));
    QCOMPARE(external2->pos(), QPoint(external1->currentMode()->size().width(), 0));
}

void testScreenConfig::workstationFallbackMode()
{
    const ConfigPtr currentConfig = loadConfig("workstationFallbackMode.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(false);
    generator->setForceNotLaptop(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr external1 = config->output(1);
    OutputPtr external2 = config->output(2);

    QCOMPARE(external1->isPrimary(), true);
    QCOMPARE(external1->isEnabled(), true);
    QCOMPARE(external1->currentModeId(), QLatin1String("1"));
    QCOMPARE(external1->pos(), QPoint(0, 0));

    QCOMPARE(external2->isPrimary(), false);
    QCOMPARE(external2->isEnabled(), true);
    QCOMPARE(external2->currentModeId(), QLatin1String("1"));
    QCOMPARE(external2->pos(), QPoint(0, 0));
}

void testScreenConfig::workstationTwoExternalDiferentSize()
{
    const ConfigPtr currentConfig = loadConfig("workstationTwoExternalDiferentSize.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(false);
    generator->setForceNotLaptop(true);

    ConfigPtr config = generator->idealConfig(currentConfig);
    OutputPtr external1 = config->output(1);
    OutputPtr external2 = config->output(2);

    QCOMPARE(external1->isPrimary(), false);
    QCOMPARE(external1->isEnabled(), true);
    QCOMPARE(external1->currentModeId(), QLatin1String("3"));
    QCOMPARE(external1->pos(), QPoint(external2->currentMode()->size().width(), 0));

    QCOMPARE(external2->isPrimary(), true);
    QCOMPARE(external2->isEnabled(), true);
    QCOMPARE(external2->currentModeId(), QLatin1String("4"));
}

void testScreenConfig::switchDisplayTwoScreens()
{
    const ConfigPtr currentConfig = loadConfig("switchDisplayTwoScreens.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(true);
    generator->setForceNotLaptop(false);
    generator->setForceDocked(false);
    generator->setForceLidClosed(false);

    // Clone all
    ConfigPtr config = generator->displaySwitch(Generator::Clone);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr external = config->outputs().value(2);
    QCOMPARE(laptop->currentModeId(), QLatin1String("2"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(external->currentModeId(), QLatin1String("3"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));

    // Extend to left
    config = generator->displaySwitch(Generator::ExtendToLeft);
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

    // Disable embedded,. enable external
    config = generator->displaySwitch(Generator::TurnOffEmbedded);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);
    ;
    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(external->currentModeId(), QLatin1String("5"));
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));

    // Enable embedded, disable external
    config = generator->displaySwitch(Generator::TurnOffExternal);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);
    ;
    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    ;
    QCOMPARE(external->isEnabled(), false);

    // Extend to right
    config = generator->displaySwitch(Generator::ExtendToRight);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);
    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(external->currentModeId(), QLatin1String("5"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

void testScreenConfig::switchDisplayTwoScreensOneRotated()
{
    const ConfigPtr currentConfig = loadConfig("switchDisplayTwoScreensOneRotated.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    generator->setForceLaptop(true);
    generator->setForceNotLaptop(false);
    generator->setForceDocked(false);
    generator->setForceLidClosed(false);

    QCOMPARE(currentConfig->outputs().value(1)->rotation(), KScreen::Output::Right);
    {
        auto config = Generator::self()->idealConfig(currentConfig);
        OutputPtr laptop = config->outputs().value(1);
        OutputPtr external = config->outputs().value(2);

        QCOMPARE(laptop->pos(), QPoint(0, 0));
        QCOMPARE(external->pos(), QPoint(800, 0));
    }

    // Skipping cloning for now, I am not sure what's the best way forward here.
    // We probably should not offer the option to clone if both displays have a different ratio?

    // Extend to left
    ConfigPtr config = generator->displaySwitch(Generator::ExtendToLeft);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr external = config->outputs().value(2);
    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(1920, 0));
    QCOMPARE(laptop->rotation(), KScreen::Output::Right);
    QCOMPARE(external->currentModeId(), QLatin1String("5"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));

    // Disable embedded,. enable external
    config = generator->displaySwitch(Generator::TurnOffEmbedded);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);
    ;
    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(external->currentModeId(), QLatin1String("5"));
    QCOMPARE(external->isPrimary(), true);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));

    // Enable embedded, disable external
    config = generator->displaySwitch(Generator::TurnOffExternal);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);
    ;
    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(laptop->rotation(), KScreen::Output::Right);
    ;
    QCOMPARE(external->isEnabled(), false);

    // Extend to right
    config = generator->displaySwitch(Generator::ExtendToRight);
    laptop = config->outputs().value(1);
    external = config->outputs().value(2);
    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(laptop->rotation(), KScreen::Output::Right);
    QCOMPARE(external->currentModeId(), QLatin1String("5"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(800, 0));
}

void testScreenConfig::switchDisplayTwoScreensNoCommonMode()
{
    const ConfigPtr currentConfig = loadConfig("switchDisplayTwoScreensNoCommonMode.json");
    QVERIFY(currentConfig);

    Generator *generator = Generator::self();
    generator->setCurrentConfig(currentConfig);
    ConfigPtr config = generator->displaySwitch(Generator::Clone);
    OutputPtr laptop = config->outputs().value(1);
    OutputPtr external = config->outputs().value(2);

    QCOMPARE(laptop->currentModeId(), QLatin1String("3"));
    QCOMPARE(laptop->isPrimary(), true);
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(external->currentModeId(), QLatin1String("5"));
    QCOMPARE(external->isPrimary(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void testScreenConfig::globalOutputData()
{
    const ConfigPtr currentConfig = loadConfig("singleOutput.json");

    auto config = Generator::self()->idealConfig(currentConfig);
    auto output = config->connectedOutputs().first();
    QCOMPARE(output->currentModeId(), QLatin1String("3"));
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->scale(), 1.0);

    output->setCurrentModeId(QStringLiteral("2"));
    output->setRotation(KScreen::Output::Left);
    output->setScale(2.0);
    ::Output::writeGlobal(output);

    config = Generator::self()->idealConfig(currentConfig);
    output = config->connectedOutputs().first();
    QCOMPARE(output->currentModeId(), QLatin1String("2"));
    QCOMPARE(output->rotation(), KScreen::Output::Left);
    // Fake backend does not support perOutputScale
    QCOMPARE(output->scale(), 1.0);

    // But simulate it now
    currentConfig->setSupportedFeatures(currentConfig->supportedFeatures() | KScreen::Config::Feature::PerOutputScaling);
    config = Generator::self()->idealConfig(currentConfig);
    output = config->connectedOutputs().first();
    QCOMPARE(output->currentModeId(), QLatin1String("2"));
    QCOMPARE(output->rotation(), KScreen::Output::Left);
    QCOMPARE(output->scale(), 2.0);

    // cleanup
    QFile::remove(::Output::dirPath() + output->hashMd5());
}

void testScreenConfig::outputPreset()
{
    const ConfigPtr currentConfig = loadConfig("singleOutput.json");
    currentConfig->setSupportedFeatures(currentConfig->supportedFeatures() | KScreen::Config::Feature::PerOutputScaling);
    auto defaultOutput = Generator::self()->idealConfig(currentConfig)->connectedOutputs().first();
    QCOMPARE(defaultOutput->currentModeId(), QLatin1String("3"));
    QCOMPARE(defaultOutput->rotation(), KScreen::Output::None);
    QCOMPARE(defaultOutput->scale(), 1.0);

    // Create the preset
    QTemporaryDir dataDir;
    qputenv("XDG_DATA_DIRS", dataDir.path().toUtf8());
    QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
    auto presetOutput = defaultOutput->clone();
    presetOutput->setCurrentModeId(QStringLiteral("2"));
    presetOutput->setRotation(KScreen::Output::Left);
    presetOutput->setScale(2.0);
    ::Output::writeGlobal(presetOutput);
    QDir(dataDir.path()).mkpath(QStringLiteral("kscreen/outputs"));
    QFile::copy(::Output::dirPath() + presetOutput->hashMd5(), dataDir.filePath(QStringLiteral("kscreen/outputs/") % presetOutput->hashMd5()));
    QFile::remove(::Output::dirPath() + presetOutput->hashMd5());

    auto config = Generator::self()->idealConfig(currentConfig);
    auto output = config->connectedOutputs().first();
    QCOMPARE(output->currentModeId(), QLatin1String("2"));
    QCOMPARE(output->rotation(), KScreen::Output::Left);
    QCOMPARE(output->scale(), 2.0);

    // But local global settings should still overwrite
    ::Output::writeGlobal(defaultOutput);
    config = Generator::self()->idealConfig(currentConfig);
    output = config->connectedOutputs().first();
    QCOMPARE(output->currentModeId(), QLatin1String("3"));
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->scale(), 1.0);

    QFile::remove(::Output::dirPath() + defaultOutput->hashMd5());
}

QTEST_MAIN(testScreenConfig)

#include "testgenerator.moc"
