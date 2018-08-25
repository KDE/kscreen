/*
 * Copyright (c) 2018 Kai Uwe Broulik <kde@broulik.de>
 *                    Work sponsored by the LiMux project of
 *                    the city of Munich.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kscreenapplet.h"

#include <QQmlEngine> // for qmlRegisterType
#include <QMetaEnum>

#include <QDBusConnection>
#include <QDBusMessage>

#include <KScreen/Config>
#include <KScreen/ConfigMonitor>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>

#include "../kded/osdaction.h"

#include <algorithm>

KScreenApplet::KScreenApplet(QObject *parent, const QVariantList &data)
    : Plasma::Applet(parent, data)
{

}

KScreenApplet::~KScreenApplet() = default;

void KScreenApplet::init()
{
    qmlRegisterSingletonType<KScreen::OsdAction>("org.kde.private.kscreen", 1, 0, "OsdAction", [](QQmlEngine *, QJSEngine *) -> QObject* {
        return new KScreen::OsdAction();
    });

    connect(new KScreen::GetConfigOperation(KScreen::GetConfigOperation::NoEDID), &KScreen::ConfigOperation::finished,
            this, [this](KScreen::ConfigOperation *op) {
                m_screenConfiguration = qobject_cast<KScreen::GetConfigOperation *>(op)->config();

                KScreen::ConfigMonitor::instance()->addConfig(m_screenConfiguration);
                connect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged, this, &KScreenApplet::checkOutputs);

                checkOutputs();
    });
}

int KScreenApplet::connectedOutputCount() const
{
    return m_connectedOutputCount;
}

void KScreenApplet::applyLayoutPreset(Action action)
{
    const QMetaEnum actionEnum = QMetaEnum::fromType<KScreen::OsdAction::Action>();
    Q_ASSERT(actionEnum.isValid());

    const QString presetName = QString::fromLatin1(actionEnum.valueToKey(action));
    if (presetName.isEmpty()) {
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.kded5"),
        QStringLiteral("/modules/kscreen"),
        QStringLiteral("org.kde.KScreen"),
        QStringLiteral("applyLayoutPreset")
    );

    msg.setArguments({presetName});

    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

void KScreenApplet::checkOutputs()
{
    if (!m_screenConfiguration) {
        return;
    }

    const int oldConnectedOutputCount = m_connectedOutputCount;

    const auto outputs = m_screenConfiguration->outputs();
    m_connectedOutputCount = std::count_if(outputs.begin(), outputs.end(), [](const KScreen::OutputPtr &output) {
        return output->isConnected();
    });

    if (m_connectedOutputCount != oldConnectedOutputCount) {
        emit connectedOutputCountChanged();
    }
}

K_EXPORT_PLASMA_APPLET_WITH_JSON(kscreen, KScreenApplet, "metadata.json")

#include "kscreenapplet.moc"
