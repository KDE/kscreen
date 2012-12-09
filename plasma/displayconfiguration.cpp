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

DisplayConfiguration::DisplayConfiguration(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args)
    , m_declarativeWidget(0)
    , m_hasNewOutput(true) /* FIXME RELEASE - this will be FALSE by default! */
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
    connect(rootObject, SIGNAL(applyAction(DisplayAction)), SLOT(slotApplyAction(DisplayAction)));
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

void DisplayConfiguration::slotUnknownDisplayConnected(const QString &output)
{
    kDebug() << "New display connected to output" << output;

    m_hasNewOutput = true;
    showPopup();
}

void DisplayConfiguration::slotApplyAction(DisplayConfiguration::DisplayAction action)
{
    kDebug() << "Applying changes";

    hidePopup();
}

void DisplayConfiguration::slotRunKCM()
{
    KToolInvocation::kdeinitExec(
        QLatin1String("kcmshell4"),
        QStringList() << QLatin1String("displayconfiguration"));

    hidePopup();
}


#include "displayconfiguration.moc"
