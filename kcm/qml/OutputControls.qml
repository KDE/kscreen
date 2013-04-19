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
import org.kde.plasma.components 0.1 as PlasmaComponents;
import org.kde.plasma.core 0.1 as PlasmaCore
import KScreen 1.0

/* We switched from PlasmaCore.Dialog to PlasmaComponents.Dialog to
   fix bug #312544. Unfortunatelly the component works correctly only
   in KDE 4.10, it does not fix the bug in KDE 4.9 and it misbehaves.

   To support KDE 4.9 (and older) and to make the code still working,
   we ship our own copy of the Dialog.qml and it's dependencies from
   kde-runtime/plasma/declarativeimports */
import "plasmacomponents" as PlasmaComponents410

Item {

    id: root;

    signal primaryTriggered();
    signal enabledToggled();

    PlasmaCore.Theme {
        id: theme;
    }

    property int rotationDirection;
    property Item parentItem;
    property int iconSize: theme.iconSizes.toolbar;
    property int fontSize: theme.defaultFont.pointSize;
    property alias isToggleButtonVisible: enabledButton.visible;

    width: parent.width - 36;
    height: parent.height - 20;

    onWidthChanged: {
        adaptToSizeChange();
    }

    onHeightChanged: {
        adaptToSizeChange();
    }

    function adaptToSizeChange()
    {
        if ((width < 100) || (height < 80)) {
            monitorName.visible = false;
            outputNameAndSize.anchors.top = root.top;
            enabledButton.scale = 0.4;
            enabledButton.anchors.topMargin = 0;
            root.fontSize = theme.smallestFont.pointSize;
            root.iconSize = theme.iconSizes.small;
        } else if ((width < 120) || (height < 100)) {
            monitorName.visible = true;
            outputNameAndSize.anchors.top = monitorName.bottom;
            enabledButton.scale = 0.6;
            enabledButton.anchors.topMargin = 2;
            root.fontSize = theme.smallestFont.pointSize;
            root.iconSize = theme.iconSizes.small;
        } else {
            monitorName.visible = true;
            outputNameAndSize.anchors.top = monitorName.bottom;
            enabledButton.scale = 0.8
            enabledButton.anchors.topMargin = 4;
            root.fontSize = theme.defaultFont.pointSize;
            root.iconSize = theme.iconSizes.toolbar;
        }
    }

    Behavior on rotation {
        /*
        RotationAnimation {
            easing.type: "OutCubic"
            duration: 250;
            // Opposite of the monitor rotation so the controls stay rightside up.
            direction: (rotationDirection == RotationAnimation.Clockwise) ?
                    RotationAnimation.Counterclockwise : RotationAnimation.Clockwise;
        }*/
    }

    Behavior on width {
        PropertyAnimation {
            duration: 250;
            easing.type: "OutCubic";
        }
    }

    Behavior on height {
        PropertyAnimation {
            duration: 250;
            easing.type: "OutCubic";
        }
    }

    /* Output name */
    Text {
        id: monitorName;

        anchors {
            top: root.top;
            topMargin: 5;
        }
        width: parent.width;

        text: output.connected ? output.edid.vendor : "";
        color: palette.text;
        font {
            family: theme.defaultFont.family;
            capitalization: theme.defaultFont.capitalization;
            italic: theme.defaultFont.italic;
            letterSpacing: theme.defaultFont.letterSpacing;
            strikeout: theme.defaultFont.strikeout;
            underline: theme.defaultFont.underline;
            weight: theme.defaultFont.weight;
            wordSpacing: theme.defaultFont.wordSpacing;
            pointSize: root.fontSize;
        }
        elide: Text.ElideRight;

        horizontalAlignment: Text.AlignHCenter;

        Behavior on font.pointSize {
            PropertyAnimation {
                duration: 100;
            }
        }
    }

    Text {
        id: outputNameAndSize;

        anchors {
            top: monitorName.bottom;
            horizontalCenter: parent.horizontalCenter;
        }
        width: parent.width;

        text: output.name;
        color: palette.text;
        font {
            family: theme.defaultFont.family;
            capitalization: theme.defaultFont.capitalization;
            italic: theme.defaultFont.italic;
            letterSpacing: theme.defaultFont.letterSpacing;
            strikeout: theme.defaultFont.strikeout;
            underline: theme.defaultFont.underline;
            weight: theme.defaultFont.weight;
            wordSpacing: theme.defaultFont.wordSpacing;
            pointSize: root.fontSize - 2;
        }

        horizontalAlignment: Text.AlignHCenter;
    }

    /* Enable/Disable output */
    PlasmaComponents.Switch {

        id: enabledButton;

        anchors {
            top: outputNameAndSize.bottom;
            topMargin: 4;
            horizontalCenter: parent.horizontalCenter;
        }

        scale: 0.8;
        checked: output.enabled;

        onCheckedChanged: {
            /* FIXME: This should be in KScreen */
            if (output.enabled != enabledButton.checked) {
                output.enabled = enabledButton.checked;
            }
            root.enabledToggled();
        }

        MouseArea {

            anchors.fill: parent;
            hoverEnabled: true;
            onClicked: {
                enabledButton.checked = !enabledButton.checked;
            }
        }
    }

    Row {
        anchors {
            horizontalCenter: parent.horizontalCenter;
            bottom: parent.bottom;
            bottomMargin: 5;
        }

        spacing: 5;
        visible: output.enabled;

        /* Rotation */
        IconButton {

            id: rotateButton;

            iconSize: root.iconSize;
            enabledIcon: "object-rotate-left";
            tooltipText: i18n("Rotate output 90° counterclockwise");

            acceptedButtons: Qt.LeftButton | Qt.RightButton;
            onClicked: {
                if (mouse.button == Qt.LeftButton) {
                    monitor.rotationDirection = RotationAnimation.Counterclockwise;
                    if (output.rotation == Output.None) {
                        output.rotation = Output.Left;
                    } else if (output.rotation == Output.Left) {
                        output.rotation = Output.Inverted;
                    } else if (output.rotation == Output.Inverted) {
                        output.rotation = Output.Right;
                    } else if (output.rotation == Output.Right) {
                        output.rotation = Output.None;
                    }
                } else {
                    monitor.rotationDirection = RotationAnimation.Clockwise;
                    if (output.rotation == Output.None) {
                        output.rotation = Output.Right;
                    } else if (output.rotation == Output.Right) {
                        output.rotation = Output.Inverted;
                    } else if (output.rotation == Output.Inverted) {
                        output.rotation = Output.Left;
                    } else if (output.rotation == Output.Left) {
                        output.rotation = Output.None;
                    }
                }
            }
        }

        /* Primary toggle */
        IconButton {

            id: primaryButton;

            iconSize: root.iconSize;
            enabledIcon: "bookmarks";
            enabled: (output.enabled && output.primary);
            tooltipText: i18n("Toggle primary output");

            onClicked: {
                if (output.enabled) {
                    output.primary = !output.primary

                    if (output.primary) {
                        root.primaryTriggered();
                    }
                }
            }
        }


        IconButton {

            id: resizeButton;

            iconSize: root.iconSize;
            enabledIcon: "view-restore"
            tooltipText: i18n("Show list of available display resolutions");

            onClicked: selectionDialog.open();
        }
    }

    PlasmaComponents410.Dialog {

        id: selectionDialog;

        visualParent: resizeButton;

        content: [
                ModeSelectionWidget {
                    id: contentItem;
                    output: root.parentItem;

                    onAccepted: selectionDialog.close();
                }
        ]

    }
}
