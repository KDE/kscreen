
#include "loop.h"

#include "config.h"
#include "output.h"
#include "mode.h"

#include <QX11Info>
#include <QtCore/QDebug>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

using namespace KScreen;

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
    Config* config = Config::current();
    config->outputs()[65]->setCurrentMode(70);
    qDebug() << "Setting config";
    Config::setConfig(config);
    qDebug() << "setted";
    printConfig();
}

void Loop::printConfig()
{
//     KScreen *screen = KScreen::self();
//     qDebug() << "Backend: " << screen->backend();

    Config *config = Config::current();
    qDebug() << "Screen:";
    qDebug() << "maxSize:" << config->screen()->maxSize();
    qDebug() << "minSize:" << config->screen()->minSize();
    qDebug() << "currentSize:" << config->screen()->currentSize();

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