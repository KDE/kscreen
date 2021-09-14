/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "output_identifier.h"

#include "../common/utils.h"

#include <kscreen/output.h>

#include <QQuickItem>
#include <QStandardPaths>
#include <QTimer>

#include <KDeclarative/kdeclarative/qmlobject.h>
#include <PlasmaQuick/Dialog>

#define QML_PATH "kpackage/kcms/kcm_kscreen/contents/ui/"

OutputIdentifier::OutputIdentifier(KScreen::ConfigPtr config, QObject *parent)
    : QObject(parent)
{
    const QString qmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral(QML_PATH "OutputIdentifier.qml"));

    for (const auto &output : config->connectedOutputs()) {
        if (!output->currentMode()) {
            continue;
        }

        const KScreen::ModePtr mode = output->currentMode();
        auto *view = new PlasmaQuick::Dialog();

        auto qmlObject = new KDeclarative::QmlObject(view);
        qmlObject->setSource(QUrl::fromLocalFile(qmlPath));
        qmlObject->completeInitialization();

        auto rootObj = qobject_cast<QQuickItem *>(qmlObject->rootObject());

        view->setMainItem(rootObj);
        view->setFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
        view->setBackgroundHints(PlasmaQuick::Dialog::NoBackground);
        view->installEventFilter(this);

        if (!rootObj) {
            delete view;
            continue;
        }

        QSize deviceSize, logicalSize;
        if (output->isHorizontal()) {
            deviceSize = mode->size();
        } else {
            deviceSize = QSize(mode->size().height(), mode->size().width());
        }
        if (config->supportedFeatures() & KScreen::Config::Feature::PerOutputScaling) {
            // Scale adjustment is not needed on Wayland, we use logical size.
            logicalSize = output->logicalSize().toSize();
        } else {
            logicalSize = deviceSize / view->effectiveDevicePixelRatio();
        }
        rootObj->setProperty("outputName", Utils::outputName(output));
        rootObj->setProperty("modeName", Utils::sizeToString(deviceSize));
        view->setProperty("screenSize", QRect(output->pos(), logicalSize));
        m_views << view;
    }

    for (auto *view : m_views) {
        view->show();
    }
    QTimer::singleShot(2500, this, &OutputIdentifier::identifiersFinished);
}

OutputIdentifier::~OutputIdentifier()
{
    qDeleteAll(m_views);
}

bool OutputIdentifier::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        if (m_views.contains(qobject_cast<PlasmaQuick::Dialog *>(object))) {
            QResizeEvent *e = static_cast<QResizeEvent *>(event);
            const QRect screenSize = object->property("screenSize").toRect();
            QRect geometry(QPoint(0, 0), e->size());
            geometry.moveCenter(screenSize.center());
            static_cast<PlasmaQuick::Dialog *>(object)->setGeometry(geometry);
        }
    }
    return QObject::eventFilter(object, event);
}
