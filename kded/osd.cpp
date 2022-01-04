/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osd.h"

#include "kscreen_daemon_debug.h"

#include "../common/utils.h"

#include <KScreen/Mode>

#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>

#include <KDeclarative/QmlObjectSharedEngine>

using namespace KScreen;

Osd::Osd(const KScreen::OutputPtr &output, QObject *parent)
    : QObject(parent)
    , m_output(output)
{
    connect(output.data(), &KScreen::Output::isConnectedChanged, this, &Osd::onOutputAvailabilityChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged, this, &Osd::onOutputAvailabilityChanged);
    connect(output.data(), &KScreen::Output::destroyed, this, &Osd::hideOsd);
}

Osd::~Osd()
{
}

void Osd::showActionSelector()
{
    if (!m_osdActionSelector) {
        const QString osdPath = QStandardPaths::locate(QStandardPaths::QStandardPaths::GenericDataLocation, QStringLiteral("kded_kscreen/qml/OsdSelector.qml"));
        if (osdPath.isEmpty()) {
            qCWarning(KSCREEN_KDED) << "Failed to find action selector OSD QML file" << osdPath;
            return;
        }
        m_osdActionSelector = new KDeclarative::QmlObjectSharedEngine(this);
        m_osdActionSelector->setSource(QUrl::fromLocalFile(osdPath));

        if (m_osdActionSelector->status() != QQmlComponent::Ready) {
            qCWarning(KSCREEN_KDED) << "Failed to load OSD QML file" << osdPath;
            delete m_osdActionSelector;
            m_osdActionSelector = nullptr;
            return;
        }

        auto *rootObject = m_osdActionSelector->rootObject();
        connect(rootObject, SIGNAL(clicked(int)), this, SLOT(onOsdActionSelected(int)));
    }
    if (auto *rootObject = m_osdActionSelector->rootObject()) {
        // On wayland, we use m_output to set an action on OSD position
        if (qGuiApp->platformName() == QLatin1String("wayland")) {
            rootObject->setProperty("screenGeometry", m_output->geometry());
        }
        rootObject->setProperty("visible", true);
    } else {
        qCWarning(KSCREEN_KDED) << "Could not get root object for action selector.";
    }
}

void Osd::onOsdActionSelected(int action)
{
    Q_EMIT osdActionSelected(static_cast<OsdAction::Action>(action));
    hideOsd();
}

void Osd::onOutputAvailabilityChanged()
{
    if (!m_output || !m_output->isConnected() || !m_output->isEnabled() || !m_output->currentMode()) {
        hideOsd();
    }
}

void Osd::hideOsd()
{
    if (m_osdActionSelector) {
        if (auto *rootObject = m_osdActionSelector->rootObject()) {
            rootObject->setProperty("visible", false);
        }
    }
}
