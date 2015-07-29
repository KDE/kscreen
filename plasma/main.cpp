/*
 * Copyright 2015  Leslie Zhai <xiang.zhai@i-soft.com.cn>
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

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>

#include "kscreenapplet.h"

QT_BEGIN_NAMESPACE

class KScreenAppletDeclarativeModule : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")

public:
    virtual void registerTypes(const char *uri) {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.kscreen"));

        qmlRegisterType<KScreenApplet>(uri, 1, 0, "KScreenApplet");
    }

    void initializeEngine(QQmlEngine *engine, const char *uri) {
        Q_UNUSED(engine);
        Q_UNUSED(uri);
    }
};

QT_END_NAMESPACE

#include "main.moc"
