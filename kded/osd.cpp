/*
 *  Copyright 2014 Martin Klapetek <mklapetek@kde.org>
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "osd.h"
#include "utils.h"
#include "kscreen_daemon_debug.h"

#include <KScreen/Mode>

#include <QLoggingCategory>
#include <QTimer>
#include <QStandardPaths>
#include <KDeclarative/QmlObject>


using namespace KScreen;

Osd::Osd(const KScreen::OutputPtr output, QObject *parent)
    : QObject(parent)
    , m_output(output)
{
    connect(output.data(), &KScreen::Output::isConnectedChanged,
            this, &Osd::onOutputAvailabilityChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            this, &Osd::onOutputAvailabilityChanged);
    connect(output.data(), &KScreen::Output::currentModeIdChanged,
            this, &Osd::updatePosition);
    connect(output.data(), &KScreen::Output::destroyed,
            this, &Osd::hideOsd);
}

Osd::~Osd()
{
}

bool Osd::initOsd()
{
    if (m_osdObject) {
        return true;
    }

    const QString &osdPath = QStandardPaths::locate(QStandardPaths::QStandardPaths::GenericDataLocation, QStringLiteral("kded_kscreen/qml/Osd.qml"));
    if (osdPath.isEmpty()) {
        qCWarning(KSCREEN_KDED) << "Failed to find OSD QML file" << osdPath;
        return false;
    }

    m_osdObject = new KDeclarative::QmlObject(this);
    m_osdObject->setSource(QUrl::fromLocalFile(osdPath));

    if (m_osdObject->status() != QQmlComponent::Ready) {
        qCWarning(KSCREEN_KDED) << "Failed to load OSD QML file" << osdPath;
        delete m_osdObject;
        m_osdObject = nullptr;
        return false;
    }

    m_timeout = m_osdObject->rootObject()->property("timeout").toInt();
    m_osdTimer = new QTimer(this);
    m_osdTimer->setSingleShot(true);
    connect(m_osdTimer, &QTimer::timeout, this, &Osd::hideOsd);
    return true;

}

void Osd::showGenericOsd(const QString &icon, const QString &text)
{
    if (!initOsd()) {
        return;
    }

    m_outputGeometry = m_output->geometry();
    auto *rootObject = m_osdObject->rootObject();
    rootObject->setProperty("itemSource", QStringLiteral("OsdItem.qml"));
    rootObject->setProperty("infoText", text);
    rootObject->setProperty("icon", icon);

    showOsd();
}

void Osd::showOutputIdentifier(const KScreen::OutputPtr output)
{
    if (!initOsd()) {
        return;
    }

    m_outputGeometry = output->geometry();

    auto *rootObject = m_osdObject->rootObject();
    auto mode = output->currentMode();
    QSize realSize = mode->size();
    if (!output->isHorizontal()) {
        realSize.transpose();
    }
    rootObject->setProperty("itemSource", QStringLiteral("OutputIdentifier.qml"));
    rootObject->setProperty("modeName", Utils::sizeToString(realSize));
    rootObject->setProperty("outputName", Utils::outputName(output));
    showOsd();
}

void Osd::showActionSelector()
{
    if (!initOsd()) {
        return;
    }

    m_outputGeometry = m_output->geometry();
    auto *rootObject = m_osdObject->rootObject();
    rootObject->setProperty("itemSource", QStringLiteral("OsdSelector.qml"));
    rootObject->setProperty("timeout", 0);
    rootObject->setProperty("outputOnly", false);
    auto osdItem = rootObject->property("osdItem").value<QObject*>();
    connect(osdItem, SIGNAL(clicked(int)),
            this, SLOT(onOsdActionSelected(int)));
    m_timeout = 0; // no timeout for this one

    showOsd();
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

void Osd::updatePosition()
{
    if (!initOsd()) {
        return;
    }

    const auto geometry = m_output->geometry();
    if (!geometry.isValid()) {
        hideOsd();
    }

    auto *rootObject = m_osdObject->rootObject();

    const int dialogWidth = rootObject->property("width").toInt();
    const int dialogHeight = rootObject->property("height").toInt();
    const int relx = geometry.x();
    const int rely = geometry.y();
    const int pos_x = relx + (geometry.width() - dialogWidth) / 2;
    const int pos_y = rely + (geometry.height() - dialogHeight) / 2;

    rootObject->setProperty("x", pos_x);
    rootObject->setProperty("y", pos_y);
}

void Osd::showOsd()
{
    m_osdTimer->stop();

    auto *rootObject = m_osdObject->rootObject();

    // only animate on X11, wayland plugin doesn't support this and
    // pukes loads of warnings into our logs
    if (qGuiApp->platformName() == QLatin1String("xcb")) {
        if (rootObject->property("timeout").toInt() > 0) {
            rootObject->setProperty("animateOpacity", false);
            rootObject->setProperty("opacity", 1);
            rootObject->setProperty("visible", true);
            rootObject->setProperty("animateOpacity", true);
            rootObject->setProperty("opacity", 0);
        } else {
            rootObject->setProperty("visible", true);
        }
    } else {
        rootObject->setProperty("visible", true);
    }
    QTimer::singleShot(0, this, &Osd::updatePosition);
    if (m_timeout > 0) {
        m_osdTimer->start(m_timeout);
    }
}

void Osd::hideOsd()
{
    if (!initOsd()) {
        return;
    }

    auto *rootObject = m_osdObject->rootObject();
    if (!rootObject) {
        return;
    }
    rootObject->setProperty("visible", false);
}

