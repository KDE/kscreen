/*
    Copyright 2017 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kscreendoctor.h"

#include <QDebug>


KScreenDoctor::KScreenDoctor(QObject *parent)
    : QObject(parent)
{
//     connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
//             this, &KScreenDaemon::configReady);
// }
//
// void KScreenDaemon::configReady(KScreen::ConfigOperation* op)
// {
//     if (op->hasError()) {
//         return;
//     }
//
//     m_monitoredConfig = qobject_cast<KScreen::GetConfigOperation*>(op)->config();
//     qCDebug(KSCREEN_KDED) << "Config" << m_monitoredConfig.data() << "is ready";
//     KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig);

}

QStringList KScreenDoctor::outputNames() const
{
    return QStringList() << QStringLiteral("eDP-1") << QStringLiteral("4K Monster");
}

QString KScreenDoctor::currentOutput() const
{
    return m_currentOutput;
}

void KScreenDoctor::setCurrentOutput(const QString &currentOutput)
{
    if (m_currentOutput != currentOutput) {
        m_currentOutput = currentOutput;
        emit currentOutputChanged();
    }
}

int KScreenDoctor::currentOutputRotation() const
{
    return 0;
}

void KScreenDoctor::setRotation(const QString &outputName, int rotation)
{
    qDebug() << "Setting output" << outputName << "to" << rotation;
}
