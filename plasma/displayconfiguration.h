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

#ifndef DISPLAYCONFIGURATION_H
#define DISPLAYCONFIGURATION_H

#include <Plasma/PopupApplet>

namespace Plasma
{
class DeclarativeWidget;
}

class QGraphicsWidget;

class DisplayConfiguration : public Plasma::PopupApplet
{
    Q_OBJECT
    Q_ENUMS(DisplayAction)

  public:
    enum DisplayAction {
        ActionNone = 0,
        ActionExtendRight,
        ActionExtendLeft,
        ActionExtendAbove,
        ActionExtendBelow,
        ActionClone,
    };

    DisplayConfiguration();
    DisplayConfiguration(QObject *parent, const QVariantList &args);
    virtual ~DisplayConfiguration();

    virtual void init();
    virtual QGraphicsWidget *graphicsWidget();

  private Q_SLOTS:
    void slotUnknownDisplayConnected(const QString &output);
    void slotRunKCM();
    void slotApplyAction(int actionId);

  private:
    void initDeclarativeWidget();

    Plasma::DeclarativeWidget *m_declarativeWidget;
    bool m_hasNewOutput;

};

K_EXPORT_PLASMA_APPLET(org.kde.plasma.kscreen, DisplayConfiguration)

#endif // DISPLAYCONFIGURATION_H
