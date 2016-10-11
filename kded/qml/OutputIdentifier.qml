/*
 * Copyright 2014 Martin Klapetek <mklapetek@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtra
import QtQuick.Window 2.2

Item {
    property QtObject rootItem
    id: root;

    property string outputName: rootItem ? rootItem.outputName : ""
    property string modeName: rootItem ? rootItem.modeName : ""

    width: childrenRect.width + 2 * childrenRect.x
    height: childrenRect.height + 2 * childrenRect.y

    PlasmaComponents.Label {
        id: displayName
        x: units.largeSpacing * 2
        y: units.largeSpacing
        font.pointSize: theme.defaultFont.pointSize * 3
        text: root.outputName;
        wrapMode: Text.WordWrap;
        horizontalAlignment: Text.AlignHCenter;
    }

    PlasmaComponents.Label {
        id: modeLabel;
        anchors {
            horizontalCenter: displayName.horizontalCenter
            top: displayName.bottom
        }
        text: root.modeName;
        horizontalAlignment: Text.AlignHCenter;
    }
}
