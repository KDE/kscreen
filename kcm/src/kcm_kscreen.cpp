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

#include <QQuickWidget>

#include <kscreen/config.h>
#include <qquickitem.h>

K_PLUGIN_FACTORY(KCMDisplayConfigurationFactory, registerPlugin<KCMKScreen>();)
K_EXPORT_PLUGIN(KCMDisplayConfigurationFactory ("kcm_kscreen" /* kcm name */,
                                                "kcm_displayconfiguration" /* catalog name */))

using namespace KScreen;

Q_DECLARE_METATYPE(KScreen::Output*)
Q_DECLARE_METATYPE(KScreen::Screen*)

KCMKScreen::KCMKScreen(QWidget* parent, const QVariantList& args) :
    KCModule(parent, args)
{
    setButtons(Apply | Default);

    KAboutData* about =
        new KAboutData(QStringLiteral("kcm_kscren"),
                    i18n("Display Configuration"),
                    QString(), i18n("Configuration for displays"),
                    KAboutLicense::GPL_V2, i18n("(c), 2012-2013 Daniel Vrátil"));

    about->addAuthor(i18n("Daniel Vrátil"), i18n("Maintainer") , QStringLiteral("dvratil@redhat.com"));
    setAboutData(about);

    QHBoxLayout *layout = new QHBoxLayout(this);
    if (KScreen::Config::current()) {
        mKScreenWidget = new Widget(this);
        layout->addWidget(mKScreenWidget);

        connect(mKScreenWidget, SIGNAL(changed()),
                this, SLOT(changed()));
    } else {
        mKScreenWidget = 0;
        QLabel *errorLabel = new QLabel(this);
        layout->addWidget(errorLabel);
        errorLabel->setText(i18n("No kscreen backend found. Please check your kscreen installation."));
    }
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

    KScreen::Config *config = mKScreenWidget->currentConfig();

    bool atLeastOneEnabledOutput = false;
    Q_FOREACH(KScreen::Output *output, config->outputs()) {
        KScreen::Mode *mode = output->currentMode();

        if (output->isEnabled()) {
            atLeastOneEnabledOutput = true;
        }

        qDebug() << output->name() << "\n"
                << "	Connected:" << output->isConnected() << "\n"
                << "	Enabled:" << output->isEnabled() << "\n"
                << "	Primary:" << output->isPrimary() << "\n"
                << "	Rotation:" << output->rotation() << "\n"
                << "	Mode:" << (mode ? mode->name() : QStringLiteral("unknown")) << "@" << (mode ? mode->refreshRate() : 0.0) << "Hz" << "\n"
                << "    Position:" << output->pos().x() << "x" << output->pos().y();
    }

    if (!atLeastOneEnabledOutput) {
        if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to disable all outputs?"),
            i18n("Disable all outputs?"),
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
                 i18n("Unsupported configuration"));
        return;
    }

    /* Store the current config, apply settings */
    config->setConfig(config);
}

void KCMKScreen::defaults()
{
    load();
}

void KCMKScreen::load()
{
    if (!mKScreenWidget) {
        return;
    }

    mKScreenWidget->setConfig(KScreen::Config::current());
}

#include "kcm_kscreen.moc"
