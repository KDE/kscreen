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
#include <KCModuleLoader>

#include <QDeclarativeDebuggingEnabler>

int main(int argc, char **argv)
{
    QDeclarativeDebuggingEnabler enabler;

    KAboutData aboutData("kcm_testapp", "kcm_testapp", ki18n("KCM Test App"), "1.0");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    KCModule *module = KCModuleLoader::loadModule("kcm_kscreen", KCModuleLoader::Inline);
    module->resize(800, 600);
    module->show();

    return app.exec();
}

