/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osd.h"

#include <KScreen/Mode>

#include <LayerShellQt/Window>

#include <KWindowSystem>

#include <Plasma/Plasma>

#include <QCursor>
#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickView>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>

using namespace KScreen;

Osd::Osd(const KScreen::OutputPtr &output, QObject *parent)
    : QObject(parent)
    , m_output(output)
{
    connect(output.data(), &KScreen::Output::isConnectedChanged, this, &Osd::onOutputAvailabilityChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged, this, &Osd::onOutputAvailabilityChanged);
    Plasma::setupPlasmaStyle(&m_engine);
}

Osd::~Osd()
{
}

void Osd::showActionSelector()
{
    if (!m_osdActionSelector) {
        m_osdActionSelector = std::make_unique<QQuickView>(&m_engine, nullptr);
        m_osdActionSelector->setInitialProperties({{QLatin1String("actions"), QVariant::fromValue(OsdAction::availableActions())}});
        m_osdActionSelector->loadFromModule("org.kde.kscreen.osd", "OsdSelector");
        m_osdActionSelector->setColor(Qt::transparent);
        m_osdActionSelector->setFlag(Qt::FramelessWindowHint);

        if (m_osdActionSelector->status() != QQuickView::Ready) {
            qWarning() << "Failed to load OSD QML file";
            m_osdActionSelector.reset();
            return;
        }

        auto rootObject = m_osdActionSelector->rootObject();
        connect(rootObject, SIGNAL(clicked(int)), this, SLOT(onOsdActionSelected(int)));
    }

    auto screen = qGuiApp->screenAt(m_output->pos());
    if (!screen) {
        screen = qGuiApp->primaryScreen();
    }

    if (m_osdActionSelector->isVisible()) {
        QMetaObject::invokeMethod(m_osdActionSelector->rootObject(), "moveRight");
    }

    auto layerWindow = LayerShellQt::Window::get(m_osdActionSelector.get());
    layerWindow->setScope(QStringLiteral("on-screen-display"));
    layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
    layerWindow->setAnchors({});
    layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
    m_osdActionSelector->setScreen(screen);
    m_osdActionSelector->setVisible(true);
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
        m_osdActionSelector->setVisible(false);
    }
}

#include "moc_osd.cpp"
