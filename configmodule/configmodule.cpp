/***************************************************************************
 *                                                                         *
 *   Copyright 2015 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "configmodule.h"
#include "modeselector.h"
#include "debug_p.h"

#include <QtQml>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KAboutData>

#include <KScreen/EDID>
#include <KScreen/GetConfigOperation>
#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/Screen>

#include "qmloutput.h"
#include "qmlscreen.h"

K_PLUGIN_FACTORY_WITH_JSON(KScreenConfigModuleFactory, "kcm_kscreen2.json", registerPlugin<KScreen::ConfigModule>();)

Q_DECLARE_METATYPE(KScreen::OutputPtr)
Q_DECLARE_METATYPE(KScreen::ScreenPtr)
Q_DECLARE_METATYPE(KScreen::ModePtr)

namespace KScreen {

ConfigModule::ConfigModule(QObject* parent, const QVariantList& args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_modeSelector(new ModeSelector(this))
{
    KAboutData* about = new KAboutData("kcm_kscreen2", i18n("Configure monitors and displays"),
                                       "2.0", QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Sebastian Kügler"), QString(), "sebas@kde.org");
    setAboutData(about);
    setButtons(Apply | Default);

    qmlRegisterType<QMLOutput>("org.kde.kscreen", 2, 0, "QMLOutput");
    qmlRegisterType<QMLScreen>("org.kde.kscreen", 2, 0, "QMLScreen");

    qmlRegisterType<KScreen::Output>("org.kde.kscreen", 2, 0, "KScreenOutput");
    qmlRegisterType<KScreen::Edid>("org.kde.kscreen", 2, 0, "KScreenEdid");
    qmlRegisterType<KScreen::Mode>("org.kde.kscreen", 2, 0, "KScreenMode");
    qmlRegisterType<KScreen::ModeSelector>("org.kde.kscreen", 2, 0, "KScreenModeSelector");
//     qmlRegisterType<KScreen::ModeList>("org.kde.kscreen", 2, 0, "KScreenModeList");

    mScreen = mainUi()->findChild<QMLScreen*>(QStringLiteral("outputView"));
    if (!mScreen) {
        qCWarning(KSCREEN_KCM) << "outputView object not found in QtQuick code";
        return;
    }
    mScreen->setEngine(engine());

    connect(mScreen, &QMLScreen::focusedOutputChanged,
            this, &ConfigModule::focusedOutputChanged);


    qCDebug(KSCREEN_KCM) << "ConfigModule loaded.";
}

ConfigModule::~ConfigModule()
{
}

void ConfigModule::load()
{
    qCDebug(KSCREEN_KCM) << "LOAD";
    connect(new GetConfigOperation(), &GetConfigOperation::finished,
            this, &ConfigModule::configReady);
}

void ConfigModule::defaults()
{
}

void ConfigModule::save()
{
}

void ConfigModule::changed()
{
}

KScreen::ModeSelector* ConfigModule::modeSelector() const
{
    return m_modeSelector;
}

void ConfigModule::focusedOutputChanged(QMLOutput *output)
{
    qCDebug(KSCREEN_KCM) << "Focused output is now:" << output->output()->name();
    m_modeSelector->setOutputPtr(output->outputPtr());
}

void ConfigModule::configReady(KScreen::ConfigOperation* op)
{
    qDebug() << "config ready!";
    setConfig(qobject_cast<GetConfigOperation*>(op)->config());
}

void ConfigModule::setConfig(const KScreen::ConfigPtr &config)
{
    if (mConfig) {
        KScreen::ConfigMonitor::instance()->removeConfig(mConfig);
        for (const KScreen::OutputPtr &output : mConfig->outputs()) {
            output->disconnect(this);
        }
    }

    mConfig = config;
    KScreen::ConfigMonitor::instance()->addConfig(mConfig);
    mScreen->setConfig(mConfig);

    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        connect(output.data(), &KScreen::Output::posChanged,
                this, [this] () {
                    setNeedsSave(true);
                }
        );
    }

    // Select the primary (or only) output by default
    QMLOutput *qmlOutput = mScreen->primaryOutput();
    if (qmlOutput) {
        mScreen->setActiveOutput(qmlOutput);
    } else {
        if (mScreen->outputs().count() > 0) {
            mScreen->setActiveOutput(mScreen->outputs()[0]);
        }
    }
    emit configSet();
}

KScreen::ConfigPtr ConfigModule::currentConfig() const
{
    return mConfig;
}


}
#include "configmodule.moc"
