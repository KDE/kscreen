/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

QQC2.ScrollView {
    id: screenView

    required property var outputs
    required property bool interactive

    property size totalSize

    function resetTotalSize() {
        totalSize = kcm.normalizeScreen();
    }

    onWidthChanged: resetTotalSize()
    onHeightChanged: resetTotalSize()

    readonly property real relativeFactor: {
        const relativeSize = Qt.size(
            totalSize.width / (0.6 * width),
            totalSize.height / (0.65 * height),
        );
        if (relativeSize.width > relativeSize.height) {
            // Available width smaller than height, optimize for width (we have
            // '>' because the available width, height is in the denominator).
            return relativeSize.width;
        } else {
            return relativeSize.height;
        }
    }

    readonly property int xOffset: (width - totalSize.width / relativeFactor) / 2;
    readonly property int yOffset: (height - totalSize.height / relativeFactor) / 2;

    readonly property bool draggingItem: contentChildren.some(child => child instanceof Output2 && child.isDragging)

    Kirigami.Heading {
        z: 90
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Kirigami.Units.smallSpacing
        }
        level: 4
        horizontalAlignment: Text.AlignHCenter
        text: i18n("Drag screens to re-arrange them")

        opacity: interactive ? 0.6 : 0
        visible: opacity > 0
        Behavior on opacity {
            PropertyAnimation {
                easing.type: Easing.InOutQuad
                duration: Kirigami.Units.shortDuration
            }
        }
    }

    Repeater {
        model: kcm.outputModel
        delegate: Output2 {
            id: output

            readonly property bool isDragging: dragHandler.active

            interactive: screenView.interactive
            showPosition: dragHandler.active && !dragHandler.avoidSnapping

            onIsSelectedChanged: {
                if (isSelected) {
                    z = 89;
                } else {
                    z = 0;
                }
            }

            property size outputSize: model.size

            visible: model.enabled && model.replicationSourceIndex === 0

            // Transform a position from screen-space to the display configuration space
            function getAbsolutePosition(pos) {
                return Qt.point((pos.x - screenView.xOffset) * screenView.relativeFactor,
                                (pos.y - screenView.yOffset) * screenView.relativeFactor);
            }

            // Transform a position from display configuration space to screen-space
            function getRelativePosition(pos) {
                return Qt.point((pos.x / screenView.relativeFactor) + screenView.xOffset,
                                (pos.y / screenView.relativeFactor) + screenView.yOffset);
            }

            onVisibleChanged: screenView.resetTotalSize()
            onOutputSizeChanged: screenView.resetTotalSize()

            x: dragHandler.avoidSnapping ? (dragStartPosition.x + dragHandler.translation.x)
                                         : (model.position ? model.position.x / screenView.relativeFactor + screenView.xOffset : 0)
            y: dragHandler.avoidSnapping ? (dragStartPosition.y + dragHandler.translation.y)
                                         : (model.position ? model.position.y / screenView.relativeFactor + screenView.yOffset : 0)

            width: model.size ? model.size.width / screenView.relativeFactor : 1
            height: model.size ? model.size.height / screenView.relativeFactor : 1

            property point dragStartPosition

            TapHandler {
                id: tapHandler
                property bool isLongPressed: false
                gesturePolicy: TapHandler.WithinBounds

                onPressedChanged: {
                    if (pressed) {
                        root.selectedOutput = model.index;
                        output.Drag.hotSpot = point.position;
                        dragStartPosition = Qt.point(output.x, output.y);
                    } else {
                        isLongPressed = false;
                    }
                }
                onLongPressed: isLongPressed = true;
                longPressThreshold: 0.3
            }

            Drag.active: dragHandler.active
            Drag.dragType: Drag.Internal
            Drag.keys: output.enabled ? ["enabledOutput"] : ["disabledOutput"]

            DragHandler {
                id: dragHandler
                enabled: output.interactive
                acceptedButtons: Qt.LeftButton
                target: null

                // Avoid snapping into place when we move 50px away from the snapped position
                readonly property bool avoidSnapping: active && Math.hypot((dragStartPosition.x + translation.x) - (getRelativePosition(model.position).x),
                                                                        (dragStartPosition.y + translation.y) - (getRelativePosition(model.position).y)) >= 50

                onTranslationChanged: {
                    var newX = dragStartPosition.x + translation.x;
                    var newY = dragStartPosition.y + translation.y;
                    model.position = getAbsolutePosition(Qt.point(newX, newY));
                }

                onActiveChanged: {
                    model.interactiveMove = active;

                    if (!active) {
                        output.Drag.drop();
                        screenView.resetTotalSize();
                    }
                }
            }
        }

        onCountChanged: resetTotalSize()
    }
}
