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
#include <KStandardDirs>
#include <KUrl>
#include <KDebug>
#include <KMessageBox>

#include <QHBoxLayout>
#include <QTimer>

#include <QDeclarativeView>
#include <QDeclarativeItem>

#include <kscreen/config.h>

K_PLUGIN_FACTORY(KCMDisplayConfigurationFactory, registerPlugin<KCMKScreen>();)
K_EXPORT_PLUGIN(KCMDisplayConfigurationFactory ("kcm_kscreen" /* kcm name */,
                                                "kcm_displayconfiguration" /* catalog name */))

#define QML_PATH "kcm_kscreen/qml/"

using namespace KScreen;

Q_DECLARE_METATYPE(KScreen::Output*)
Q_DECLARE_METATYPE(KScreen::Screen*)

KCMKScreen::KCMKScreen(QWidget* parent, const QVariantList& args) :
    KCModule(KCMDisplayConfigurationFactory::componentData(), parent, args)
{
    KAboutData* about =
        new KAboutData("kscreen", "kcm_kscren",
                    ki18n("Display Configuration"),
                    "", ki18n("Configuration for displays"),
                    KAboutData::License_GPL_V2, ki18n("(c), 2012-2013 Daniel Vrátil"));

    about->addAuthor(ki18n("Daniel Vrátil"), ki18n("Maintainer") , "dvratil@redhat.com");
    setAboutData(about);

    m_outputTimer = new QTimer(this);
    connect(m_outputTimer, SIGNAL(timeout()), SLOT(clearOutputIdentifiers()));

    QHBoxLayout *layout = new QHBoxLayout(this);
    mKScreenWidget = new Widget(this);
    layout->addWidget(mKScreenWidget);

    connect(mKScreenWidget, SIGNAL(changed()),
            this, SLOT(changed()));
}

KCMKScreen::~KCMKScreen()
{
    clearOutputIdentifiers();
}

void KCMKScreen::save()
{
    kDebug() << "Saving";

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

        kDebug() << output->name() << "\n"
                << "	Connected:" << output->isConnected() << "\n"
                << "	Enabled:" << output->isEnabled() << "\n"
                << "	Primary:" << output->isPrimary() << "\n"
                << "	Rotation:" << output->rotation() << "\n"
                << "	Mode:" << (mode ? mode->name() : "unknown") << "@" << (mode ? mode->refreshRate() : 0.0) << "Hz" << "\n"
                << "    Position:" << output->pos().x() << "x" << output->pos().y();
    }

    if (!atLeastOneEnabledOutput) {
        if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to disable all outputs?"),
            i18n("Disable all outputs?"),
            KGuiItem(i18n("&Disable All Outputs"), KIcon(QLatin1String("dialog-ok-apply"))),
            KGuiItem(i18n("&Reconfigure"), KIcon(QLatin1String("dialog-cancel"))),
            QString(), KMessageBox::Dangerous) == KMessageBox::No)
        {
            return;
        }
    }

    /* Store the current config, apply settings */
    config->setConfig(config);
}

void KCMKScreen::defaults()
{
    if (!mKScreenWidget) {
        return;
    }

    mKScreenWidget->setConfig(KScreen::Config::current());
}


void KCMKScreen::clearOutputIdentifiers()
{
    m_outputTimer->stop();
    qDeleteAll(m_outputIdentifiers);
    m_outputIdentifiers.clear();
}

void KCMKScreen::identifyOutputs()
{
    const QString qmlPath = KStandardDirs::locate(
            "data", QLatin1String(QML_PATH "OutputIdentifier.qml"));

    m_outputTimer->stop();
    clearOutputIdentifiers();

    /* Obtain the current active configuration from KScreen */
    OutputList outputs = KScreen::Config::current()->outputs();
    Q_FOREACH (KScreen::Output *output, outputs) {
        if (!output->isConnected() || !output->currentMode()) {
            continue;
        }

        Mode *mode = output->currentMode();

        QDeclarativeView *view = new QDeclarativeView();
        view->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
        view->setResizeMode(QDeclarativeView::SizeViewToRootObject);
        view->setSource(KUrl::fromPath(qmlPath));

        QDeclarativeItem *rootObj = dynamic_cast<QDeclarativeItem*>(view->rootObject());
        if (!rootObj) {
            kWarning() << "Failed to obtain root item";
            continue;
        }
        rootObj->setProperty("outputName", output->name());
        rootObj->setProperty("modeName", mode->name());

        QRect outputRect(output->pos(), mode->size());
        QRect geometry(QPoint(0, 0), view->sizeHint());
        geometry.moveCenter(outputRect.center());
        view->setGeometry(geometry);

        m_outputIdentifiers << view;
    }

    Q_FOREACH (QWidget *widget, m_outputIdentifiers) {
        widget->show();
    }

    m_outputTimer->start(2500);
}

void KCMKScreen::moveMouse(int dX, int dY)
{
    QPoint pos = QCursor::pos();
    pos.rx() += dX;
    pos.ry() += dY;

    QCursor::setPos(pos);
}

void KCMKScreen::outputMousePressed()
{
    //m_declarativeView->setCursor(Qt::ClosedHandCursor);
}

void KCMKScreen::outputMouseReleased()
{
    //m_declarativeView->setCursor(Qt::ArrowCursor);
}
