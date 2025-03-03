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

    readonly property bool draggingItem: layout.children.some(child => child.isDragging)

    // We have to create our own Flickable as the default
    // created by ScrollView will clip contents
    Flickable {
        anchors.fill: parent

        contentWidth: paddingItem.implicitWidth
        contentHeight: Math.max(paddingItem.implicitHeight, availableHeight)

        Item {
            id: paddingItem
            anchors.fill: parent

            implicitWidth: Math.max(layout.implicitWidth, layout.iconSize) + Kirigami.Units.smallSpacing * 2
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

                readonly property int iconSize: Kirigami.Units.iconSizes.medium

                spacing: Kirigami.Units.smallSpacing

                Repeater {
                    model: kcm.outputModel
                    delegate: Kirigami.Icon {
                        id: outputIcon

                        readonly property bool isDragging: dragHandler.active
                        readonly property bool outputDisabled: !model.enabled

                        implicitWidth: layout.iconSize
                        implicitHeight: layout.iconSize

                        source: "monitor-symbolic"
                        visible: outputDisabled

                        color: root.selectedOutput === model.index ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
                        Behavior on color {
                            PropertyAnimation {
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                        }

                        HoverHandler {
                            id: hoverHandler
                        }

                        TapHandler {
                            onPressedChanged: {
                                if (pressed) {
                                    root.selectedOutput = model.index;
                                    outputIcon.Drag.hotSpot = point.position;
                                }
                            }
                        }

                        QQC2.ToolTip {
                            visible: hoverHandler.hovered && !dragHandler.active
                            text: model.display
                        }

                        function enable() {
                            model.enabled = true;
                        }

                        Item {
                            anchors.fill: parent
                            // So we can show a grabby hand cursor when hovered over
                            HoverHandler {
                                cursorShape: kcm.multipleScreensAvailable ? Qt.SizeAllCursor : undefined
                            }
                        }

                        property point dragStartPosition

                        Drag.active: dragHandler.active
                        Drag.dragType: Drag.Internal
                        Drag.keys: ["disabledOutput"]

                        DragHandler {
                            id: dragHandler
                            acceptedButtons: Qt.LeftButton
                            target: null

                            onTranslationChanged: {
                                outputIcon.x = dragStartPosition.x + translation.x;
                                outputIcon.y = dragStartPosition.y + translation.y;
                            }

                            onActiveChanged: {
                                if (active) {
                                    outputIcon.dragStartPosition = Qt.point(outputIcon.x, outputIcon.y);
                                } else {
                                    outputIcon.Drag.drop();
                                    outputIcon.x = dragStartPosition.x;
                                    outputIcon.y = dragStartPosition.y;
                                }
                            }
                        }
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
    }


}
