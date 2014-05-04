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

#include <QtDeclarative/QDeclarativeEngine>

#include <KStandardDirs>
#include <KDebug>
#include <KUrl>
#include <QDir>

Q_DECLARE_METATYPE(KScreen::Output*)
Q_DECLARE_METATYPE(QMLScreen*)

QMLOutputComponent::QMLOutputComponent(QDeclarativeEngine *engine, QMLScreen *parent):
    QDeclarativeComponent(engine, parent),
    m_engine(engine)
{
    const QString qmlPath = KStandardDirs::locate("data", QLatin1String("kcm_kscreen/qml/Output.qml"));
    loadUrl(KUrl::fromPath(qmlPath));
}

QMLOutputComponent::~QMLOutputComponent()
{
}

QMLOutput* QMLOutputComponent::createForOutput(KScreen::Output *output)
{
    QObject *instance;

    instance = beginCreate(m_engine->rootContext());
    if (!instance) {
        kWarning() << errorString();
        return 0;
    }

    instance->setProperty("output", QVariant::fromValue(output));
    instance->setProperty("screen", QVariant::fromValue(qobject_cast<QMLScreen*>(parent())));

    completeCreate();

    return qobject_cast<QMLOutput*>(instance);
}

#include "qmloutputcomponent.moc"
