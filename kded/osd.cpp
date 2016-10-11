/*
 *  Copyright 2014 (c) Martin Klapetek <mklapetek@kde.org>
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

#include <KScreen/Mode>

#include <QDBusConnection>
#include <QLoggingCategory>
#include <QTimer>
#include <QWindow>
#include <QStandardPaths>
#include <QDebug>
#include <QUrl>
#include <QQuickItem>
#include <KDeclarative/QmlObject>

namespace KScreen {

Osd::Osd(QObject *parent)
    : QObject(parent)
    , m_osdPath(QStandardPaths::locate(QStandardPaths::QStandardPaths::GenericDataLocation, QStringLiteral("kded_kscreen/qml/Osd.qml")))
{
    init();
}

Osd::~Osd()
{
}

bool Osd::setRootProperty(const char* name, const QVariant& value)
{
    return m_osdObject->rootObject()->setProperty(name, value);
}

void Osd::showOutputIdentifier(const KScreen::OutputPtr output)
{
    m_output = output;
    if (!init()) {
        return;
    }

    auto *rootObject = m_osdObject->rootObject();
    auto mode = output->currentMode();
    QSize realSize;
    if (output->isHorizontal()) {
        realSize = mode->size();
    } else {
        realSize = QSize(mode->size().height(), mode->size().width());
    }
    rootObject->setProperty("itemSource", QStringLiteral("OutputIdentifier.qml"));
    rootObject->setProperty("modeName", Utils::sizeToString(realSize));
    rootObject->setProperty("outputName", Utils::outputName(output));
    rootObject->setProperty("icon", QStringLiteral("preferences-desktop-display-randr"));
    showOsd();
}

void Osd::updatePosition()
{
    if (m_output == nullptr) {
        return;
    }

    auto *rootObject = m_osdObject->rootObject();

    const int dialogWidth = rootObject->property("width").toInt();
    const int dialogHeight = rootObject->property("height").toInt();
    const int relx = m_output->pos().x();
    const int rely = m_output->pos().y();
    const int pos_x = relx + (m_output->geometry().width() - dialogWidth) / 2;
    const int pos_y = rely + (m_output->geometry().height() - dialogHeight) / 2;

    rootObject->setProperty("x", pos_x);
    rootObject->setProperty("y", pos_y);
}


bool Osd::init()
{
    if (m_osdObject && m_osdObject->rootObject()) {
        return true;
    }

    if (m_osdPath.isEmpty()) {
        return false;
    }

    if (!m_osdObject) {
        m_osdObject = new KDeclarative::QmlObject(this);
    }

    m_osdObject->setSource(QUrl::fromLocalFile(m_osdPath));

    if (m_osdObject->status() != QQmlComponent::Ready) {
        qWarning() << "Failed to load OSD QML file" << m_osdPath;
        return false;
    }

    m_timeout = m_osdObject->rootObject()->property("timeout").toInt();

    if (!m_osdTimer) {
        m_osdTimer = new QTimer(this);
        m_osdTimer->setSingleShot(true);
        connect(m_osdTimer, &QTimer::timeout, this, &Osd::hideOsd);
    }

    return true;
}

void Osd::showProgress(const QString &icon, const int percent, const QString &additionalText)
{
    if (!init()) {
        return;
    }

    auto *rootObject = m_osdObject->rootObject();

    int value = qBound(0, percent, 100);
    rootObject->setProperty("osdValue", value);
    rootObject->setProperty("osdAdditionalText", additionalText);
    rootObject->setProperty("showingProgress", true);
    rootObject->setProperty("icon", icon);

    emit osdProgress(icon, value, additionalText);
    showOsd();
}

void Osd::showText(const QString &icon, const QString &text)
{
    if (!init()) {
        return;
    }

    auto *rootObject = m_osdObject->rootObject();

    rootObject->setProperty("showingProgress", false);
    rootObject->setProperty("osdValue", text);
    rootObject->setProperty("icon", icon);

    emit osdText(icon, text);
    showOsd();
}

void Osd::showOsd()
{
    m_osdTimer->stop();

    auto *rootObject = m_osdObject->rootObject();

    // if our OSD understands animating the opacity, do it;
    // otherwise just show it to not break existing lnf packages
    if (rootObject->property("animateOpacity").isValid()) {
        rootObject->setProperty("animateOpacity", false);
        rootObject->setProperty("opacity", 1);
        rootObject->setProperty("visible", true);
        rootObject->setProperty("animateOpacity", true);
        rootObject->setProperty("opacity", 0);
    } else {
        rootObject->setProperty("visible", true);
    }
    updatePosition();
    m_osdTimer->start(m_timeout);
}

void Osd::hideOsd()
{
    auto *rootObject = m_osdObject->rootObject();
    if (!rootObject) {
        return;
    }

    rootObject->setProperty("visible", false);

    // this is needed to prevent fading from "old" values when the OSD shows up
    rootObject->setProperty("osdValue", 0);
}

} // ns
