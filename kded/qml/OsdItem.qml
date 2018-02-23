/*
 * Copyright 2014 Martin Klapetek <mklapetek@kde.org>
 * Copyright 2016 Sebastian Kügler <sebas@kde.org>
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

import QtQuick 2.5
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    property QtObject rootItem

    height: Math.min(units.gridUnit * 15, Screen.desktopAvailableHeight / 5)
    width: height

    PlasmaCore.IconItem {
        id: icon
        height: parent.height - label.height - ((units.smallSpacing/2) * 3)
        width: parent.width
        source: rootItem.icon
    }

    PlasmaExtras.Heading {
        id: label
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: Math.floor(units.smallSpacing / 2)
        }

        text: rootItem.infoText
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideLeft
        minimumPointSize: theme.defaultFont.pointSize
        fontSizeMode: Text.HorizontalFit
    }
    Component.onCompleted: print("osditem loaded..." + root.infoText);
}
