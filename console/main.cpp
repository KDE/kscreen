/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include <QtGui/QApplication>

#include "kscreen.h"
#include "config.h"
#include "output.h"
#include "mode.h"

#include <caca.h>

#include <QtCore/QDebug>

int main (int argc, char *argv[])
{
    QApplication app(argc, argv);

    setenv("KSCREEN_BACKEND", "XRandR", 1);

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

//     config->outputs()[65]->setEnabled(false);
//     config->outputs()[65]->setPos(QPoint(0,0));
//     config->outputs()[65]->setPrimary(false);

//     config->outputs()[65]->setEnabled(true);
//     config->outputs()[65]->setCurrentMode(70);
//     config->outputs()[65]->setPos(QPoint(0,0));
//     config->outputs()[68]->setEnabled(true);
//     config->outputs()[68]->setCurrentMode(70);
//     config->outputs()[68]->setPos(QPoint(1920, 0));
//     config->outputs()[68]->setPrimary(true);

//     screen->setConfig(config);
}