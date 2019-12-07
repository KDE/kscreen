/*
 * Copyright (c) 2018 Kai Uwe Broulik <kde@broulik.de>
 *                    Work sponsored by the LiMux project of
 *                    the city of Munich.
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

#pragma once

#include <Plasma/Applet>

#include <KScreen/Types>

class KScreenApplet : public Plasma::Applet
{
    Q_OBJECT

    /**
     * The number of currently connected (not necessarily enabled) outputs
     */
    Q_PROPERTY(int connectedOutputCount READ connectedOutputCount NOTIFY connectedOutputCountChanged)

public:
    explicit KScreenApplet(QObject *parent, const QVariantList &data);
    ~KScreenApplet() override;

    enum Action {
        SwitchToExternal,
        SwitchToInternal,
        Clone,
        ExtendLeft,
        ExtendRight
    };
    Q_ENUM(Action)

    void init() override;

    int connectedOutputCount() const;

    Q_INVOKABLE void applyLayoutPreset(Action action);

Q_SIGNALS:
    void connectedOutputCountChanged();

private:
    void checkOutputs();

    KScreen::ConfigPtr m_screenConfiguration;
    int m_connectedOutputCount = 0;

};
