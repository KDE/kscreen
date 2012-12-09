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


import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: displayconfiguration
    property int minimumWidth: 290
    property int minimumHeight: 340

    signal runKCM();
//     signal applyAction(DisplayAction config);

    PlasmaComponents.Label {
        id: header
        text: i18n("A new display has been detected");
        anchors {
            top: parent.top;
            topMargin: 3;
            left: parent.left;
            right: parent.right
        }
        horizontalAlignment: Text.AlignHCenter
    }

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"
    }
    PlasmaCore.SvgItem {
        id: headerSeparator
        svg: lineSvg
        elementId: "horizontal-line"
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            topMargin: 3
        }
        height: lineSvg.elementSize("horizontal-line").height
    }

    Column {
        id: actionsColumn;
        anchors {
            top: headerSeparator.bottom;
            left: parent.left;
            right: parent.right;
        }

        ActionIcon {
            id: extendRight;
            icon: "extend-right";
            label: qsTr("Extend Right");

//             onClicked: displayconfiguration.applyAction(Handler.ActionExtendRight);
        }

        ActionIcon {
            id: extendLeft;
            icon: "extend-left";
            label: qsTr("Extend Left");

//             onClicked: displayconfiguration.applyAction(Handler.ActionExtendLeft);
        }

        ActionIcon {
            id: clone;
            icon: "clone";
            label: qsTr("Clone");

//             onClicked: displayconfiguration.applyAction(Handler.ActionClone);
        }

        ActionIcon {
            id: extendAbove;
            icon: "extend-above";
            label: qsTr("Extend Above");

//             onClicked: displayconfiguration.applyAction(Handler.ActionExtendAbove);
        }

        ActionIcon {
            id: extendBelow;
            icon: "extend-below";
            label: qsTr("Extend Below");

//             onClicked: displayconfiguration.applyAction(Handler.ActionExtendBelow);
        }

        ActionIcon {
            id: noAction;
            icon: "no-action";
            label: qsTr("No Action");

//             onClicked: displayconfiguration.applyAction(Handler.ActionNoAction);
        }
    }

    Row {
        anchors {
            top: actionsColumn.bottom;
            right: parent.right;
            left: parent.left;
            bottom: parent.bottom;
        }

        PlasmaComponents.Button {
            id: kcmButton;

            text: qsTr("Advanced Configuration");
            iconSource: "preferences-system";

            onClicked: displayconfiguration.runKCM();
        }
    }
}
