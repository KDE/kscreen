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
#include "xtouchscreen.h"

#include <KScreen/Config>
#include <KScreen/ConfigMonitor>
#include <KScreen/GetConfigOperation>
#include <KScreen/SetConfigOperation>
#include <KScreen/Output>

#include <QRotationSensor>
#include <QDebug>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>


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
                findTouchscreen();
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
            connect(m_sensor, &QOrientationSensor::readingChanged, this, &KScreenDoctor::updateRotation);
        }
        if (!m_autoRotate) {
            delete m_sensor;
            m_sensor = nullptr;
        }

        emit autoRotateChanged();
    }
}

void KScreenDoctor::updateRotation()
{
    if (m_sensor) {
        if (!m_sensor->reading()) return;
        m_currentRotation = m_sensor->reading()->orientation();
        QString o;
        switch (m_currentRotation) {
            case QOrientationReading::TopUp:
                o = "none";

/*
            None = 1,
            Left = 2,
            Inverted = 4,
            Right = 8
 */
                setRotation(0);
                // X.h #defines None, so we can't use this enum here, work around it
                // by using its numeric value. Yes, really ugly.
                //m_touchscreen->setRotation(static_cast<KScreen::Output::Rotation>(1));
                break;
            case QOrientationReading::TopDown:
                o = "inverted";
                setRotation(180);
                //m_touchscreen->setRotation(KScreen::Output::Inverted);
                break;
            case QOrientationReading::LeftUp:
                o = "left";
                setRotation(270);
                //m_touchscreen->setRotation(KScreen::Output::Left);
                break;
            case QOrientationReading::RightUp:
                o = "right";
                setRotation(90);
                //m_touchscreen->setRotation(KScreen::Output::Right);
                break;
            default:
                o = "other";
                qDebug() << "Weird Rotation, unhandled case.";
                return;
        }
        qDebug() << "Rotation is now: " << o;
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
                            // X.h #defines None, so we can't use this enum here, work around it
                            // by using its numeric value. Yes, really ugly.
                            output->setRotation(static_cast<KScreen::Output::Rotation>(1));
                        } else if (rotation == 90) {
                            output->setRotation(KScreen::Output::Right);
                        } else if (rotation == 180) {
                            output->setRotation(KScreen::Output::Inverted);
                        } else if (rotation == 270) {
                            output->setRotation(KScreen::Output::Left);
                        }
//                         auto *op = new KScreen::SetConfigOperation(config);
//                         op->exec();
                        m_touchscreen->setRotation(output->rotation());
                    }
                }
            }
    );
}

void KScreenDoctor::findTouchscreen()
{
    Display *display = XOpenDisplay(0);
    int nDevices = 0;
    XDeviceInfo *devices = XListInputDevices(display, &nDevices);
    for (int i = 0; i < nDevices; i++) {
        const char *name = devices[i].name;
        char *type = 0;
        if (devices[i].type) {
            type = XGetAtomName(display, devices[i].type);
        }
        if (QString::fromLocal8Bit(type) == QStringLiteral("TOUCH")) {
            m_touchscreen = new KScreen::XTouchscreen(this);
            m_touchscreen->setName(name);
            qDebug() << "Found touchscreen with NAME:" << name;
            XFree(type);
            break;
        }
        XFree(type);
    }
    XFreeDeviceList(devices);
    XCloseDisplay(display);
}


