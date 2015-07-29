/*
 * Copyright 2013  Dan Vratil <dvratil@redhat.com>
 * Copyright 2015  Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kscreenapplet.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QMessageBox>

#include <KToolInvocation>
#include <KLocalizedString>

#include <kscreen/edid.h>
#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

bool leftPos(KScreen::OutputPtr output1, KScreen::OutputPtr output2) {
    return (output1->pos().x() < output2->pos().x());
}

KScreenApplet::KScreenApplet(QObject *parent):
    QObject(parent),
    m_hasNewOutput(false)
{
    m_resetTimer = new QTimer(this);

    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished, 
            this, &KScreenApplet::slotConfigReady);

    KScreen::ConfigMonitor *configMonitor = KScreen::ConfigMonitor::instance();
    connect(configMonitor, SIGNAL(configurationChanged()),
            this, SLOT(slotConfigurationChanged()));
}

KScreenApplet::~KScreenApplet()
{
}

void KScreenApplet::slotConfigReady(KScreen::ConfigOperation* op)
{
    if (op->hasError())
        return;

    m_config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();

    slotConfigurationChanged();

    init();
}

void KScreenApplet::init()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    bool conn = connection.connect(QLatin1String("org.kde.kded"),
                                   QLatin1String("/modules/kscreen"),
                                   QLatin1String("org.kde.KScreen"),
                                   QLatin1String("unknownOutputConnected"),
                                   //QLatin1String("outputConnected"),
                                   QLatin1String("s"),
                                   this, SLOT(slotUnknownDisplayConnected(QString)));
    if (!conn) {
        QMessageBox msgBox(QMessageBox::Warning, 
            i18n("Warning"), i18n("fail to connect kscreen kded dbus"));
        msgBox.exec();
    }
}

void KScreenApplet::slotUnknownDisplayConnected(const QString &outputName)
{
    qDebug() << "New display connected to output" << outputName;
    m_newOutputName = outputName;

    QString displayName;
    KScreen::OutputPtr newOutput = outputForName(outputName);
    KScreen::Edid *edid = newOutput->edid();
    if (!edid) {
        displayName = outputName;
    } else {
        displayName = edid->vendor() + QLatin1String(" ") + edid->name();
    }

    m_displayName = displayName;
    emit displayNameChanged();

    m_hasNewOutput = true;

    // Show the notification for only 20 seconds, should be enough...
    m_resetTimer->singleShot(20000, this, SLOT(slotResetApplet()));
}

void KScreenApplet::applyAction(int actionId)
{
    DisplayAction action = (DisplayAction) actionId;
    qDebug() << "Applying changes" << action;

    if (action == ActionNone) {
        qDebug() << "Action: None";
        slotResetApplet();
        return;
    }

    KScreen::OutputPtr newOutput = outputForName(m_newOutputName);
    qDebug() << "Output for" << m_newOutputName << ":" << newOutput;

    if (newOutput == 0) {
        slotResetApplet();
        return;
    }

    if (action == ActionDisable) {
        qDebug() << "Action: Disable";
        newOutput->setEnabled(false);
        slotResetApplet();
        return;
    }

    newOutput->setEnabled(true);
    newOutput->setCurrentModeId(newOutput->preferredModeId());
    KScreen::ModePtr newMode = newOutput->currentMode();
    qDebug() << "It's mode is" << newMode;

    // Only take enabled outputs, order them from left to right
    KScreen::OutputList allOutputs = m_config->outputs();
    KScreen::OutputList::Iterator iter;
    QList<KScreen::OutputPtr> outputs;
    for (iter = allOutputs.begin(); iter != allOutputs.end(); ++iter) {
        KScreen::OutputPtr output = iter.value();
        if (output->isConnected() && output->isEnabled()) {
            outputs << output;
        }
    }
    qSort(outputs.begin(), outputs.end(), &leftPos);

    if (action == ActionClone) {
        qDebug() << "Action: Clone";
        /* Set the new output as a clone of the primary output */
        KScreen::OutputPtr primary = m_config->primaryOutput();
        if (!primary || primary == newOutput) {
            primary = outputs.first();
            if (primary == newOutput) {
                primary = outputs.at(1);
            }
        }
        newOutput->setPos(primary->pos());
        QList<int> clones = primary->clones();
        clones << newOutput->id();
        primary->setClones(clones);
    } else if (action == ActionExtendLeft) {
        qDebug() << "Action: ExtendLeft";
        int globalWidth = newMode->size().width();
        newOutput->setPos(QPoint(0, 0));
        Q_FOREACH(KScreen::OutputPtr output, outputs) {
            if (!output->isConnected() || !output->isEnabled() || (output == newOutput)) {
                continue;
            }

            QPoint pos = output->pos();
            pos.setX(globalWidth);
            output->setPos(pos);
            globalWidth += output->currentMode()->size().width();
        }

    } else if (action == ActionExtendRight) {
        qDebug() << "Action: ExtendRight";
        int globalWidth = 0;
        Q_FOREACH(KScreen::OutputPtr output, outputs) {
            if (!output->isConnected() || !output->isEnabled() || (output == newOutput)) {
                continue;
            }

            QPoint pos = output->pos();
            pos.setX(globalWidth);
            output->setPos(pos);
            globalWidth += output->currentMode()->size().width();
        }
        newOutput->setPos(QPoint(globalWidth, 0));
    }

    /* Update the settings */
    Q_FOREACH(KScreen::OutputPtr output, outputs) {
        if (!output->isEnabled()) {
            continue;
        }

        qDebug() << output->name();
        qDebug() << "\tSize:" << output->currentMode()->size();
        qDebug() << "\tPos:" << output->pos();
        qDebug() << "\tClones:" << output->clones();
    }

    if (!KScreen::Config::canBeApplied(m_config)) {
        QMessageBox msgBox(QMessageBox::Warning, 
            i18n("Warning"), i18n("Invalid KScreen configuration!"));
        msgBox.exec();
    }

    auto *op = new KScreen::SetConfigOperation(m_config);
    op->exec();

    slotResetApplet();
}

void KScreenApplet::runKCM()
{
    KToolInvocation::kdeinitExec(
        QLatin1String("kcmshell5"),
        QStringList() << QLatin1String("kscreen"));
}

void KScreenApplet::slotResetApplet()
{
    m_hasNewOutput = false;
    m_newOutputName.clear();
}

void KScreenApplet::slotConfigurationChanged()
{
    // TODO: if external monitor has been already plugged in before KDE boot
    // just use !primary output as the external one
    if (m_newOutputName == "") {
        for (KScreen::OutputPtr &output : m_config->outputs()) {
            if (!output->isConnected())
                continue;

            if (output != m_config->primaryOutput()) {
                m_newOutputName = output->name();
                KScreen::Edid *edid = output->edid();
                m_displayName = edid->vendor() + QLatin1String(" ") + edid->name();
                emit displayNameChanged();
            }
        }
    }

    m_connectedOutputs = m_config.isNull() ? 0 : m_config->connectedOutputs().count();
    emit connectedOutputsChanged();
}

KScreen::OutputPtr KScreenApplet::outputForName(const QString &name)
{
    KScreen::OutputList outputs = m_config->outputs();
    KScreen::OutputList::Iterator iter;
    for (iter = outputs.begin(); iter != outputs.end(); ++iter) {
        KScreen::OutputPtr output = iter.value();

        if (output->name() == name) {
            return output;
        }
    }

    return KScreen::OutputPtr(nullptr);
}

#include "kscreenapplet.moc"
