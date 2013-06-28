/*
    Copyright (C) 2013  Dan Vratil <dvratil@redhat.com>

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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
//#include <KCModuleLoader>

#include <QDir>
#include <unistd.h>

#include <QDeclarativeView>
#include <QDeclarativeDebuggingEnabler>
#include <QtDeclarative>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>
#include <kscreen/mode.h>

#include "iconbutton.h"
#include "qmloutput.h"
#include "qmlscreen.h"
#include "qmlslider.h"

Q_DECLARE_METATYPE(KScreen::Output*)

int main(int argc, char **argv)
{
    QDeclarativeDebuggingEnabler enabler;

    KAboutData aboutData("kcm_testapp", "kcm_testapp", ki18n("KCM Test App"), "1.0");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    //KCModule *module = KCModuleLoader::loadModule("kcm_kscreen", KCModuleLoader::Inline);

    qmlRegisterType<QMLOutput>("org.kde.kscreen", 1, 0, "QMLOutput");
    qmlRegisterType<QMLScreen>("org.kde.kscreen", 1, 0, "QMLScreen");
    qmlRegisterType<QMLSlider>("org.kde.kscreen", 1, 0, "QMLSlider");
    qmlRegisterType<IconButton>("org.kde.kscreen", 1, 0, "IconButton");

    qmlRegisterType<KScreen::Output>("org.kde.kscreen", 1, 0, "KScreenOutput");
    qmlRegisterType<KScreen::Edid>("org.kde.kscreen", 1, 0, "KScreenEdid");
    qmlRegisterType<KScreen::Mode>("org.kde.kscreen", 1, 0, "KScreenMode");

    QDir cwd = QDir::current();
    cwd.cdUp();
    cwd.cdUp();
    cwd.cd(QLatin1String("kcm/qml"));
    chdir(cwd.path().toLatin1().constData());
    QDeclarativeView view;
    view.engine()->addImportPath(QLatin1String("/usr/lib64/kde4/imports/"));
    view.setSource(QUrl("main.qml"));
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view.resize(900, 800);
    view.show();

    /*
    module->resize(800, 600);
    module->show();
    */

    return app.exec();
}
