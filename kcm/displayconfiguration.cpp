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


#include "displayconfiguration.h"
#include "controlpanel.h"
#include "qmloutputview.h"
#include "qmloutput.h"

#include <KPluginFactory>
#include <KAboutData>
#include <QGridLayout>
#include <QLabel>
#include <KDebug>
#include <KStandardDirs>

#include <QtDeclarative>
#include <QDeclarativeView>
#include <QDeclarativeContext>

#include <kscreen/provider.h>
#include <kscreen/config.h>

K_PLUGIN_FACTORY(KCMDisplayConfiguraionFactory, registerPlugin<DisplayConfiguration>();)
K_EXPORT_PLUGIN(KCMDisplayConfiguraionFactory ("kcm_displayconfiguration" /* kcm name */,
                "kcm_displayconfiguration" /* catalog name */))

using namespace KScreen;

Q_DECLARE_METATYPE(KScreen::Output*);


DisplayConfiguration::DisplayConfiguration(QWidget* parent, const QVariantList& args) :
    KCModule(KCMDisplayConfiguraionFactory::componentData(), parent, args)
{
    KAboutData* about =
        new KAboutData("displayconfiguration", "displayconfiguration",
                       ki18n("Monitor display Configuration"),
                       "", ki18n("Configuration for displays"),
                       KAboutData::License_GPL_V2, ki18n("(c), 2012 Dan Vrátil"));

    about->addAuthor(ki18n("Dan Vrátil"), ki18n("Maintainer") , "dvratil@redhat.com");
    setAboutData(about);

    m_outputTimer = new QTimer(this);
    connect(m_outputTimer, SIGNAL(timeout()), SLOT(clearOutputIdentifiers()));

    /* FIXME: Workaround to prevent KScreen::QRandr eating our X11 events */
    qApp->setEventFilter(DisplayConfiguration::x11EventFilter);

    QGridLayout* mainLayout = new QGridLayout(this);

    //setenv("KSCREEN_BACKEND", "Fake", 1);
    setenv("KSCREEN_BACKEND", "XRandR", 1);
    Provider* screen = Provider::self();
    if (screen && screen->isValid()) {
	QString importPath = KStandardDirs::installPath("lib") +
		QDir::separator() + "kde4" + QDir::separator() + "imports";

	qmlRegisterType<QMLOutputView>("KScreen", 1, 0, "QMLOutputView");
	qmlRegisterType<QMLOutput>("KScreen", 1, 0, "QMLOutput");

	/* FIXME Clear up this */
	qmlRegisterInterface<KScreen::Output*>("Output");
	qmlRegisterInterface<KScreen::Mode*>("OutputMode");
	qmlRegisterType<KScreen::Output>("KScreen", 1, 0, "Output");
	qmlRegisterType<KScreen::Mode>("KScreen", 1, 0, "OutputMode");

        m_declarativeView = new QDeclarativeView(this);
	m_declarativeView->engine()->addImportPath(importPath);
        m_declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
	m_declarativeView->setStyleSheet("background: transparent");
	m_declarativeView->setMinimumHeight(500);
        mainLayout->addWidget(m_declarativeView, 0, 0);
	/* Declarative view will be initialized from load() */

	m_controlPanel = new ControlPanel(this);
	mainLayout->addWidget(m_controlPanel, 1, 0);
    } else {
        QLabel* label = new QLabel(this);
        label->setText(i18n("No supported X Window System extension found"));
        label->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);

        mainLayout->addWidget(label, 0, 0);
    }
}


DisplayConfiguration::~DisplayConfiguration()
{
}

bool DisplayConfiguration::x11EventFilter(void* message, long int* result)
{
	Q_UNUSED(message);
	Q_UNUSED(result);

	/* Propagate the event */
	return false;
}

void DisplayConfiguration::load()
{
	kDebug() << "Loading...";

	KScreen::Provider *provider = KScreen::Provider::self();
	if (!provider || !provider->isValid()) {
		return;
	}

	const QString qmlPath = KStandardDirs::locate(
		"data", QLatin1String(QML_PATH "main.qml"));
	m_declarativeView->setSource(qmlPath);

	QDeclarativeItem *rootObj = dynamic_cast<QDeclarativeItem*>(m_declarativeView->rootObject());
	if (!rootObj) {
		kWarning() << "Failed to obtain root item";
		return;
	}

	connect(rootObj, SIGNAL(identifyOutputsRequested()), SLOT(identifyOutputs()));

	QMLOutputView *outputView = rootObj->findChild<QMLOutputView*>("outputView");
	if (!outputView) {
		kWarning() << "Failed to obtain output view";
		return;
	}

	m_config = provider->config();
	Q_FOREACH (KScreen::Output *output, m_config->outputs().values()) {
		outputView->addOutput(m_declarativeView->engine(), output);
	}

	connect(outputView, SIGNAL(changed()), SLOT(changed()));
	connect(outputView, SIGNAL(activeOutputChanged(QMLOutput*)), m_controlPanel, SLOT(setOutput(QMLOutput*)));
}

void DisplayConfiguration::save()
{
	kDebug() << "Saving";

	KScreen::Provider *provider = KScreen::Provider::self();
	if (!provider || !provider->isValid()) {
		return;
	}

	Q_FOREACH(KScreen::Output *output, m_config->outputs()) {
		KScreen::Mode *mode = output->mode(output->currentMode());
		kDebug() << output->name() << "\n"
			 << "	Connected:" << output->isConnected() << "\n"
			 << "	Enabled:" << output->isEnabled() << "\n"
			 << "	Primary:" << output->isPrimary() << "\n"
			 << "	Rotation:" << output->rotation() << "\n"
			 << "	Mode:" << (mode ? mode->name() : "unknown") << "@" << (mode ? mode->refreshRate() : 0.0) << "Hz" << "\n"
			 << "   Position:" << output->pos().x() << "x" << output->pos().y();
	}

	/* Store the current config, apply settings */
	provider->setConfig(m_config);
}

void DisplayConfiguration::clearOutputIdentifiers()
{
	m_outputTimer->stop();
	qDeleteAll(m_outputIdentifiers);
	m_outputIdentifiers.clear();
}


void DisplayConfiguration::identifyOutputs()
{
	const QString qmlPath = KStandardDirs::locate(
		"data", QLatin1String(QML_PATH "OutputIdentifier.qml"));

	m_outputTimer->stop();
	clearOutputIdentifiers();

	/* Obtain the current active configuration from KScreen */
	Provider *screen = Provider::self();
	OutputList outputs = screen->config()->outputs();
	Q_FOREACH (KScreen::Output *output, outputs) {
		if (!output->isConnected() || output->currentMode() == 0) {
			continue;
		}

		Mode *mode = output->mode(output->currentMode());

		QDeclarativeView *view = new QDeclarativeView();
		view->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
		view->setResizeMode(QDeclarativeView::SizeViewToRootObject);
		view->setSource(QUrl::fromLocalFile(qmlPath));

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
