/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "qmloutputcomponent.h"
#include "qmloutput.h"
#include "qmlscreen.h"

#include <kscreen/output.h>

#include <QDir>
#include <QStandardPaths>
#include <QQmlEngine>

Q_DECLARE_METATYPE(KScreen::OutputPtr)
Q_DECLARE_METATYPE(QMLScreen*)

QMLOutputComponent::QMLOutputComponent(QQmlEngine *engine, QMLScreen *parent):
    QQmlComponent(engine, parent),
    m_engine(engine)
{
    const QString qmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kcm_kscreen/qml/Output.qml"));
    loadUrl(QUrl::fromLocalFile(qmlPath));
}

QMLOutputComponent::~QMLOutputComponent()
{
}

QMLOutput* QMLOutputComponent::createForOutput(const KScreen::OutputPtr &output)
{
    QObject *instance;

    instance = beginCreate(m_engine->rootContext());
    if (!instance) {
        qWarning() << errorString();
        return 0;
    }

    bool success = false;
    success = instance->setProperty("outputPtr", QVariant::fromValue(output));
    Q_ASSERT(success);
    success = instance->setProperty("screen", QVariant::fromValue(qobject_cast<QMLScreen*>(parent())));
    Q_ASSERT(success);
    Q_UNUSED(success);

    completeCreate();

    return qobject_cast<QMLOutput*>(instance);
}
