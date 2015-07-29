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

#ifndef KSCREENAPPLET_H
#define KSCREENAPPLET_H

#include <QObject>
#include <QTimer>

#include <kscreen/output.h>
#include <kscreen/config.h>
#include <kscreen/configoperation.h>

class KScreenApplet : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_ENUMS(DisplayAction)

public:
    enum DisplayAction {
        ActionNone = 0,
        ActionExtendRight,
        ActionExtendLeft,
        ActionClone,
        ActionDisable,
    };

    KScreenApplet(QObject *parent = nullptr);
    virtual ~KScreenApplet();

    QString displayName() const { return m_displayName; }

    Q_INVOKABLE void runKCM();
    Q_INVOKABLE void applyAction(int actionId);

    virtual void init();

Q_SIGNALS:
    void displayNameChanged();

private Q_SLOTS:
    void slotConfigReady(KScreen::ConfigOperation *op);
    void slotUnknownDisplayConnected(const QString &output);
    void slotResetApplet();
    void slotConfigurationChanged();

private:
    KScreen::OutputPtr outputForName(const QString &name);

    KScreen::ConfigPtr m_config;
    bool m_hasNewOutput;
    QString m_newOutputName;
    QTimer *m_resetTimer;
    QString m_displayName = "UNKNOWN";
};

#endif // KSCREENAPPLET_H
