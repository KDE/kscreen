/*
    SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "../../common/globals.h"
#include "../../common/osdaction.h"
#include "../../kded/config.h"

#include <QObject>
#include <QStandardPaths>
#include <QTest>

#include <KScreen/Config>
#include <KScreen/EDID>
#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/Screen>

#include <kscreen/backendmanager_p.h>
#include <kscreen/config.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

using namespace KScreen;

class TestOsdActions : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testSwitchToExternal();
    void testSwitchToInternal();
    void testClone();
    void testExtendLeft();
    void testExtendRight();

    void testSwitchToExternalRotated();
    void testSwitchToInternalRotated();
    void testCloneRotated();
    void testExtendLeftRotated();
    void testExtendRightRotated();

private:
    KScreen::ConfigPtr loadConfig(const QByteArray &fileName);
    KScreen::ConfigPtr reloadConfig();
    QPair<OutputPtr, OutputPtr> findScreens(KScreen::ConfigPtr);

    QTemporaryDir m_temporaryDir;
};

void TestOsdActions::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    qputenv("KSCREEN_LOGGING", "false");
    setenv("KSCREEN_BACKEND", "Fake", 1);
}

KScreen::ConfigPtr TestOsdActions::loadConfig(const QByteArray &fileName)
{
    KScreen::BackendManager::instance()->shutdownBackend();

    QByteArray path(TEST_DATA "configs/" + fileName);
    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), path}});
    return reloadConfig();
}

KScreen::ConfigPtr TestOsdActions::reloadConfig()
{
    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation;
    if (!op->exec()) {
        qWarning() << op->errorString();
        return ConfigPtr();
    }
    return op->config();
}

QPair<OutputPtr, OutputPtr> TestOsdActions::findScreens(ConfigPtr config)
{
    const auto outputs = config->outputs();
    auto internalIt = std::find_if(outputs.cbegin(), outputs.cend(), [](const auto &output) {
        return output->type() == KScreen::Output::Type::Panel;
    });
    if (internalIt == outputs.end()) {
        internalIt = outputs.begin();
    }
    const OutputPtr &internal = *internalIt;
    const OutputPtr &external = *std::find_if(outputs.cbegin(), outputs.cend(), [&internal](const auto &output) {
        return output != internal;
    });
    return {internal, external};
}

void TestOsdActions::testSwitchToExternal()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::SwitchToExternal);
    QVERIFY(op);
    QVERIFY(op->exec());

    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), false);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void TestOsdActions::testSwitchToInternal()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::SwitchToInternal);
    QVERIFY(op);
    QVERIFY(op->exec());
    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(external->isEnabled(), false);
}

void TestOsdActions::testClone()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::Clone);
    QVERIFY(op);
    QVERIFY(op->exec());
    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void TestOsdActions::testExtendLeft()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::ExtendLeft);
    QVERIFY(op);
    QVERIFY(op->exec());
    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), true);
    // the display is not in the max vertical resolution, not the best horizontal
    QCOMPARE(laptop->pos(), QPoint(1600, 0));
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void TestOsdActions::testExtendRight()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("laptopAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::ExtendRight);
    QVERIFY(op);
    QVERIFY(op->exec());
    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

void TestOsdActions::testSwitchToExternalRotated()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("steamdeckAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::SwitchToExternal);
    QVERIFY(op);
    QVERIFY(op->exec());

    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), false);

    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->rotation(), Output::None);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void TestOsdActions::testSwitchToInternalRotated()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("steamdeckAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::SwitchToInternal);
    QVERIFY(op);
    QVERIFY(op->exec());
    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(laptop->rotation(), Output::Right);
    QCOMPARE(external->isEnabled(), false);
}

void TestOsdActions::testCloneRotated()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("steamdeckAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::Clone);
    // we won't find a matching mode,this will return null
    QVERIFY(!op);
    auto [laptop, external] = findScreens(reloadConfig());

    // things should be unchanged
    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(laptop->rotation(), Output::Right);
    QCOMPARE(external->isEnabled(), false);
}

void TestOsdActions::testExtendLeftRotated()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("steamdeckAndExternal.json");
    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::ExtendLeft);
    QVERIFY(op);
    QVERIFY(op->exec());
    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), true);
    // the display is not in the max vertical resolution, not the best horizontal
    QCOMPARE(laptop->pos(), QPoint(1600, 0));
    QCOMPARE(laptop->rotation(), Output::Right);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(0, 0));
}

void TestOsdActions::testExtendRightRotated()
{
    const KScreen::ConfigPtr currentConfig = loadConfig("steamdeckAndExternal.json");
    qDebug() << currentConfig->output(1);

    QVERIFY(currentConfig);

    KScreen::SetConfigOperation *op = KScreen::OsdAction::applyAction(currentConfig, KScreen::OsdAction::Action::ExtendRight);
    QVERIFY(op);
    QVERIFY(op->exec());
    auto [laptop, external] = findScreens(reloadConfig());

    QCOMPARE(laptop->isEnabled(), true);
    QCOMPARE(laptop->pos(), QPoint(0, 0));
    QCOMPARE(laptop->rotation(), Output::Right);
    QCOMPARE(external->isEnabled(), true);
    QCOMPARE(external->pos(), QPoint(1280, 0));
}

QTEST_MAIN(TestOsdActions);

#include "osdactions.moc"
