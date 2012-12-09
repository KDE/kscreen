
#include "loop.h"

#include "config.h"
#include "output.h"
#include "mode.h"
#include "configmonitor.h"
#include "edid.h"

#include <QX11Info>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

using namespace KScreen;

Loop::Loop(QObject* parent): QObject(parent)
{
    start();
}

Loop::~Loop()
{

}

void Loop::start()
{
    qDebug() << "START";
    QDateTime date = QDateTime::currentDateTime();
    m_config = Config::current();
    qDebug() << "Took: " << date.secsTo(QDateTime::currentDateTime());
    ConfigMonitor::instance()->addConfig(m_config);
    connect(ConfigMonitor::instance(), SIGNAL(configurationChanged()), SLOT(printConfig()));

    //config->outputs()[65]->setCurrentMode(70);
    //Config::setConfig(config);
}

void Loop::printConfig()
{
//     KScreen *screen = KScreen::self();
//     qDebug() << "Backend: " << screen->backend();

    qDebug() << "\n============================================================\n"
                "============================================================\n"
                "============================================================\n";

    qDebug() << "Screen:";
    qDebug() << "maxSize:" << m_config->screen()->maxSize();
    qDebug() << "minSize:" << m_config->screen()->minSize();
    qDebug() << "currentSize:" << m_config->screen()->currentSize();

    OutputList outputs = m_config->outputs();
    OutputList outputEnabled;
    Q_FOREACH(Output *output, outputs) {
        qDebug() << "Id: " << output->id();
        qDebug() << "Name: " << output->name();
        qDebug() << "Type: " << output->type();
        qDebug() << "Connected: " << output->isConnected();
        qDebug() << "Enabled: " << output->isEnabled();
        qDebug() << "Primary: " << output->isPrimary();
        qDebug() << "Rotation: " << output->rotation();
        qDebug() << "Pos: " << output->pos();
        if (output->currentMode()) {
            qDebug() << "Size: " << output->mode(output->currentMode())->size();
        }
        qDebug() << "Clones: " << output->clones().isEmpty();
        qDebug() << "Mode: " << output->currentMode();
        qDebug() << "Modes: ";

        ModeList modes = output->modes();
        Q_FOREACH(Mode* mode, modes) {
            qDebug() << "\t" << mode->id() << "  " << mode->name() << " " << mode->size() << " " << mode->refreshRate();
        }

        Edid* edid = output->edid();
	qDebug() << "EDID Info: ";
	if (edid != 0) {
	    qDebug() << "\tDevice ID: " << edid->deviceId();
	    qDebug() << "\tName: " << edid->name();
	    qDebug() << "\tVendor: " << edid->vendor();
	    qDebug() << "\tSerial: " << edid->serial();
	    qDebug() << "\tEISA ID: " << edid->eisaId();
	    qDebug() << "\tHash: " << edid->hash();
	    qDebug() << "\tWidth: " << edid->width();
	    qDebug() << "\tHeight: " << edid->height();
	    qDebug() << "\tGamma: " << edid->gamma();
	    qDebug() << "\tRed: " << edid->red();
	    qDebug() << "\tGreen: " << edid->green();
	    qDebug() << "\tBlue: " << edid->blue();
	    qDebug() << "\tWhite: " << edid->white();
	} else {
	    qDebug() << "\tUnavailable";
	}

        if (output->isEnabled()) {
            outputEnabled.insert(output->id(), output);
        }
        qDebug() << "\n-----------------------------------------------------\n";
    }
}
#include <loop.moc>