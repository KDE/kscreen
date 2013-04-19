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
import KScreen 1.0 as KScreen;

Item {

    id: root;

    property variant virtualScreen: null;

    signal identifyOutputsRequested();

    objectName: "root";
    focus: true;

    anchors.fill: parent;

    SystemPalette {
        id: palette;
    }

    /* Don't use colors from theme, we want to be consistent with KCM, not
     * with Plasma here. Font sizes are the same though */
    PlasmaCore.Theme {

        id: theme;
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

            OutputView {

                id: outputView;

                anchors.fill: parent;
                clip: true;

                objectName: "outputView";

                maxContentWidth: virtualScreen ? virtualScreen.maxSize.width : root.width;
                maxContentHeight: virtualScreen ? virtualScreen.maxSize.width : root.width;
            }
        }

        PlasmaComponents.ScrollBar {

            id: horizontalScrollbar;

            /* The scrollbars seem to have broken anchoring? */
            y: background.height - height;
            width: background.width - verticalScrollbar.width;

            orientation: Qt.Horizontal;
            flickableItem: outputView;
        }

        PlasmaComponents.ScrollBar {

            id: verticalScrollbar;

            x: background.width - width;
            height: background.height - horizontalScrollbar.height;

            orientation: Qt.Vertical;
            flickableItem: outputView;
        }

        Column {

            anchors {
                left: parent.left;
                right: identifyButton.left;
                bottom: horizontalScrollbar.top;
                margins: 5;
            }

            spacing: 5;

            Tip {

                id: dragTip;

                icon: "dialog-information";
                text: i18n("Tip: Hold Ctrl while dragging a display to disable snapping");
            }

            Tip {

                id: noActiveOutputsWarning;

                icon: "dialog-warning";
                text: i18n("Warning: There are no active outputs!");
            }
        }

        IconButton {

            id: identifyButton;

            anchors {
                right: verticalScrollbar.left;
                bottom: horizontalScrollbar.top;
                margins: 5;
            }

            enabledIcon: "kdocumentinfo"
            iconSize: theme.largeIconSize;
            smooth: true
            tooltipText: i18n("Identify outputs");

            onClicked: root.identifyOutputsRequested();
        }
    }
}
