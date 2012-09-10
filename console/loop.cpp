
#include "loop.h"

#include "kscreen.h"
#include "config.h"
#include "output.h"
#include "mode.h"

#include <QX11Info>
#include <QtCore/QDebug>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

Loop::Loop(QObject* parent): QObject(parent)
{
    QMetaObject::invokeMethod(this, "start");
}

Loop::~Loop()
{

}

void Loop::start()
{
    qDebug() << "START";
    KScreen* screen = KScreen::self();
    Config* config = screen->config();
        Time time;
        {
            XRRScreenResources* r = XRRGetScreenResources(QX11Info::display(), XRootWindow(QX11Info::display(), 0));
            qDebug() << "RTime: " << r->timestamp;
        }
    qDebug() << "Time: " << XRRTimes(QX11Info::display(), 0, &time);
    qDebug() << "Time: " << time;
//     config->outputs()[65]->setEnabled(true);
//     config->outputs()[65]->setPos(QPoint(0,0));
//     config->outputs()[65]->setPrimary(false);

//     config->outputs()[65]->setCurrentMode(70);

//     config->outputs()[68]->setEnabled(true);
//     config->outputs()[68]->setCurrentMode(70);
    config->outputs()[68]->setPos(QPoint(1920, 0));
//     config->outputs()[68]->setPrimary(true);
    qDebug() << "Setting config";
    screen->setConfig(config);
    qDebug() << "setted";
    XRRScreenResources* r = XRRGetScreenResources(QX11Info::display(), XRootWindow(QX11Info::display(), 0));
    qDebug() << "RTime: " << r->timestamp;
    qDebug() << "Time: " << XRRTimes(QX11Info::display(), 0, &time);
    qDebug() << "Time: " << time;

    printConfig();
}

void Loop::printConfig()
{
    KScreen *screen = KScreen::self();
    qDebug() << "Backend: " << screen->backend();

    Config *config = screen->config();

    OutputList outputs = config->outputs();
    OutputList outputEnabled;
    Q_FOREACH(Output *output, outputs) {
        qDebug() << "Id: " << output->id();
        qDebug() << "Name: " << output->name();
        qDebug() << "Type: " << output->type();
        qDebug() << "Connected: " << output->isConnected();
        qDebug() << "Enabled: " << output->isEnabled();
        qDebug() << "Primary: " << output->isPrimary();
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

        if (output->isEnabled()) {
            outputEnabled.insert(output->id(), output);
        }
        qDebug() << "\n==================================================\n";
    }
}
#include <loop.moc>