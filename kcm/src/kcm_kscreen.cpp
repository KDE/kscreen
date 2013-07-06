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
#include "qmloutput.h"
#include "qmlcursor.h"
#include "modeselectionwidget.h"
#include "fallbackcomponent.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KStandardDirs>
#include <KUrl>
#include <KDebug>
#include <Plasma/TreeView>
#include <KMessageBox>

#include <QGridLayout>
#include <QLabel>

#include <QtDeclarative>
#include <QDeclarativeView>
#include <QDeclarativeContext>

#include <kscreen/config.h>
#include <kscreen/edid.h>
#include <kscreen/configmonitor.h>

K_PLUGIN_FACTORY(KCMDisplayConfigurationFactory, registerPlugin<KCMKScreen>();)
K_EXPORT_PLUGIN(KCMDisplayConfigurationFactory ("kcm_kscreen" /* kcm name */,
                                                "kcm_displayconfiguration" /* catalog name */))

#define QML_PATH "kcm_kscreen/qml/"

using namespace KScreen;

Q_DECLARE_METATYPE(KScreen::Output*)
Q_DECLARE_METATYPE(KScreen::Screen*)

KCMKScreen::KCMKScreen(QWidget* parent, const QVariantList& args) :
    KCModule(KCMDisplayConfigurationFactory::componentData(), parent, args),
    m_config(0),
    m_declarativeView(0)
{
    KAboutData* about =
        new KAboutData("kscreen", "kcm_kscren",
                    ki18n("Display Configuration"),
                    "", ki18n("Configuration for displays"),
                    KAboutData::License_GPL_V2, ki18n("(c), 2012 Dan Vrátil"));

    about->addAuthor(ki18n("Dan Vrátil"), ki18n("Maintainer") , "dvratil@redhat.com");
    setAboutData(about);

    m_outputTimer = new QTimer(this);
    connect(m_outputTimer, SIGNAL(timeout()), SLOT(clearOutputIdentifiers()));

    /* FIXME: Workaround to prevent KScreen::QRandr eating our X11 events */
    qApp->setEventFilter(KCMKScreen::x11EventFilter);

    QGridLayout* mainLayout = new QGridLayout(this);

    m_config = Config::current();
    if (m_config) {
        QString importPath = KStandardDirs::installPath("lib") +
                QDir::separator() + "kde4" + QDir::separator() + "imports";

        qmlRegisterType<FallbackComponent>("org.kde.plasma.extras410", 0, 1, "FallbackComponent");

        qmlRegisterType<QMLOutput>("KScreen", 1, 0, "QMLOutput");
        qmlRegisterType<ModeSelectionWidget>("KScreen", 1, 0, "ModeSelectionWidget");

        qmlRegisterInterface<KScreen::Output*>("Output");
        qmlRegisterInterface<KScreen::Mode*>("OutputMode");
        qmlRegisterInterface<KScreen::Edid*>("EDID");
        qmlRegisterInterface<KScreen::Screen*>("Screen");
        qmlRegisterType<KScreen::Output>("KScreen", 1, 0, "Output");
        qmlRegisterType<KScreen::Mode>("KScreen", 1, 0, "OutputMode");
        qmlRegisterType<KScreen::Edid>("KScreen", 1, 0, "EDID");
        qmlRegisterType<KScreen::Screen>("KScreen", 1, 0, "Screen");

        m_declarativeView = new QDeclarativeView(this);
        m_declarativeView->setFrameStyle(QFrame::Panel | QFrame::Raised);
        m_declarativeView->engine()->addImportPath(importPath);
        m_declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
        m_declarativeView->setStyleSheet("background: transparent");
        m_declarativeView->setMinimumHeight(440);
        mainLayout->addWidget(m_declarativeView, 0, 0);

        /* Declarative view will be initialized from load() */
    } else {
        QLabel* label = new QLabel(this);
        label->setText(i18n("No supported X Window System extension found"));
        label->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);

        mainLayout->addWidget(label, 0, 0);
    }
}

KCMKScreen::~KCMKScreen()
{
}

bool KCMKScreen::x11EventFilter(void* message, long int* result)
{
    Q_UNUSED(message);
    Q_UNUSED(result);

    /* Propagate the event */
    return false;
}

void KCMKScreen::load()
{
    kDebug() << "Loading...";

    if (!m_declarativeView) {
        return;
    }

    if (m_config) {
        KScreen::ConfigMonitor::instance()->removeConfig(m_config);
    }
    m_config = Config::current();
    KScreen::ConfigMonitor::instance()->addConfig(m_config);

    const QString qmlPath = KStandardDirs::locate(
            "data", QLatin1String(QML_PATH "main.qml"));
    m_declarativeView->setSource(qmlPath);

    QMLCursor *cursor = new QMLCursor(m_declarativeView);
    m_declarativeView->rootContext()->setContextProperty(QLatin1String("_cursor"), cursor);

    QDeclarativeItem *rootObj = dynamic_cast<QDeclarativeItem*>(m_declarativeView->rootObject());
    if (!rootObj) {
        kWarning() << "Failed to obtain root item";
        return;
    }

    rootObj->setProperty("virtualScreen", QVariant::fromValue(m_config->screen()));
    connect(rootObj, SIGNAL(identifyOutputsRequested()), SLOT(identifyOutputs()));

    QDeclarativeItem *outputView = rootObj->findChild<QDeclarativeItem*>(QLatin1String("outputView"));
    if (!outputView) {
        kWarning() << "Failed to obtain output view";
        return;
    }

    const QList<KScreen::Output*> outputs = m_config->outputs().values();
    Q_FOREACH (KScreen::Output *output, outputs) {
        QMetaObject::invokeMethod(outputView, "addOutput", Q_ARG(QVariant, QVariant::fromValue(output)));
    }
    QMetaObject::invokeMethod(outputView, "reorderOutputs");

    connect(outputView, SIGNAL(outputChanged()), SLOT(changed()));
    connect(outputView, SIGNAL(moveMouse(int,int)), SLOT(moveMouse(int,int)));
    connect(outputView, SIGNAL(outputMousePressed()), SLOT(outputMousePressed()));
    connect(outputView, SIGNAL(outputMouseReleased()), SLOT(outputMouseReleased()));
}

void KCMKScreen::save()
{
    kDebug() << "Saving";

    if (!m_declarativeView) {
        return;
    }

    bool atLeastOneEnabledOutput = false;
    Q_FOREACH(KScreen::Output *output, m_config->outputs()) {
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
    m_config->setConfig(m_config);
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
    m_declarativeView->setCursor(Qt::ClosedHandCursor);
}

void KCMKScreen::outputMouseReleased()
{
    m_declarativeView->setCursor(Qt::ArrowCursor);
}

#include "kcm_kscreen.moc"
