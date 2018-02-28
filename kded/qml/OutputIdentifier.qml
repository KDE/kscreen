/*
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
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ColumnLayout {

    property QtObject rootItem

    property string outputName: rootItem ? rootItem.outputName : ""
    property string modeName: rootItem ? rootItem.modeName : ""

    PlasmaComponents.Label {
        id: displayName

        Layout.maximumWidth: Screen.width * 0.8
        Layout.maximumHeight: Screen.height * 0.8
        Layout.margins: units.largeSpacing
        Layout.bottomMargin: units.smallSpacing

        text: root.outputName
        font.pointSize: theme.defaultFont.pointSize * 3
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideLeft
    }

    PlasmaComponents.Label {
        id: modeLabel;

        Layout.fillWidth: true
        Layout.bottomMargin: units.largeSpacing

        text: root.modeName;
        horizontalAlignment: Text.AlignHCenter;
    }
}
