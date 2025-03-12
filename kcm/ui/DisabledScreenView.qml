/*
 *  SPDX-FileCopyrightText: 2025 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels

QQC2.ScrollView {
    id: disabledScreenView

    readonly property bool draggingItem: layout.children.some(child => child instanceof Output2 && child.isDragging)

    // We have to create our own Flickable as the default
    // created by ScrollView will clip contents
    Flickable {
        anchors.fill: parent

        contentWidth: layout.width
        contentHeight: Math.max(layout.implicitHeight, availableHeight)

        Rectangle {
            anchors.fill: parent

            color: Kirigami.Theme.activeBackgroundColor
            opacity: dropArea.containsDrag ? 1 : 0
            visible: opacity > 0

            Behavior on opacity {
                PropertyAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        ColumnLayout {
            id: layout

            readonly property int outputWidth: Kirigami.Units.gridUnit * 6
            readonly property int outputMargins: Kirigami.Units.smallSpacing

            height: Math.max(implicitHeight, disabledScreenView.height)
            width: outputWidth + outputMargins * 2

            spacing: 0

            Item {
                Layout.fillHeight: true
            }

            Repeater {
                model: kcm.outputModel
                delegate: Output2 {
                    id: output
                    Layout.margins: layout.outputMargins

                    visible: !model.enabled

                    implicitWidth: layout.outputWidth
                    implicitHeight: Math.round((implicitWidth / model.size.width) * model.size.height)
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }

        DropArea {
            id: dropArea
            anchors.fill: parent

            keys: ["enabledOutput"]
            onDropped: (drop) => drop.source.disable()
        }

        /*
        Item {
            id: paddingItem
            anchors.fill: parent

            implicitWidth: layout.implicitWidth + Kirigami.Units.smallSpacing * 2
            implicitHeight: layout.implicitHeight + Kirigami.Units.smallSpacing * 2

            Rectangle {
                anchors.fill: parent

                color: Kirigami.Theme.activeBackgroundColor
                opacity: dropArea.containsDrag ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    PropertyAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            ColumnLayout {
                id: layout
                anchors.top: parent.top
                anchors.topMargin: Kirigami.Units.smallSpacing
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: Kirigami.Units.smallSpacing

                Repeater {
                    model: kcm.outputModel
                    delegate: Output2 {
                        id: output
                        implicitWidth: Kirigami.Units.gridUnit * 6
                        implicitHeight: Math.round((implicitWidth / model.size.width) * model.size.height)
                        //implicitHeight: Kirigami.Units.gridUnit * 3

                        //readonly property bool isDragging: dragHandler.active
                        //readonly property bool outputDisabled: !model.enabled

                        //visible: !model.enabled

                        // HoverHandler {
                        //     id: hoverHandler
                        // }
                        //
                        // TapHandler {
                        //     onPressedChanged: {
                        //         if (pressed) {
                        //             root.selectedOutput = model.index;
                        //             outputIcon.Drag.hotSpot = point.position;
                        //         }
                        //     }
                        // }
                        //
                        // QQC2.ToolTip {
                        //     visible: hoverHandler.hovered && !dragHandler.active
                        //     text: model.display
                        // }
                        //
                        // function enable() {
                        //     model.enabled = true;
                        // }
                        //
                        // Item {
                        //     anchors.fill: parent
                        //     // So we can show a grabby hand cursor when hovered over
                        //     HoverHandler {
                        //         cursorShape: kcm.multipleScreensAvailable ? Qt.SizeAllCursor : undefined
                        //     }
                        // }
                        //
                        // property point dragStartPosition
                        //
                        // Drag.active: dragHandler.active
                        // Drag.dragType: Drag.Internal
                        // Drag.keys: ["disabledOutput"]
                        //
                        // DragHandler {
                        //     id: dragHandler
                        //     acceptedButtons: Qt.LeftButton
                        //     target: null
                        //
                        //     onTranslationChanged: {
                        //         output.x = dragStartPosition.x + translation.x;
                        //         output.y = dragStartPosition.y + translation.y;
                        //     }
                        //
                        //     onActiveChanged: {
                        //         if (active) {
                        //             output.dragStartPosition = Qt.point(output.x, output.y);
                        //         } else {
                        //             output.Drag.drop();
                        //             output.x = dragStartPosition.x;
                        //             output.y = dragStartPosition.y;
                        //         }
                        //     }
                        // }
                    }
                }
            }

            DropArea {
                id: dropArea
                anchors.fill: parent

                keys: ["enabledOutput"]
                onDropped: (drop) => drop.source.disable()
            }
        }
        */
    }
}
