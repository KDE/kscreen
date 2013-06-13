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
    qDebug() << "KScreen version: " << KSCREEN_VERSION;

    if (!m_config) {
        qDebug() << "Config is invalid, probably backend couldn't load";
        return;
    }
    if (!m_config->screen()) {
        qDebug() << "No screen in the configuration, broken backend";
        return;
    }

    qDebug() << "Screen:";
    qDebug() << "\tmaxSize:" << m_config->screen()->maxSize();
    qDebug() << "\tminSize:" << m_config->screen()->minSize();
    qDebug() << "\tcurrentSize:" << m_config->screen()->currentSize();

    OutputList outputs = m_config->outputs();
    Q_FOREACH(Output *output, outputs) {
        qDebug() << "\n-----------------------------------------------------\n";
        qDebug() << "Id: " << output->id();
        qDebug() << "Name: " << output->name();
        qDebug() << "Type: " << typetoString(output->type());
        qDebug() << "Connected: " << output->isConnected();
        if (!output->isConnected()) {
            continue;
        }
        qDebug() << "Enabled: " << output->isEnabled();
        qDebug() << "Primary: " << output->isPrimary();
        qDebug() << "Rotation: " << output->rotation();
        qDebug() << "Pos: " << output->pos();
        if (output->currentMode()) {
            qDebug() << "Size: " << output->currentMode()->size();
        }
        if (output->clones().isEmpty()) {
            qDebug() << "Clones: " << "None";
        } else {
            qDebug() << "Clones: " << output->clones().count();
        }
        qDebug() << "Mode: " << output->currentModeId();
        qDebug() << "Preferred Mode: " << output->preferredModeId();
        qDebug() << "Preferred modes: " << output->preferredModes();
        qDebug() << "Modes: ";

        ModeList modes = output->modes();
        Q_FOREACH(Mode* mode, modes) {
            qDebug() << "\t" << mode->id() << "  " << mode->name() << " " << mode->size() << " " << mode->refreshRate();
        }

        Edid* edid = output->edid();
        qDebug() << "EDID Info: ";
        if (edid && edid->isValid()) {
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

QString Console::typetoString(const Output::Type& type) const
{
    switch (type) {
        case Output::Unknown:
            return QLatin1String("Unknown");
        case Output::Panel:
            return QLatin1String("Panel (Laptop)");
        case Output::VGA:
            return QLatin1String("VGA");
        case Output::DVII:
            return QLatin1String("DVI-I");
        case Output::DVIA:
            return QLatin1String("DVI-A");
        case Output::DVID:
            return QLatin1String("DVI-D");
        case Output::HDMI:
            return QLatin1String("HDMI");
        case Output::TV:
            return QLatin1String("TV");
        case Output::TVComposite:
            return QLatin1String("TV-Composite");
        case Output::TVSVideo:
            return QLatin1String("TV-SVideo");
        case Output::TVComponent:
            return QLatin1String("TV-Component");
        case Output::TVSCART:
            return QLatin1String("TV-SCART");
        case Output::TVC4:
            return QLatin1String("TV-C4");
        case Output::DisplayPort:
            return QLatin1String("DisplayPort");
        default:
            return QLatin1String("Invalid Type");

    };
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
