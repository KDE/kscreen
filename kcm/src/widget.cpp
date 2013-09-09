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

#include "widget.h"
#include "controlpanel.h"

#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeEngine>

#include <QtGui/QVBoxLayout>

#include "declarative/qmloutput.h"
#include "declarative/qmlscreen.h"
#include "declarative/iconbutton.h"

#include <kscreen/output.h>
#include <kscreen/edid.h>
#include <kscreen/mode.h>
#include <QDir>

Widget::Widget(QWidget *parent):
    QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_declarativeView = new QDeclarativeView(this);
    m_declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    layout->addWidget(m_declarativeView);
    loadQml();

    m_controlPanel = new ControlPanel(this);
    layout->addWidget(m_controlPanel);
}

Widget::~Widget()
{
}

void Widget::loadQml()
{
    qmlRegisterType<QMLOutput>("org.kde.kscreen", 1, 0, "QMLOutput");
    qmlRegisterType<QMLScreen>("org.kde.kscreen", 1, 0, "QMLScreen");
    qmlRegisterType<IconButton>("org.kde.kscreen", 1, 0, "IconButton");

    qmlRegisterType<KScreen::Output>("org.kde.kscreen", 1, 0, "KScreenOutput");
    qmlRegisterType<KScreen::Edid>("org.kde.kscreen", 1, 0, "KScreenEdid");
    qmlRegisterType<KScreen::Mode>("org.kde.kscreen", 1, 0, "KScreenMode");

    QDir dir = QDir::current();
    dir.cdUp();
    dir.cdUp();
    dir.cd("kcm/qml");
    QDir::setCurrent(dir.path());

    const QString file = QDir::currentPath() + "/main.qml";
    m_declarativeView->engine()->addImportPath(QLatin1String("/usr/lib64/kde4/imports/"));
    m_declarativeView->setSource(QUrl::fromLocalFile(file));

    QGraphicsObject *rootObject = m_declarativeView->rootObject();
    QMLScreen *screen = rootObject->findChild<QMLScreen*>(QLatin1String("outputView"));
    if (!screen) {
        return;
    }

}

#include "widget.moc"
