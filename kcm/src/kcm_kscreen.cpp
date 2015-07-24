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


#include "kcm_kscreen.h"
#include "widget.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KMessageBox>
#include <KLocalizedString>

#include <QHBoxLayout>
#include <QTimer>
#include <qstandardpaths.h>
#include <QLabel>

#include <QQuickView>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

#include <qquickitem.h>

K_PLUGIN_FACTORY(KCMDisplayConfigurationFactory, registerPlugin<KCMKScreen>();)

using namespace KScreen;

Q_DECLARE_METATYPE(KScreen::OutputPtr)
Q_DECLARE_METATYPE(KScreen::ScreenPtr)

KCMKScreen::KCMKScreen(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , mKScreenWidget(0)
{
    setButtons(Apply | Default);

    KAboutData* about =
        new KAboutData(QStringLiteral("kcm_kscreen"),
                    i18n("Display Configuration"),
                    QString(KSCREEN_VERSION), i18n("Configuration for displays"),
                    KAboutLicense::GPL_V2, i18n("(c), 2012-2013 Daniel Vrátil"));

    about->addAuthor(i18n("Daniel Vrátil"), i18n("Maintainer") , QStringLiteral("dvratil@redhat.com"));
    setAboutData(about);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
}

void KCMKScreen::configReady(ConfigOperation* op)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    if (op->hasError()) {
        mKScreenWidget = 0;
        delete mKScreenWidget;
        QLabel *errorLabel = new QLabel(this);
        layout->addWidget(errorLabel);
        errorLabel->setText(i18n("No kscreen backend found. Please check your kscreen installation."));
        return;
    }

    if (!mKScreenWidget) {
        mKScreenWidget = new Widget(this);
        layout->addWidget(mKScreenWidget);
        connect(mKScreenWidget, SIGNAL(changed()),
                this, SLOT(changed()));
    }

    mKScreenWidget->setConfig(qobject_cast<GetConfigOperation*>(op)->config());
    mKScreenWidget->getShowOsd();
}

KCMKScreen::~KCMKScreen()
{
}

void KCMKScreen::save()
{
    qDebug() << "Saving";

    if (!mKScreenWidget) {
        return;
    }

    mKScreenWidget->setShowOsd();

    const KScreen::ConfigPtr &config = mKScreenWidget->currentConfig();

    bool atLeastOneEnabledOutput = false;
    Q_FOREACH(const KScreen::OutputPtr &output, config->outputs()) {
        KScreen::ModePtr mode = output->currentMode();

        if (output->isEnabled()) {
            atLeastOneEnabledOutput = true;
        }

        qDebug() << output->name() << output->id() << output.data() << "\n"
                << "	Connected:" << output->isConnected() << "\n"
                << "	Enabled:" << output->isEnabled() << "\n"
                << "	Primary:" << output->isPrimary() << "\n"
                << "	Rotation:" << output->rotation() << "\n"
                << "	Mode:" << (mode ? mode->name() : QStringLiteral("unknown")) << "@" << (mode ? mode->refreshRate() : 0.0) << "Hz" << "\n"
                << "    Position:" << output->pos().x() << "x" << output->pos().y();
    }

    if (!atLeastOneEnabledOutput) {
        if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to disable all outputs?"),
            i18nc("@title:window", "Disable All Outputs"),
            KGuiItem(i18n("&Disable All Outputs"), QIcon::fromTheme(QLatin1String("dialog-ok-apply"))),
            KGuiItem(i18n("&Reconfigure"), QIcon::fromTheme(QLatin1String("dialog-cancel"))),
            QString(), KMessageBox::Dangerous) == KMessageBox::No)
        {
            return;
        }
    }

    if (!Config::canBeApplied(config)) {
        KMessageBox::information(this,
            i18n("Sorry, your configuration could not be applied.\n\n"
                 "Common reasons are that the overall screen size is too big, or you enabled more displays than supported by your GPU."),
                 i18nc("@title:window", "Unsupported Configuration"));
        return;
    }

    /* Store the current config, apply settings */
    auto *op = new SetConfigOperation(config);
    /* Block until the operation is completed, otherwise KCMShell will terminate
     * before we get to execute the Operation */
    op->exec();
}

void KCMKScreen::defaults()
{
    qDebug() << "LOAD";
    load();
}

void KCMKScreen::load()
{
    qDebug() << "LOAD";
    connect(new GetConfigOperation(), &GetConfigOperation::finished,
            this, &KCMKScreen::configReady);
}

#include "kcm_kscreen.moc"
