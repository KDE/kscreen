/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 2.9
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.4 as Controls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.private.kcm.kscreen 1.0 as KScreen

Controls.ToolButton {
    id: root

    property int value
    property var rot
    property var tooltip

    Layout.fillWidth: true
    height: childrenRect.height

    checked: element.rotation === rot

    Controls.ToolTip {
        text: tooltip
        timeout: 5000
    }

    contentItem: Kirigami.Icon {
        source: "view-preview"
        rotation: root.value
    }

    onClicked: {
        if (element.rotation === rot) {
            return;
        }

        element.rotation = rot;
        screen.resetTotalSize();
    }

    implicitWidth: contentItem.implicitWidth + 2 * Kirigami.Units.smallSpacing
    implicitHeight: contentItem.implicitHeight + 2 * Kirigami.Units.smallSpacing

    Component.onCompleted: {
        switch(value) {
        case 90:
            rot = KScreen.Output.Left;
            tooltip = i18n("90° Clockwise");
            break;
        case 180:
            rot = KScreen.Output.Inverted;
            tooltip = i18n("Upside Down");
            break;
        case 270:
            rot = KScreen.Output.Right;
            tooltip = i18n("90° Counterclockwise")
            break;
        case 0:
        default:
            rot = KScreen.Output.None;
            tooltip = i18n("No Rotation");
        }
    }
}
