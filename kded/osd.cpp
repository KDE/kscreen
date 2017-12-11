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
    , m_osdObject(new KDeclarative::QmlObject(this))
{
    const QString &osdPath = QStandardPaths::locate(QStandardPaths::QStandardPaths::GenericDataLocation, QStringLiteral("kded_kscreen/qml/Osd.qml"));
    if (osdPath.isEmpty()) {
        qCWarning(KSCREEN_KDED) << "Failed to find OSD QML file" << osdPath;
    }

    m_osdObject->setSource(QUrl::fromLocalFile(osdPath));

    if (m_osdObject->status() != QQmlComponent::Ready) {
        qCWarning(KSCREEN_KDED) << "Failed to load OSD QML file" << osdPath;
        return;
    }

    m_timeout = m_osdObject->rootObject()->property("timeout").toInt();
    m_osdTimer = new QTimer(this);
    m_osdTimer->setSingleShot(true);
    connect(m_osdTimer, &QTimer::timeout, this, &Osd::hideOsd);
}

Osd::~Osd()
{
}

void Osd::showGenericOsd(const QString &icon, const QString &text)
{
    m_outputGeometry = m_output->geometry();
    auto *rootObject = m_osdObject->rootObject();
    rootObject->setProperty("itemSource", QStringLiteral("OsdItem.qml"));
    rootObject->setProperty("infoText", text);
    rootObject->setProperty("icon", icon);

    showOsd();
}

void Osd::showOutputIdentifier(const KScreen::OutputPtr output)
{
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

void Osd::updatePosition()
{
    if (!m_outputGeometry.isValid()) {
        return;
    }

    auto *rootObject = m_osdObject->rootObject();

    const int dialogWidth = rootObject->property("width").toInt();
    const int dialogHeight = rootObject->property("height").toInt();
    const int relx = m_outputGeometry.x();
    const int rely = m_outputGeometry.y();
    const int pos_x = relx + (m_outputGeometry.width() - dialogWidth) / 2;
    const int pos_y = rely + (m_outputGeometry.height() - dialogHeight) / 2;

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
    updatePosition();
    if (m_timeout > 0) {
        m_osdTimer->start(m_timeout);
    }
}

void Osd::hideOsd()
{
    auto *rootObject = m_osdObject->rootObject();
    if (!rootObject) {
        return;
    }
    rootObject->setProperty("visible", false);
}

