/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "output_identifier.h"

#include "../common/utils.h"

#include <kscreen/output.h>

#include <QStandardPaths>
#include <QTimer>
#include <QQuickItem>
#include <QQuickView>

#define QML_PATH "kpackage/kcms/kcm_kscreen/contents/ui/"

OutputIdentifier::OutputIdentifier(KScreen::ConfigPtr config, QObject *parent)
    : QObject(parent)
{

    const QString qmlPath =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                   QStringLiteral(QML_PATH
                                                  "OutputIdentifier.qml"));

    for (const auto &output : config->connectedOutputs()) {
        if (!output->currentMode()) {
            continue;
        }

        const KScreen::ModePtr mode = output->currentMode();

        auto *view = new QQuickView();

        view->setFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
        view->setResizeMode(QQuickView::SizeViewToRootObject);
        view->setSource(QUrl::fromLocalFile(qmlPath));
        view->installEventFilter(this);

        QQuickItem *rootObj = view->rootObject();
        if (!rootObj) {
            continue;
        }

        QSize deviceSize, logicalSize;
        if (output->isHorizontal()) {
            deviceSize = mode->size();
        } else {
            deviceSize = QSize(mode->size().height(), mode->size().width());
        }
        if (config->supportedFeatures() & KScreen::Config::Feature::PerOutputScaling) {
            // no scale adjustment needed on Wayland
            logicalSize = deviceSize;
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

bool OutputIdentifier::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::Resize) {
        if (m_views.contains(qobject_cast<QQuickView*>(object))) {
            QResizeEvent *e = static_cast<QResizeEvent*>(event);
            const QRect screenSize = object->property("screenSize").toRect();
            QRect geometry(QPoint(0, 0), e->size());
            geometry.moveCenter(screenSize.center());
            static_cast<QQuickView*>(object)->setGeometry(geometry);
        }
    }
    return QObject::eventFilter(object, event);
}
