/*************************************************************************************
 *  Copyright 2016 by Sebastian Kügler <sebas@kde.org>                               *
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

#include "configmodule.h"
#include "qmlscreen.h"
#include "modeselector.h"
#include "debug_p.h"

#include <QtTest>
#include <QtCore/QObject>

#include <kscreen/config.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/backendmanager_p.h>

namespace KScreen {

class TestConfigModule : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void loadConfigModule();
    void testModeSelector();
    void testLabels();
    void testInvalidIndex();
    void testResolution();
    void testRefreshIndex();

private:
    KScreen::ConfigPtr setConfig(const QByteArray &fileName);
    void printModes(ModeSelector *modeselector);
    KScreen::ConfigPtr m_currentConfig;
};

KScreen::ConfigPtr TestConfigModule::setConfig(const QByteArray& fileName)
{
    KScreen::BackendManager::instance()->shutdownBackend();

    QByteArray path(TEST_DATA "configs/" + fileName);
    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" + path);
    //qDebug() << "Using config:" << path;

    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation;
    if (!op->exec()) {
        qWarning() << op->errorString();
        return ConfigPtr();
    }
    return op->config();
}

void TestConfigModule::printModes(ModeSelector* modeselector)
{
    int i = 0;
    for (auto msize : modeselector->modeSizes()) {
        qCDebug(KSCREEN_KCM) << "       " << i << msize << modeselector->m_refreshRatesTable[msize];
        i++;
    }
}

QMLOutput* findQMLOutput(QList<QMLOutput*> outputs, const QString& name) {
    for (const auto output : outputs) {
        if (output->output() && output->output()->name() == name) {
            return output;
        }
    }
    return nullptr;
}

void TestConfigModule::initTestCase()
{
    qputenv("KSCREEN_LOGGING", "false");
    setenv("KSCREEN_BACKEND", "Fake", 1);
}

void TestConfigModule::cleanupTestCase()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    m_currentConfig = ConfigPtr();
}

void TestConfigModule::loadConfigModule()
{
    m_currentConfig = setConfig("yogaAndTv.json");
    QVERIFY(!m_currentConfig.isNull());
    QVERIFY(m_currentConfig->isValid());

    auto configmodule = new ConfigModule(this, QVariantList());
    QVERIFY(configmodule->currentConfig().isNull());

    QSignalSpy configSpy(configmodule, &ConfigModule::configSet);
    configmodule->load();
    QVERIFY(configmodule->currentConfig().isNull());
    QVERIFY(configSpy.wait(1000));
    QVERIFY(!configmodule->currentConfig().isNull());
    QVERIFY(configmodule->currentConfig()->isValid());

    // some basic integrity checks for the QMLScreen
    QVERIFY(configmodule->mScreen != nullptr);
    QCOMPARE(configmodule->mScreen->outputs().count(), 8);
}

void TestConfigModule::testModeSelector()
{
    const ConfigPtr currentConfig = setConfig("yogaAndTv.json");
    auto configmodule = new ConfigModule(this, QVariantList());
    QSignalSpy configSpy(configmodule, &ConfigModule::configSet);
    configmodule->load();
    auto modeselector = configmodule->modeSelector();

    QCOMPARE(modeselector->modeLabelMin(), QString());
    QCOMPARE(modeselector->modeLabelMax(), QString());
    QCOMPARE(modeselector->refreshLabelMin(), QString());
    QCOMPARE(modeselector->refreshLabelMax(), QString());

    QVERIFY(configSpy.wait(1000));
    QVERIFY(!configmodule->currentConfig().isNull());
    QVERIFY(configmodule->currentConfig()->isValid());

    QSignalSpy modesChanged(modeselector, &ModeSelector::modesChanged);
    QSignalSpy selectedChanged(modeselector, &ModeSelector::selectedModeChanged);
    QSignalSpy refreshChanged(modeselector, &ModeSelector::refreshRatesChanged);
    QSignalSpy sizesChanged(modeselector, &ModeSelector::selectedModeChanged);

    modeselector->setSelectedResolutionIndex(5);
    QCOMPARE(refreshChanged.count(), 1);
    QCOMPARE(selectedChanged.count(), 1);

    modeselector->setSelectedRefreshRate(1);
    QCOMPARE(refreshChanged.count(), 1);
    QCOMPARE(selectedChanged.count(), 2);
    QCOMPARE(modeselector->refreshRates().count(), 3);
    modeselector->setSelectedRefreshRate(2);
    QCOMPARE(selectedChanged.count(), 3);
    modeselector->setSelectedRefreshRate(2);
    QCOMPARE(selectedChanged.count(), 3);

    QCOMPARE(modesChanged.count(), 0);

    auto qmltv = findQMLOutput(configmodule->mScreen->outputs(), "HDMI-2");
    QVERIFY(qmltv != nullptr);
    configmodule->focusedOutputChanged(qmltv);
    QCOMPARE(modesChanged.count(), 1);
    QCOMPARE(qmltv->modeSizes().count(), 17);

    modeselector->setSelectedResolutionIndex(1);
    modeselector->setSelectedRefreshRate(6);

    const int rcount = refreshChanged.count();
    const int mcount = modesChanged.count();

    modeselector->setSelectedResolutionIndex(1);
    modeselector->setSelectedRefreshRate(6);

    modeselector->setSelectedResolutionIndex(1);
    modeselector->setSelectedRefreshRate(6);

    QCOMPARE(modesChanged.count(), mcount);
    QCOMPARE(refreshChanged.count(), rcount);

    // change the focused output, make sure we get update signals
    configmodule->focusedOutputChanged(findQMLOutput(configmodule->mScreen->outputs(), "eDP-1"));

    QCOMPARE(refreshChanged.count(), rcount + 1);
    QCOMPARE(modesChanged.count(), 2);
    QCOMPARE(modeselector->modeSizes().count(), 33);
}

void TestConfigModule::testInvalidIndex()
{
    setConfig("yogaAndTv.json");
    auto configmodule = new ConfigModule(this, QVariantList());
    QSignalSpy configSpy(configmodule, &ConfigModule::configSet);
    auto modeselector = configmodule->modeSelector();
    QSignalSpy selectedChanged(modeselector, &ModeSelector::selectedModeChanged);
    QSignalSpy refreshChanged(modeselector, &ModeSelector::refreshRatesChanged);

    configmodule->load();
    QVERIFY(configSpy.wait(1000));

    modeselector->setSelectedResolutionIndex(0);
    const int rbefore = refreshChanged.count();
    const int sbefore = selectedChanged.count();
    QCOMPARE(refreshChanged.count(), rbefore);
    QCOMPARE(selectedChanged.count(), sbefore);

    // Some invalid calls, make sure no unnecessary signals arrive
    modeselector->setSelectedRefreshRate(6);
    QCOMPARE(refreshChanged.count(), rbefore);
    QCOMPARE(selectedChanged.count(), sbefore);

    modeselector->setSelectedResolutionIndex(9999);
    QCOMPARE(refreshChanged.count(), rbefore);
    QCOMPARE(selectedChanged.count(), sbefore);

    // this is 1 out of bounds, it's going to be ignored
    modeselector->setSelectedRefreshRate(modeselector->refreshRates().count());
    QCOMPARE(refreshChanged.count(), rbefore);
    QCOMPARE(selectedChanged.count(), sbefore);
}

void TestConfigModule::testLabels()
{
    const ConfigPtr currentConfig = setConfig("yogaAndTv.json");
    auto configmodule = new ConfigModule(this, QVariantList());
    QSignalSpy configSpy(configmodule, &ConfigModule::configSet);
    configmodule->load();
    auto modeselector = configmodule->modeSelector();

    QCOMPARE(modeselector->modeLabelMin(), QString());
    QCOMPARE(modeselector->modeLabelMax(), QString());
    QCOMPARE(modeselector->refreshLabelMin(), QString());
    QCOMPARE(modeselector->refreshLabelMax(), QString());

    QVERIFY(configSpy.wait(1000));
    QVERIFY(!configmodule->currentConfig().isNull());
    QVERIFY(configmodule->currentConfig()->isValid());
    QCOMPARE(modeselector->modeLabelMin(), QStringLiteral("320x240"));
    QCOMPARE(modeselector->modeLabelMax(), QStringLiteral("2560x1440"));
    QCOMPARE(modeselector->refreshLabelMin(), QStringLiteral("48.00"));
    QCOMPARE(modeselector->refreshLabelMax(), QStringLiteral("60.00"));

    configmodule->focusedOutputChanged(findQMLOutput(configmodule->mScreen->outputs(), "HDMI-2"));
    QCOMPARE(modeselector->modeLabelMin(), QStringLiteral("720x400"));
    QCOMPARE(modeselector->modeLabelMax(), QStringLiteral("1920x1080"));
    QCOMPARE(modeselector->refreshLabelMin(), QStringLiteral("23.98"));
    QCOMPARE(modeselector->refreshLabelMax(), QStringLiteral("60.00"));

    modeselector->setSelectedResolutionIndex(1);
    modeselector->setSelectedRefreshRate(6);
    QCOMPARE(modeselector->refreshLabelMin(), QStringLiteral("59.94"));
    QCOMPARE(modeselector->refreshLabelMax(), QStringLiteral("120.00"));

    configmodule->focusedOutputChanged(findQMLOutput(configmodule->mScreen->outputs(), "eDP-1"));
    QCOMPARE(modeselector->modeLabelMin(), QStringLiteral("320x240"));
    QCOMPARE(modeselector->modeLabelMax(), QStringLiteral("2560x1440"));
    QCOMPARE(modeselector->refreshLabelMin(), QStringLiteral("48.00"));
    QCOMPARE(modeselector->refreshLabelMax(), QStringLiteral("60.00"));

}

void TestConfigModule::testResolution()
{

}

void TestConfigModule::testRefreshIndex()
{
    const ConfigPtr currentConfig = setConfig("yogaAndTv.json");
    auto configmodule = new ConfigModule(this, QVariantList());
    QSignalSpy configSpy(configmodule, &ConfigModule::configSet);
    configmodule->load();
    auto modeselector = configmodule->modeSelector();
    QVERIFY(configSpy.wait(1000));
    configmodule->focusedOutputChanged(findQMLOutput(configmodule->mScreen->outputs(), "HDMI-2"));
    int rindex = modeselector->modeSizes().count() - 1;
    //modeselector->setSelectedResolutionIndex(rindex);

    int sr = modeselector->preferredRefreshIndexForSizeIndex(rindex);
    modeselector->setSelectedRefreshRate(sr);
    qreal r50 = modeselector->output()->currentMode()->refreshRate();
    //QCOMPARE(r50, 50.00);

    QCOMPARE(modeselector->refreshRates().count(), 10);
    modeselector->setSelectedRefreshRate(0);
    qCDebug(KSCREEN_KCM) << "modeselector->refreshRates()" << modeselector->refreshRates();
    QCOMPARE(modeselector->refreshRates().count(), 10);
    qCDebug(KSCREEN_KCM) << " xxx" << modeselector->output()->currentMode()->refreshRate();
}

}

QTEST_MAIN(KScreen::TestConfigModule)

#include "testconfigmodule.moc"
