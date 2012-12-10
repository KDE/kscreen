/*
 * Copyright 2012  Dan Vratil <dvratil@redhat.com>
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

#include "displayconfiguration.h"

#include <QDeclarativeItem>

#include <QDBusConnection>
#include <QDBusInterface>

#include <Plasma/Package>
#include <Plasma/DeclarativeWidget>
#include <KToolInvocation>

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/edid.h>

DisplayConfiguration::DisplayConfiguration(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args)
    , m_declarativeWidget(0)
    , m_hasNewOutput(true) /* FIXME RELEASE - this will be FALSE by default! */
{
    qmlRegisterType<DisplayConfiguration>("org.kde.kscreen", 1, 0, "DisplayConfiguration");
    
    setenv("KSCREEN_BACKEND", "XRandR", false);
}

DisplayConfiguration::DisplayConfiguration():
    PopupApplet(0, QVariantList())
{

}


DisplayConfiguration::~DisplayConfiguration()
{
}

void DisplayConfiguration::init()
{
    QDBusConnection connection = QDBusConnection::systemBus();
    bool conn = connection.connect(QLatin1String("org.kde.kded"),
                                   QLatin1String("modules/kscreen"),
                                   QLatin1String("org.kde.kscreen"),
                                   QLatin1String("unkownDisplayConnected"),
                                   QLatin1String("s"),
                                   this, SLOT(slotUnknownDisplayConnected(QString)));
    /* FIXME RELEASE
    if (!conn) {
        setFailedToLaunch(true, i18n("Failed to connect to KScreen KDED daemon"));
    }
    */
}

void DisplayConfiguration::initDeclarativeWidget()
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


QGraphicsWidget *DisplayConfiguration::graphicsWidget()
{
    if (hasFailedToLaunch() || !m_hasNewOutput) {
        return 0;
    }

    if (!m_declarativeWidget) {
        initDeclarativeWidget();
    }

    return m_declarativeWidget;
}

void DisplayConfiguration::slotUnknownDisplayConnected(const QString &outputName)
{
    kDebug() << "New display connected to output" << outputName;
    m_newOutputName = outputName;

    QString displayName;
    KScreen::Output *newOutput = outputForName(outputName);
    KScreen::Edid *edid = newOutput->edid();
    if (!edid) {
        displayName = outputName;
    } else {
        displayName = edid->name();
    }

    QDeclarativeItem *rootObject = qobject_cast<QDeclarativeItem*>(m_declarativeWidget->rootObject());
    rootObject->setProperty("displayName", displayName);

    m_hasNewOutput = true;
    showPopup();
}

void DisplayConfiguration::slotApplyAction(int actionId)
{
    DisplayAction action = (DisplayAction) actionId;
    kDebug() << "Applying changes" << action;

    if (action == ActionNone) {
        m_hasNewOutput = false;
        m_newOutputName.clear();
        hidePopup();
        return;
    }

    KScreen::Output *newOutput = outputForName(m_newOutputName);

    if (newOutput == 0) {
        m_hasNewOutput = false;
        m_newOutputName.clear();
        hidePopup();
        return;
    }

    newOutput->setEnabled(true);
    newOutput->setCurrentMode(newOutput->preferredMode());
    KScreen::Mode *newMode = newOutput->mode(newOutput->currentMode());

    KScreen::Config *config = KScreen::Config::current();
    KScreen::OutputList outputs = config->outputs();
    KScreen::OutputList::Iterator iter;
    if (action == ActionClone) {
        for (iter = outputs.begin(); iter != outputs.end(); ++iter) {
            KScreen::Output *output = iter.value();
            if (!output->isConnected() || !output->isEnabled() || (output == newOutput)) {
                continue;
            }

            /* Set the new output as a clone of the first connected output in the list */
            QList<int> clones = output->clones();
            clones << newOutput->id();
            output->setClones(clones);
            break;
        }

    } else if (action == ActionExtendLeft) {
        newOutput->setPos(QPoint(0, 0));
        for (iter = outputs.begin(); iter != outputs.end(); ++iter) {
            KScreen::Output *output = iter.value();
            if (!output->isConnected() || !output->isEnabled() || (output == newOutput)) {
                continue;
            }

            QPoint pos = output->pos();
            pos.setX(pos.x() + newMode->size().width());
            output->setPos(pos);
        }

    } else if (action == ActionExtendRight) {
        int offset = 0;
        for (iter = outputs.begin(); iter != outputs.end(); ++iter) {
            KScreen::Output *output = iter.value();
            if (!output->isConnected() || !output->isEnabled() || (output == newOutput)) {
                continue;
            }

            offset += output->mode(output->currentMode())->size().width();
        }
        newOutput->setPos(QPoint(offset, 0));
    }

    /* Update the settings */
    KScreen::Config::setConfig(config);

    m_hasNewOutput = false;
    m_newOutputName.clear();
    hidePopup();
}

void DisplayConfiguration::slotRunKCM()
{
    KToolInvocation::kdeinitExec(
        QLatin1String("kcmshell4"),
        QStringList() << QLatin1String("displayconfiguration"));

    hidePopup();
}

KScreen::Output *DisplayConfiguration::outputForName(const QString &name)
{
    KScreen::Config *config = KScreen::Config::current();
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



#include "displayconfiguration.moc"
