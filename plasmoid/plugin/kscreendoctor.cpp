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

#include <KScreen/Config>
#include <KScreen/ConfigMonitor>
#include <KScreen/GetConfigOperation>
#include <KScreen/SetConfigOperation>
#include <KScreen/Output>

#include <QOrientationSensor>

#include <QDebug>


KScreenDoctor::KScreenDoctor(QObject *parent)
    : QObject(parent)
{
    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
            this, [=] (KScreen::ConfigOperation* op) {
                if (op->hasError()) {
                    return;
                }

                m_monitoredConfig = qobject_cast<KScreen::GetConfigOperation*>(op)->config();
                qDebug() << "Config" << m_monitoredConfig.data() << "is ready";
                KScreen::ConfigMonitor::instance()->addConfig(m_monitoredConfig);
                updateOutputs();
            }
    );
}

void KScreenDoctor::updateOutputs()
{
    m_outputNames.clear();
    m_outputNames << QStringLiteral("Fake");
    Q_ASSERT(m_monitoredConfig);
    for (auto output: m_monitoredConfig->outputs()) {
        if (output->isConnected()) {
            m_outputNames << output->name();
        }

    }
    qDebug() << "OUTPUTS:" << m_outputNames;
    emit outputNamesChanged();

    // Current output still valid?
    if (m_currentOutput.isEmpty() || !m_outputNames.contains(m_currentOutput)) {
        m_currentOutput = m_outputNames.at(0);
        // Perhaps the primary output isn't the first, in that case, select the primary instead
        for (auto output: m_monitoredConfig->outputs()) {
            if (output->isConnected() && output->isPrimary()) {
                m_currentOutput = output->name();
            }
        }
        emit currentOutputChanged();
    }
    qDebug() << "CURRENT:" << m_currentOutput;
}


QStringList KScreenDoctor::outputNames() const
{
    return m_outputNames;
}

QString KScreenDoctor::currentOutput() const
{
    return m_currentOutput;
}

void KScreenDoctor::setCurrentOutput(const QString &currentOutput)
{
    if (m_currentOutput != currentOutput) {
        m_currentOutput = currentOutput;
            qDebug() << "Current output is now: " << m_currentOutput;
        emit currentOutputChanged();
    }
}

bool KScreenDoctor::autoRotate() const
{
    return m_autoRotate;
}

void KScreenDoctor::setAutoRotate(bool rotate)
{
    qDebug() << "Autorotate is now" << rotate;
    if (rotate != m_autoRotate) {
        m_autoRotate = rotate;

        if (m_autoRotate && !m_sensor) {
            m_sensor = new QOrientationSensor(this);
            m_sensor->start();
            connect(m_sensor, &QOrientationSensor::readingChanged, this, &KScreenDoctor::updateOrientation);
        }

        emit autoRotateChanged();
    }
}

void KScreenDoctor::updateOrientation()
{
    if (m_sensor) {
        if (!m_sensor->reading()) return;
        m_currentOrientation = m_sensor->reading()->orientation();
        QString o;
        switch (m_currentOrientation) {
            case QOrientationReading::TopUp:
                o = "normal";
                break;
            case QOrientationReading::TopDown:
                o = "bottom-up";
                break;
            case QOrientationReading::LeftUp:
                o = "left-up";
                break;
            case QOrientationReading::RightUp:
                o = "right-up";
                break;
            default:
                o = "other";
                return;
        }
        qDebug() << "Orientation is now: " << o;
    }
}


int KScreenDoctor::currentOutputRotation() const
{
    return 0;
}

void KScreenDoctor::setRotation(int rotation)
{
    qDebug() << "Setting output" << m_currentOutput << "to" << rotation;
    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
            this, [=] (KScreen::ConfigOperation* op) {
                if (op->hasError()) {
                    return;
                }

                auto config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();
                KScreen::OutputPtr currentOutput = nullptr;
                for (auto output: config->outputs()) {
                    if (output->name() == m_currentOutput) {
                        qDebug() << "Setting output rotation on " << output->name();
                        if (rotation == 0) {
                            output->setRotation(KScreen::Output::None);
                        } else if (rotation == 90) {
                            output->setRotation(KScreen::Output::Right);
                        } else if (rotation == 180) {
                            output->setRotation(KScreen::Output::Inverted);
                        } else if (rotation == 270) {
                            output->setRotation(KScreen::Output::Left);
                        }
                        auto *op = new KScreen::SetConfigOperation(config);
                        op->exec();
                    }
                }
            }
    );
}
