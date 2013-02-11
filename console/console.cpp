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

#include "console.h"

#include <QX11Info>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QTextStream>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/mode.h>
#include <kscreen/configmonitor.h>
#include <kscreen/edid.h>

#include <KStandardDirs>

using namespace KScreen;

Console::Console(QObject* parent): QObject(parent)
{
    qDebug() << "START";
    QDateTime date = QDateTime::currentDateTime();
    m_config = Config::current();
    qDebug() << "Config::current() took" << date.msecsTo(QDateTime::currentDateTime()) << "milliseconds";
}

Console::~Console()
{

}

void Console::printConfig()
{
    if (!m_config) {
        qDebug() << "Config is invalid, probably backend couldn't load";
        return;
    }
    if (!m_config->screen()) {
        qDebug() << "No screen in the configuration, broken backend";
        return;
    }

    qDebug() << "Screen:";
    qDebug() << "maxSize:" << m_config->screen()->maxSize();
    qDebug() << "minSize:" << m_config->screen()->minSize();
    qDebug() << "currentSize:" << m_config->screen()->currentSize();

    OutputList outputs = m_config->outputs();
    Q_FOREACH(Output *output, outputs) {
        qDebug() << "\n-----------------------------------------------------\n";
        qDebug() << "Id: " << output->id();
        qDebug() << "Name: " << output->name();
        qDebug() << "Type: " << output->type();
        qDebug() << "Connected: " << output->isConnected();
        qDebug() << "Enabled: " << output->isEnabled();
        qDebug() << "Primary: " << output->isPrimary();
        qDebug() << "Rotation: " << output->rotation();
        qDebug() << "Pos: " << output->pos();
        if (output->currentMode()) {
            qDebug() << "Size: " << output->currentMode()->size();
        }
        qDebug() << "Clones: " << output->clones().isEmpty();
        qDebug() << "Mode: " << output->currentModeId();
        qDebug() << "Preferred modes: " << output->preferredModes();
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
    }
}

void Console::printSerializations()
{
    QString path = KStandardDirs::locateLocal("data", "kscreen/");
    qDebug() << "Configs in: " << path;

    QDir dir(path);
    QStringList files = dir.entryList(QDir::Files);
    qDebug() << "Number of files: " << files.count() << endl;

    bool ok;
    QJson::Parser parser;
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull);
    Q_FOREACH(const QString fileName, files) {
        ok = true;
        qDebug() << fileName;
        QFile file(path + "/" + fileName);
        file.open(QFile::ReadOnly);
        QVariant data = parser.parse(file.readAll(), &ok);
        if (!ok) {
            qDebug() << "    " << "can't parse file";
            qDebug() << "    " << parser.errorString();
            continue;
        }

        qDebug() << serializer.serialize(data) << endl;
    }
}

void Console::monitor()
{
    ConfigMonitor::instance()->addConfig(m_config);
}

void Console::monitorAndPrint()
{
    monitor();
    connect(ConfigMonitor::instance(), SIGNAL(configurationChanged()), SLOT(printConfig()));
}

#include <console.moc>
