/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

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

import QtQuick 1.1
import org.kde.qtextracomponents 0.1
import org.kde.plasma.core 0.1 as PlasmaCore

MouseArea {

    id: mouseArea;

    property int iconSize: 22;
    property bool enabled: true;
    property string enabledIcon;
    property string disabledIcon;
    property alias tooltipText: tooltipLabel.text;

    width: iconSize;
    height: iconSize;

    hoverEnabled: true;

    SystemPalette {

        id: palette;
    }

    PlasmaCore.Theme {

        id: theme;
    }

    Timer {

        id: tooltipTimer;
        interval: 600;
        running: false;

        onTriggered: tooltip.show();
    }

    QIconItem {

        id: button;

        anchors.fill: parent;

        icon: {
            if (parent.enabled) {
                return parent.enabledIcon;
            } else if (parent.disabledIcon == "") {
                return parent.enabledIcon;
            } else {
                return parent.disabledIcon;
            }
        }

        enabled: parent.enabled;
    }

    Behavior on width {
        PropertyAnimation {
            duration: 100;
        }
    }

    Behavior on height {
        PropertyAnimation {
            duration: 100;
        }
    }

    onEntered: {
        tooltipTimer.start();
    }

    onExited: {
        tooltip.hide();
    }

    /* FIXME: We should not mix Plasma and QWidgets, but until there's proper
     * implementation of dialogs in QtQuick, we have no choice (and I'm not
     * going to write the dialog myself) - dan */
    PlasmaCore.Dialog {

        id: tooltip;

        windowFlags: Qt.ToolTip;

        mainItem: Text {

            id: tooltipLabel;

            height: paintedHeight

            color: theme.textColor;
        }

        function show() {
            var pos = popupPosition(mouseArea, Qt.AlignLeft);
            x = pos.x + mouseArea.mouseX;
            y = pos.y + mouseArea.mouseY + 5;

            visible = true;
        }

        function hide() {
            if (tooltipTimer.running) {
                tooltipTimer.stop();
            }

            visible = false;
        }
    }
}
