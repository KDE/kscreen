/*
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "osdmanager.h"
#include "osd.h"

#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>

#include <QDBusConnection>
#include <QLoggingCategory>
#include <QDebug>

namespace KScreen {

OsdManager* OsdManager::m_instance = 0;

OsdManager::OsdManager(QObject *parent)
    : QObject(parent)
{
}

OsdManager::~OsdManager()
{
}

OsdManager* OsdManager::self()
{
    if (!OsdManager::m_instance) {
        m_instance = new OsdManager();
    }

    return m_instance;
}

void OsdManager::showOutputIdentifiers()
{
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, &OsdManager::slotIdentifyOutputs);
}

void OsdManager::slotIdentifyOutputs(KScreen::ConfigOperation *op)
{
    if (op->hasError()) {
        return;
    }

    const KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();

    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        connect(output.data(), &KScreen::Output::isConnectedChanged,
                this, [this](){
                    KScreen::Output *output = qobject_cast<KScreen::Output*>(sender());
                    qDebug() << "outputConnectedChanged():" << output->name();
                    if (!output->isConnected() || !output->isEnabled() || !output->currentMode()) {
                        KScreen::Osd* osd = nullptr;
                        if (m_osds.keys().contains(output->name())) {
                            osd->deleteLater();
                            m_osds.remove(output->name());
                        }
                    }
                }, Qt::UniqueConnection);

        if (!output->isConnected() || !output->isEnabled() || !output->currentMode()) {
            continue;
        }

        KScreen::Osd* osd = nullptr;
        qDebug() << "output:" << output->name() << m_osds;
        if (m_osds.keys().contains(output->name())) {
            osd = m_osds.value(output->name());
        } else {
            osd = new KScreen::Osd(this);
            m_osds.insert(output->name(), osd);
        }
        osd->showOutputIdentifier(output);
    }
}


}
