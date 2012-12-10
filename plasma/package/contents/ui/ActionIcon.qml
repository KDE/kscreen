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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

Item {
    id: button;

    property alias icon: actionIcon.icon;
    property alias label: actionLabel.text;

    signal clicked();

    anchors {
        left: parent.left;
        right: parent.right;
    }

    height: 40;

    PlasmaCore.FrameSvgItem {

        id: stateNormal;

        anchors.fill: parent;

        opacity: 1.0;
        imagePath: "widgets/tasks";
        prefix: "normal";
    }

    PlasmaCore.FrameSvgItem {

        id: stateHover;

        anchors.fill: parent;

        opacity: 0.0;
        imagePath: "widgets/tasks";
        prefix: "hover";
    }

    PlasmaCore.FrameSvgItem {

        id: statePressed;

        anchors.fill: parent;

        opacity: 0.0;
        imagePath: "widgets/tasks";
        prefix: "focus";
    }

    MouseArea {

        id: mouseArea;

        anchors.fill: parent;

        hoverEnabled: true;

        onEntered: button.state = "hover";
        onExited: button.state = "normal";
        onPressed: button.state = "pressed";
        onReleased: button.state = "hover";
        onClicked: button.clicked();
    }

    QIconItem {

        id: actionIcon;

        anchors {
            left: parent.left;
            top: parent.top;
            bottom: parent.bottom;
            margins: 10;
        }

        width: 30;

        smooth: true;
    }

    PlasmaComponents.Label {

        id: actionLabel;

        anchors {
            top: parent.top;
            left: actionIcon.right;
            bottom: parent.bottom;
            right: parent.right;
            margins: 10;
        }
    }

    states: [
        State {
            name: "normal";
            PropertyChanges {
                target: stateHover;
                opacity: 0.0;
            }
            PropertyChanges {
                target: statePressed;
                opacity: 0.0;
            }
            PropertyChanges {
                target: stateNormal;
                opacity: 1.0;
            }
        },

        State {
            name: "hover";
            PropertyChanges {
                target: stateNormal;
                opacity: 0.0;
            }
            PropertyChanges {
                target: statePressed;
                opacity: 0.0;
            }
            PropertyChanges {
                target: stateHover;
                opacity: 1.0;
            }
        },

        State {
            name: "pressed";
            PropertyChanges {
                target: stateHover;
                opacity: 0.0;
            }
            PropertyChanges {
                target: stateNormal;
                opacity: 0.0;
            }
            PropertyChanges {
                target: statePressed;
                opacity: 1.0;
            }
        }
    ]

    transitions: Transition {
         PropertyAnimation {
             properties: "opacity";
             duration: 200;
        }
     }
}