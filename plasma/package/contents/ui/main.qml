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
import org.kde.kscreen 1.0

Item {

    id: root;

    property int minimumWidth: 290;
    property int minimumHeight: childrenRect.height;
    property string displayName;

    signal runKCM();
    signal applyAction(int config);

    PlasmaComponents.Label {

        id: header

        anchors {
            top: parent.top;
            topMargin: 3;
            left: parent.left;
            right: parent.right
        }

        text: i18nc("%1 is name of the newly connected display", "A new display %1 has been detected", root.displayName);
        wrapMode: Text.WordWrap;
        horizontalAlignment: Text.AlignHCenter
    }

    PlasmaCore.Svg {

        id: lineSvg

        imagePath: "widgets/line"
    }

    PlasmaCore.SvgItem {

        id: headerSeparator

        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            topMargin: 3
        }

        height: lineSvg.elementSize("horizontal-line").height

        svg: lineSvg
        elementId: "horizontal-line"
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

            icon: "go-next";
            label: i18nc("Places the newly connected screen right of the existing one", "Extend to Right");

            onClicked: root.applyAction(KScreenApplet.ActionExtendRight);
        }

        ActionIcon {

            id: extendLeft;

            icon: "go-previous";
            label: i18nc("Places the newly connected screen left of the existing one", "Extend to Left");

            onClicked: root.applyAction(KScreenApplet.ActionExtendLeft);
        }

        ActionIcon {

            id: clone;

            icon: "window-duplicate";
            label: i18nc("Makes the newly conencted screen a clone of the primary one", "Clone Primary Output");

            onClicked: root.applyAction(KScreenApplet.ActionClone);
        }

        ActionIcon {

            id: disable;

            icon: "window-close";
            label: i18nc("Disables the newly connected screen",  "Disable");

            onClicked: root.applyAction(KScreenApplet.ActionDisable);
        }

        ActionIcon {

            id: noAction;

            icon: "dialog-cancel";
            label: i18n("No Action");

            onClicked: root.applyAction(KScreenApplet.ActionNoAction);
        }

        ActionIcon {

            id: runKCM;

            icon: "preferences-system";
            label: i18nc("Opens KScreen KCM", "Advanced Configuration");

            onClicked: root.runKCM();
        }
    }
}
