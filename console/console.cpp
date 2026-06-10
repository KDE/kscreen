/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "console.h"

#include <KWindowSystem>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTextStream>

#include <kscreen/config.h>
#include <kscreen/configmonitor.h>
#include <kscreen/edid.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/mode.h>
#include <kscreen/screen.h>
#include <kscreen/types.h>

static QTextStream cout(stdout);

namespace KScreen
{
namespace ConfigSerializer
{
// Exported private symbol in configserializer_p.h in KScreen
extern QJsonObject serializeConfig(const KScreen::ConfigPtr &config);
}
}

using namespace KScreen;

Console::Console(const ConfigPtr &config)
    : QObject()
    , m_config(config)
{
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

    connect(m_config.data(), &Config::prioritiesChanged, [&]() {
        qDebug() << "Priorities changed:" << m_config.data()->outputs();
    });

    qDebug() << "Screen:";
    qDebug() << "\tmaxSize:" << m_config->screen()->maxSize();
    qDebug() << "\tminSize:" << m_config->screen()->minSize();
    qDebug() << "\tcurrentSize:" << m_config->screen()->currentSize();

    const OutputList outputs = m_config->outputs();
    for (const OutputPtr &output : outputs) {
        qDebug() << "\n-----------------------------------------------------\n";
        qDebug() << "Id: " << output->id();
        qDebug() << "Name: " << output->name();
        qDebug() << "Type: " << output->typeName();
        qDebug() << "Connected: " << output->isConnected();
        if (!output->isConnected()) {
            continue;
        }
        qDebug() << "Enabled: " << output->isEnabled();
        qDebug() << "Priority: " << output->priority();
        qDebug() << "Rotation: " << output->rotation();
        qDebug() << "Pos: " << output->pos();
        qDebug() << "MMSize: " << output->sizeMm();
        qDebug() << "FollowPreferredMode: " << output->followPreferredMode();
        if (output->currentMode()) {
            qDebug() << "Size: " << output->size();
        }
        qDebug() << "Scale: " << output->scale();
        if (output->clones().isEmpty()) {
            qDebug() << "Clones: "
                     << "None";
        } else {
            qDebug() << "Clones: " << output->clones().count();
        }
        qDebug() << "Mode: " << output->currentModeId();
        qDebug() << "Preferred Mode: " << output->preferredModeId();
        qDebug() << "Preferred modes: " << output->preferredModes();
        qDebug() << "Modes: ";

        const ModeList modes = output->modes();
        for (const ModePtr &mode : modes) {
            qDebug() << "\t" << mode->id() << "  " << mode->name() << " " << mode->size() << " " << mode->refreshRate();
        }

        Edid *edid = output->edid();
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

void Console::printJSONConfig()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/kwinoutputconfig.json"));
    file.open(QFile::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    cout << doc.toJson(QJsonDocument::Indented);
}

void Console::printSerializations()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/kwinoutputconfig.json"));
    file.open(QFile::ReadOnly);
    qDebug().noquote() << file.readAll();
}

void Console::monitor()
{
    ConfigMonitor::instance()->addConfig(m_config);
}

void Console::monitorAndPrint()
{
    monitor();
    connect(ConfigMonitor::instance(), &ConfigMonitor::configurationChanged, this, &Console::printConfig);
}

#include "moc_console.cpp"
