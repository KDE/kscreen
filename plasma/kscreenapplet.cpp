/*
 * Copyright 2013  Dan Vratil <dvratil@redhat.com>
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

#include <QTimer>
#include <QDeclarativeItem>
#include <QGraphicsSceneMouseEvent>

#include <QDBusConnection>
#include <QDBusInterface>

#include <Plasma/Package>
#include <Plasma/DeclarativeWidget>
#include <KToolInvocation>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>
#include <kscreen/configmonitor.h>

bool leftPos(KScreen::Output* output1, KScreen::Output* output2) {
    return (output1->pos().x() < output2->pos().x());
}

KScreenApplet::KScreenApplet(QObject *parent, const QVariantList &args):
    Plasma::PopupApplet(parent, args),
    m_declarativeWidget(0),
    m_hasNewOutput(false)
{
    qmlRegisterType<KScreenApplet>("org.kde.kscreen", 1, 0, "KScreenApplet");
    setPopupIcon(QLatin1String("video-display"));

    m_resetTimer = new QTimer(this);

    KScreen::ConfigMonitor *configMonitor = KScreen::ConfigMonitor::instance();
    connect(configMonitor, SIGNAL(configurationChanged()),
            this, SLOT(slotConfigurationChanged()));

    slotConfigurationChanged();
}

KScreenApplet::KScreenApplet():
    PopupApplet(0, QVariantList())
{

}

KScreenApplet::~KScreenApplet()
{
}

void KScreenApplet::init()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    bool conn = connection.connect(QLatin1String("org.kde.kded"),
                                   QLatin1String("/modules/kscreen"),
                                   QLatin1String("org.kde.KScreen"),
                                   QLatin1String("unkownOutputConnected"),
                                   //QLatin1String("outputConnected"),
                                   QLatin1String("s"),
                                   this, SLOT(slotUnknownDisplayConnected(QString)));
    if (!conn) {
        setFailedToLaunch(true, i18n("Failed to connect to KScreen daemon"));
    }
}

void KScreenApplet::initDeclarativeWidget()
{
    m_declarativeWidget = new Plasma::DeclarativeWidget(this);

    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    Plasma::Package package(QString(), "org.kde.plasma.kscreen.qml", structure);
    m_declarativeWidget->setQmlPath(package.filePath("mainscript"));

    QDeclarativeItem *rootObject = qobject_cast<QDeclarativeItem*>(m_declarativeWidget->rootObject());
    if (!rootObject) {
        setFailedToLaunch(true, i18n("Failed to load root object"));
        return;
    }

    connect(rootObject, SIGNAL(runKCM()), SLOT(slotRunKCM()));
    connect(rootObject, SIGNAL(applyAction(int)), SLOT(slotApplyAction(int)));
}


QGraphicsWidget *KScreenApplet::graphicsWidget()
{
    if (hasFailedToLaunch()) {
        return 0;
    }

    if (!m_declarativeWidget) {
        initDeclarativeWidget();
    }

    return m_declarativeWidget;
}

void KScreenApplet::slotUnknownDisplayConnected(const QString &outputName)
{
    kDebug() << "New display connected to output" << outputName;
    m_newOutputName = outputName;

    QString displayName;
    KScreen::Output *newOutput = outputForName(outputName, KScreen::Config::current());
    KScreen::Edid *edid = newOutput->edid();
    if (!edid) {
        displayName = outputName;
    } else {
        displayName = edid->vendor() + QLatin1String(" ") + edid->name();
    }

    QDeclarativeItem *rootObject = qobject_cast<QDeclarativeItem*>(m_declarativeWidget->rootObject());
    rootObject->setProperty("displayName", displayName);

    m_hasNewOutput = true;
    showPopup();

    // Show the notification for only 20 seconds, should be enough...
    m_resetTimer->singleShot(20000, this, SLOT(slotResetApplet()));
}

void KScreenApplet::slotApplyAction(int actionId)
{
    DisplayAction action = (DisplayAction) actionId;
    kDebug() << "Applying changes" << action;

    if (action == ActionNone) {
        kDebug() << "Action: None";
        slotResetApplet();
        return;
    }

    KScreen::Config *config = KScreen::Config::current();
    KScreen::Output *newOutput = outputForName(m_newOutputName, config);
    kDebug() << "Output for" << m_newOutputName << ":" << newOutput;

    if (newOutput == 0) {
        slotResetApplet();
        return;
    }

    if (action == ActionDisable) {
        kDebug() << "Action: Disable";
        newOutput->setEnabled(false);
        slotResetApplet();
        return;
    }

    newOutput->setEnabled(true);
    newOutput->setCurrentModeId(newOutput->preferredModeId());
    KScreen::Mode *newMode = newOutput->currentMode();
    kDebug() << "It's mode is" << newMode;

    // Only take enabled outputs, order them from left to right
    KScreen::OutputList allOutputs = config->outputs();
    KScreen::OutputList::Iterator iter;
    QList<KScreen::Output*> outputs;
    for (iter = allOutputs.begin(); iter != allOutputs.end(); ++iter) {
        KScreen::Output *output = iter.value();
        if (output->isConnected() && output->isEnabled()) {
            outputs << output;
        }
    }
    qSort(outputs.begin(), outputs.end(), &leftPos);

    if (action == ActionClone) {
        kDebug() << "Action: Clone";
        /* Set the new output as a clone of the primary output */
        KScreen::Output *primary = config->primaryOutput();
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
        kDebug() << "Action: ExtendLeft";
        int globalWidth = newMode->size().width();
        newOutput->setPos(QPoint(0, 0));
        Q_FOREACH(KScreen::Output *output, outputs) {
            if (!output->isConnected() || !output->isEnabled() || (output == newOutput)) {
                continue;
            }

            QPoint pos = output->pos();
            pos.setX(globalWidth);
            output->setPos(pos);
            globalWidth += output->currentMode()->size().width();
        }

    } else if (action == ActionExtendRight) {
        kDebug() << "Action: ExtendRight";
        int globalWidth = 0;
        Q_FOREACH(KScreen::Output *output, outputs) {
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
    Q_FOREACH(KScreen::Output *output, outputs) {
        if (!output->isEnabled()) {
            continue;
        }

        kDebug() << output->name();
        kDebug() << "\tSize:" << output->currentMode()->size();
        kDebug() << "\tPos:" << output->pos();
        kDebug() << "\tClones:" << output->clones();
    }

    KScreen::Config::setConfig(config);
    slotResetApplet();
}

void KScreenApplet::slotRunKCM()
{
    KToolInvocation::kdeinitExec(
        QLatin1String("kcmshell4"),
        QStringList() << QLatin1String("kscreen"));

    hidePopup();
}

void KScreenApplet::slotResetApplet()
{
    m_hasNewOutput = false;
    m_newOutputName.clear();
    hidePopup();
}

void KScreenApplet::slotConfigurationChanged()
{
    KScreen::Config *config = KScreen::Config::current();
    if (!config || !config->isValid()) {
        setStatus(Plasma::PassiveStatus);
        return;
    }

    if (config->connectedOutputs().count() > 1) {
        setStatus(Plasma::ActiveStatus);
    } else {
        setStatus(Plasma::PassiveStatus);
    }
}

void KScreenApplet::popupEvent(bool show)
{
    if (show && !m_hasNewOutput) {
        slotRunKCM();
        return;
    }

    Plasma::PopupApplet::popupEvent(show);
}

KScreen::Output *KScreenApplet::outputForName(const QString &name, KScreen::Config *config)
{
    KScreen::OutputList outputs = config->outputs();
    KScreen::OutputList::Iterator iter;
    for (iter = outputs.begin(); iter != outputs.end(); ++iter) {
        KScreen::Output *output = iter.value();

        if (output->name() == name) {
            return output;
        }
    }

    return 0;
}

#include "kscreenapplet.moc"
