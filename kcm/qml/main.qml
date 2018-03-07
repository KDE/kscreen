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

import QtQuick 2.1
import QtQuick.Controls 1.1 as Controls
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrols 2.0
import org.kde.kscreen 1.0

Item {

    id: root;

    property variant virtualScreen: null;

    objectName: "root";
    focus: true;

    SystemPalette {
        id: palette;
    }

    Rectangle {
        id: background;

        anchors.fill: parent;
        focus: true;

        color: palette.base;

        FocusScope {

            id: outputViewFocusScope;

            anchors.fill: parent;
            focus: true;

            QMLScreen {

                id: outputView;

                anchors.fill: parent;
                clip: true;

                objectName: "outputView";
            }
        }

        Column {

            anchors {
                left: parent.left;
                right: identifyButton.left;
                bottom: parent.bottom;
                margins: 5;
            }

            spacing: 5;

            Tip {

                id: dragTip;

                icon: "dialog-information";
                text: i18nd("kcm_displayconfiguration", "Tip: Hold Ctrl while dragging a display to disable snapping");
            }

            Tip {

                id: noActiveOutputsWarning;

                icon: "dialog-warning";
                text: i18nd("kcm_displayconfiguration", "Warning: There are no active outputs!");
            }

            Tip {

                id: tooManyActiveOutputs;
                objectName: "tooManyActiveOutputs";

                icon: "dialog-error";
                text: i18ndp("kcm_displayconfiguration",
                             "Your system only supports up to %1 active screen",
                             "Your system only supports up to %1 active screens",
                             virtualScreen ? virtualScreen.maxActiveOutputsCount : 1);
            }
        }


        Controls.ToolButton {

            id: identifyButton
            objectName: "identifyButton";

            anchors {
                right: parent.right
                bottom: parent.bottom
                margins: 5
            }

            height: width
            width: theme.largeIconSize;
            iconName: "kdocumentinfo"

            tooltip: i18nd("kcm_displayconfiguration", "Identify outputs");
        }
    }
}
