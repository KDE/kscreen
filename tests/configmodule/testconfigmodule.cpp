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
}

void TestConfigModule::loadConfigModule()
{
    const ConfigPtr currentConfig = loadConfig("yogaAndTv.json");
    QVERIFY(!currentConfig.isNull());
    QVERIFY(currentConfig->isValid());

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
    const ConfigPtr currentConfig = loadConfig("yogaAndTv.json");
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

    QSignalSpy modesChanged(modeselector, &ModeSelector::modesChanged);
    QSignalSpy selectedChanged(modeselector, &ModeSelector::selectedModeChanged);
    QSignalSpy refreshChanged(modeselector, &ModeSelector::refreshRatesChanged);
    QSignalSpy sizesChanged(modeselector, &ModeSelector::selectedModeChanged);

    modeselector->setSelectedSize(modeselector->modeSizes().count() - 1);
    QCOMPARE(refreshChanged.count(), 1);
    QCOMPARE(selectedChanged.count(), 1);
    QCOMPARE(modeselector->refreshLabelMin(), QStringLiteral("48.00"));
    QCOMPARE(modeselector->refreshLabelMax(), QStringLiteral("60.00"));

    modeselector->setSelectedRefreshRate(0);
    QCOMPARE(refreshChanged.count(), 1);
    QCOMPARE(selectedChanged.count(), 2);
    QCOMPARE(modeselector->refreshRates().count(), 2);
    modeselector->setSelectedRefreshRate(1);
    QCOMPARE(selectedChanged.count(), 3);
    modeselector->setSelectedRefreshRate(1);
    QCOMPARE(selectedChanged.count(), 3);

    QCOMPARE(modesChanged.count(), 0);

    auto qmltv = findQMLOutput(configmodule->mScreen->outputs(), "HDMI-2");
    QVERIFY(qmltv != nullptr);
    configmodule->focusedOutputChanged(qmltv);
    QCOMPARE(modesChanged.count(), 1);
    QCOMPARE(qmltv->modeSizes().count(), 17);
    QCOMPARE(modeselector->modeLabelMin(), QStringLiteral("720x400"));
    QCOMPARE(modeselector->modeLabelMax(), QStringLiteral("1920x1080"));
    QCOMPARE(modeselector->refreshLabelMin(), QStringLiteral("23.98"));
    QCOMPARE(modeselector->refreshLabelMax(), QStringLiteral("60.00"));
    //printModes(modeselector);

    modeselector->setSelectedSize(1);
    modeselector->setSelectedRefreshRate(6);
    QCOMPARE(modesChanged.count(), 1);
    QCOMPARE(refreshChanged.count(), 3);
    QCOMPARE(modeselector->refreshLabelMin(), QStringLiteral("59.94"));
    QCOMPARE(modeselector->refreshLabelMax(), QStringLiteral("120.00"));

    // change the focused output, make sure we get update signals
    configmodule->focusedOutputChanged(findQMLOutput(configmodule->mScreen->outputs(), "eDP-1"));

    QCOMPARE(refreshChanged.count(), 4);
    QCOMPARE(selectedChanged.count(), 4);
    QCOMPARE(modesChanged.count(), 2);
    QCOMPARE(modeselector->modeSizes().count(), 33);
    QCOMPARE(modeselector->modeLabelMin(), QStringLiteral("320x240"));
    QCOMPARE(modeselector->modeLabelMax(), QStringLiteral("2560x1440"));

    const int rbefore = refreshChanged.count();

    QCOMPARE(refreshChanged.count(), rbefore);

    modeselector->setSelectedSize(0);
    QCOMPARE(refreshChanged.count(), rbefore + 1);
    QCOMPARE(selectedChanged.count(), 5);

    // Some invalid calls, make sure no unnecessary signals arrive
    modeselector->setSelectedRefreshRate(6);
    QCOMPARE(refreshChanged.count(), rbefore + 1);
    QCOMPARE(selectedChanged.count(), 5);

    modeselector->setSelectedSize(9999);
    QCOMPARE(refreshChanged.count(), rbefore + 1);
    QCOMPARE(selectedChanged.count(), 5);

    // this is 1 out of bounds, it's going to be ignored
    modeselector->setSelectedRefreshRate(modeselector->refreshRates().count());
    QCOMPARE(refreshChanged.count(), rbefore + 1);
    QCOMPARE(selectedChanged.count(), 5);
}

}

QTEST_MAIN(KScreen::TestConfigModule)

#include "testconfigmodule.moc"
