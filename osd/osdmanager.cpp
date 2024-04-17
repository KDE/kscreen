/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osdmanager.h"
#include "osd.h"
#include "osdserviceadaptor.h"

#include <KScreen/Config>
#include <KScreen/EDID>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>

#include <QDBusConnection>
#include <QDBusMessage>

#include <QQmlEngine>

namespace KScreen
{
OsdManager::OsdManager(QObject *parent)
    : QObject(parent)
    , m_cleanupTimer(new QTimer(this))
{
    qmlRegisterUncreatableMetaObject(OsdAction::staticMetaObject, "org.kde.KScreen", 1, 0, "OsdAction", QStringLiteral("Can't create OsdAction"));
    new OsdServiceAdaptor(this);

    // free up memory when the osd hasn't been used for more than 1 minute
    m_cleanupTimer->setInterval(60000);
    m_cleanupTimer->setSingleShot(true);
    connect(m_cleanupTimer, &QTimer::timeout, this, [this]() {
        quit();
    });
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/kde/kscreen/osdService"), this, QDBusConnection::ExportAdaptors);
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.kscreen.osdService"));
}

void OsdManager::hideOsd()
{
    for (auto osd : m_osds) {
        osd->hideOsd();
    }
    if (m_pendingConfigOperations.empty()) {
        quit();
    }
}

void OsdManager::quit()
{
    qDeleteAll(m_osds);
    m_osds.clear();
    qApp->quit();
}

OsdManager::~OsdManager()
{
}

void OsdManager::setConfigOperationFinished(ConfigOperation *operation)
{
    if (m_pendingConfigOperations.empty() && std::ranges::none_of(m_osds, &Osd::visible)) {
        quit();
    }
}

void OsdManager::showActionSelector()
{
    auto getConfigOperation = new KScreen::GetConfigOperation();
    m_pendingConfigOperations.push_back(getConfigOperation);
    connect(getConfigOperation, &KScreen::GetConfigOperation::finished, this, [this](const KScreen::ConfigOperation *op) {
        m_pendingConfigOperations.removeOne(op);
        if (op->hasError()) {
            qWarning() << op->errorString();
            return;
        }

        // Show selector on at most one of the enabled screens
        const auto outputs = op->config()->outputs();
        KScreen::OutputPtr osdOutput;
        for (const auto &output : outputs) {
            if (!output->isConnected() || !output->isEnabled() || !output->currentMode()) {
                continue;
            }

            // Prefer laptop screen
            if (output->type() == KScreen::Output::Panel) {
                osdOutput = output;
                break;
            }

            // Fallback to primary
            if (output->isPrimary()) {
                osdOutput = output;
                break;
            }
        }
        // no laptop or primary screen, just take the first usable one
        if (!osdOutput) {
            for (const auto &output : outputs) {
                if (output->isConnected() && output->isEnabled() && output->currentMode()) {
                    osdOutput = output;
                    break;
                }
            }
        }

        if (!osdOutput) {
            return;
        }

        KScreen::Osd *osd = nullptr;
        if (m_osds.contains(osdOutput->name())) {
            osd = m_osds.value(osdOutput->name());
        } else {
            osd = new KScreen::Osd(osdOutput, this);
            m_osds.insert(osdOutput->name(), osd);
            connect(osd, &Osd::osdActionSelected, this, [this, cfg = op->config()](OsdAction::Action action) {
                auto configOperation = OsdAction::applyAction(cfg, action);
                if (configOperation) {
                    connect(configOperation, &ConfigOperation::finished, this, &OsdManager::setConfigOperationFinished);
                } else {
                    quit();
                }
            });
        }

        osd->showActionSelector();
        m_cleanupTimer->start();
    });
}
}

#include "moc_osdmanager.cpp"
