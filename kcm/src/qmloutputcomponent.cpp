/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "qmloutputcomponent.h"
#include "qmloutput.h"

#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeItem>
#include <KStandardDirs>
#include <KDebug>
#include <KUrl>

#include <kscreen/output.h>

Q_DECLARE_METATYPE(KScreen::Output*)

QMLOutputComponent::QMLOutputComponent(QDeclarativeEngine *engine, QObject *parent):
    QDeclarativeComponent(engine, parent),
    m_engine(engine)
{
    QString qmlPath = KStandardDirs::locate (
            "data", QLatin1String("kcm_displayconfiguration/qml/Output.qml"));

    loadUrl(KUrl::fromPath(qmlPath));
}

QMLOutputComponent::~QMLOutputComponent()
{
}

QMLOutput* QMLOutputComponent::createForOutput(KScreen::Output* output)
{
    QObject *instance;

    /* Create object hierarchy */
    instance = beginCreate(m_engine->rootContext());
    if (!instance) {
        kWarning() << errorString();
        return 0;
    }

    /* Assign the output object before property bindings are made */
    instance->setProperty("output", QVariant::fromValue(output));

    /* Create property bindings */
    completeCreate();

    return dynamic_cast<QMLOutput*>(instance);
}

#include "qmloutputcomponent.moc"
