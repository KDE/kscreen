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

    contentWidth: paddingItem.implicitWidth
    contentHeight: Math.max(paddingItem.implicitHeight, availableHeight)

    readonly property bool hasDisabledScreens: layout.children.some(child => child.outputDisabled)

    Item {
        id: paddingItem
        anchors.fill: parent

        implicitWidth: Math.max(layout.implicitWidth, layout.iconSize) + Kirigami.Units.smallSpacing * 2
        implicitHeight: layout.implicitHeight + Kirigami.Units.smallSpacing * 2

        Kirigami.Heading {
            anchors.centerIn: parent

            text: i18nc("@info, screens referring to displays/monitors", "Disabled screens")
            visible: !disabledScreenView.hasDisabledScreens

            transformOrigin: Item.Center
            rotation: 90

            level: 4
            width: parent.height - Kirigami.Units.smallSpacing * 2
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight

            opacity: dropArea.containsDrag || disabledScreenView.hasDisabledScreens ? 0 : 0.6
            Behavior on opacity {
                PropertyAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        DropArea {
            anchors.fill: parent
            id: dropArea

            keys: ["enabledOutput"]
            onDropped: (drop) => drop.source.disable()
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
                        onTapped: {
                            root.selectedOutput = model.index;
                        }
                    }

                    QQC2.ToolTip {
                        visible: hoverHandler.hovered
                        text: model.display
                    }

                    function enable() {
                        model.enabled = true;
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
    }
}
