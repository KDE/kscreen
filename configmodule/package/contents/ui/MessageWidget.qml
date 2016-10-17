/*
    Copyright 2016 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

import QtQuick 2.1
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
//import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kscreen 2.0

Rectangle {
    id: messageWidget

    Layout.preferredHeight: Math.max(units.gridUnit * 2, ((label.lineCount -1) * units.gridUnit + units.smallSpacing * 2))
    Layout.fillWidth: true

    property alias text: label.text
    property bool shown: false
    property bool dismissed: false

    color: palette.highlight
    border.color: palette.text
    border.width: Math.ceil(units.gridUnit / 30)

    opacity: (!dismissed && shown) ? 1 : 0
    visible: (opacity > 0);

    Behavior on opacity { NumberAnimation { duration: units.longDuration } }

    RowLayout {
        anchors {
            fill: parent
            margins: units.smallSpacing
            leftMargin: units.smallSpacing * 2
        }
        Label {
            id: label
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: palette.highlightedText
        }
        PlasmaComponents.ToolButton {
            Layout.minimumWidth: units.gridUnit * 2
            iconSource: "window-close"
            onClicked: {
                messageWidget.shown = false
                messageWidget.dismissed = true
            }
        }
    }
}
