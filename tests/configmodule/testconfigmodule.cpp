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

private:
    KScreen::ConfigPtr loadConfig(const QByteArray &fileName);
    void printModes(ModeSelector *modeselector);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void loadConfigModule();
    void testModeSelector();
};

KScreen::ConfigPtr TestConfigModule::loadConfig(const QByteArray& fileName)
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

void TestConfigModule::printModes(ModeSelector* modeselector)
{
    int i = 0;
    for (auto msize : modeselector->modeSizes()) {
        qCDebug(KSCREEN_KCM) << "       " << i << msize << modeselector->m_modesTable[msize];
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
}

void TestConfigModule::loadConfigModule()
{
    const ConfigPtr currentConfig = loadConfig("yogaAndTv.json");
    // ...
    QVERIFY(!currentConfig.isNull());
    QVERIFY(currentConfig->isValid());
    auto configmodule = new ConfigModule(this, QVariantList());
    QVERIFY(configmodule->currentConfig().isNull());
    configmodule->load();
    QVERIFY(configmodule->currentConfig().isNull());
    QSignalSpy configSpy(configmodule, &ConfigModule::configSet);

    QVERIFY(configSpy.wait(1000));
    QVERIFY(!configmodule->currentConfig().isNull());
    QVERIFY(configmodule->currentConfig()->isValid());

    QVERIFY(configmodule->mScreen != nullptr);
    QCOMPARE(configmodule->mScreen->outputs().count(), 8);

}

void TestConfigModule::testModeSelector()
{
    const ConfigPtr currentConfig = loadConfig("yogaAndTv.json");
    auto configmodule = new ConfigModule(this, QVariantList());
    configmodule->load();
    QSignalSpy configSpy(configmodule, &ConfigModule::configSet);
    QVERIFY(configSpy.wait(1000));
    QVERIFY(!configmodule->currentConfig().isNull());
    QVERIFY(configmodule->currentConfig()->isValid());

    auto qmlpanel = configmodule->mScreen->outputs().first();

    auto modeselector = configmodule->modeSelector();
    QSignalSpy modesChanged(modeselector, &ModeSelector::modesChanged);
    QSignalSpy selectedChanged(modeselector, &ModeSelector::selectedModeChanged);
    QSignalSpy refreshChanged(modeselector, &ModeSelector::refreshRatesChanged);
    QSignalSpy sizesChanged(modeselector, &ModeSelector::selectedModeChanged);

    qCDebug(KSCREEN_KCM) << "Modes:" << modeselector->modeSizes().count();
    modeselector->setSelectedSize(modeselector->modeSizes().count() - 1);
    QCOMPARE(refreshChanged.count(), 1);
    QCOMPARE(selectedChanged.count(), 1);
    modeselector->setSelectedRefreshRate(0);
    QCOMPARE(refreshChanged.count(), 2);
    QCOMPARE(selectedChanged.count(), 2);
    QCOMPARE(modeselector->refreshRates().count(), 2);
    qCDebug(KSCREEN_KCM) << "Rates:" << modeselector->refreshRates();
    modeselector->setSelectedRefreshRate(1);
    QCOMPARE(selectedChanged.count(), 3);
    modeselector->setSelectedRefreshRate(1);
    QCOMPARE(selectedChanged.count(), 1);

    QCOMPARE(modesChanged.count(), 0);

//     for (auto op : configmodule->mScreen->outputs()) {
//         qDebug() << "op :" << op->output()->name();
//     }
    auto qmltv = findQMLOutput(configmodule->mScreen->outputs(), "HDMI-2");
    QVERIFY(qmltv != nullptr);
    configmodule->focusedOutputChanged(qmltv);
    QCOMPARE(modesChanged.count(), 1);

    QCOMPARE(qmltv->modeSizes().count(), 17);
    printModes(modeselector);
    modeselector->setSelectedSize(1);
    modeselector->setSelectedRefreshRate(6);

    qCDebug(KSCREEN_KCM) << "rr:" << refreshChanged.count();
    QCOMPARE(refreshChanged.count(), 3);

    configmodule->focusedOutputChanged(qmlpanel);

    // This is all invalid, it shouldn't crash or emit changed signals
    const int rbefore = refreshChanged.count();
//     QCOMPARE(selectedChanged.count(), 5);
    QCOMPARE(refreshChanged.count(), rbefore);
    QCOMPARE(modesChanged.count(), 2);
    modeselector->setSelectedSize(0);
    QCOMPARE(refreshChanged.count(), rbefore + 1);
//     QCOMPARE(selectedChanged.count(), 6);
    modeselector->setSelectedRefreshRate(6);
    QCOMPARE(refreshChanged.count(), rbefore + 1);
//     QCOMPARE(selectedChanged.count(), 6);
    modeselector->setSelectedSize(9999);
    QCOMPARE(refreshChanged.count(), rbefore + 1);
//     QCOMPARE(selectedChanged.count(), 6);
    modeselector->setSelectedRefreshRate(modeselector->refreshRates().count());
}

}

QTEST_MAIN(KScreen::TestConfigModule)

#include "testconfigmodule.moc"
