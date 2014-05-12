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

#include <QQuickWidget>

#include <kscreen/config.h>
#include <qquickitem.h>

K_PLUGIN_FACTORY(KCMDisplayConfigurationFactory, registerPlugin<KCMKScreen>();)
K_EXPORT_PLUGIN(KCMDisplayConfigurationFactory ("kcm_kscreen" /* kcm name */,
                                                "kcm_displayconfiguration" /* catalog name */))

#define QML_PATH "kcm_kscreen/qml/"

using namespace KScreen;

Q_DECLARE_METATYPE(KScreen::Output*)
Q_DECLARE_METATYPE(KScreen::Screen*)

KCMKScreen::KCMKScreen(QWidget* parent, const QVariantList& args) :
    KCModule(parent, args)
{
    setButtons(Apply | Default);

    KAboutData* about =
        new KAboutData(QStringLiteral("kscreen"), QStringLiteral("kcm_kscren"),
                    i18n("Display Configuration"),
                    QString(), i18n("Configuration for displays"),
                    KAboutData::License_GPL_V2, i18n("(c), 2012-2013 Daniel Vrátil"));

    about->addAuthor(i18n("Daniel Vrátil"), i18n("Maintainer") , QStringLiteral("dvratil@redhat.com"));
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



void KCMKScreen::clearOutputIdentifiers()
{
    m_outputTimer->stop();
    qDeleteAll(m_outputIdentifiers);
    m_outputIdentifiers.clear();
}

void KCMKScreen::identifyOutputs()
{
    const QString qmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String(QML_PATH "OutputIdentifier.qml"));

    m_outputTimer->stop();
    clearOutputIdentifiers();

    /* Obtain the current active configuration from KScreen */
    OutputList outputs = KScreen::Config::current()->outputs();
    Q_FOREACH (KScreen::Output *output, outputs) {
        if (!output->isConnected() || !output->currentMode()) {
            continue;
        }

        Mode *mode = output->currentMode();

        QQuickWidget *view = new QQuickWidget();
        view->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
        view->setResizeMode(QQuickWidget::SizeViewToRootObject);
        view->setSource(QUrl::fromLocalFile(qmlPath));

        QQuickItem *rootObj = view->rootObject();
        if (!rootObj) {
            qWarning() << "Failed to obtain root item";
            continue;
        }
        QSize realSize;
        if (output->isHorizontal()) {
            realSize = mode->size();
        } else {
            realSize = QSize(mode->size().height(), mode->size().width());
        }
        rootObj->setProperty("outputName", output->name());
        rootObj->setProperty("modeName", QStringLiteral("%1x%2").arg(realSize.width()).arg(realSize.height()));

        const QRect outputRect(output->pos(), realSize);
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


#include "kcm_kscreen.moc"
