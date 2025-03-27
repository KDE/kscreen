/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2012 Dan Vratil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: output

    property bool interactive: false

    readonly property bool isSelected: root.selectedOutput === model.index
    property size outputSize: model.size

    property string textWithScale: i18nc("Width, height, scale; separated with no-break space", "(%1 × %2, %3%)",
                                         model.resolution.width.toString(), model.resolution.height.toString(), Math.round(model.scale * 100.0))
    property string textWithoutScale: i18nc("Width, height; separated with no-break space", "(%1 × %2)",
                                            model.resolution.width.toString(), model.resolution.height.toString())

    onIsSelectedChanged: {
        if (isSelected) {
            z = 89;
        } else {
            z = 0;
        }
    }

    function getAbsolutePosition(pos) {
        return Qt.point((pos.x - screen.xOffset) * screen.relativeFactor,
                        (pos.y - screen.yOffset) * screen.relativeFactor);
    }

    visible: model.enabled && model.replicationSourceIndex === 0

    onVisibleChanged: screen.resetTotalSize()
    onOutputSizeChanged: screen.resetTotalSize()

    x: model.position ? model.position.x / screen.relativeFactor + screen.xOffset : 0
    y: model.position ? model.position.y / screen.relativeFactor + screen.yOffset : 0

    width: model.size ? model.size.width / screen.relativeFactor : 1
    height: model.size ? model.size.height / screen.relativeFactor : 1

    Rectangle {
        id: outline

        readonly property int orientationPanelWidth: 10

        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        radius: Kirigami.Units.cornerRadius
        color: Kirigami.Theme.alternateBackgroundColor

        border {
            color: isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
            width: 1

            Behavior on color {
                PropertyAnimation {
                    duration: Kirigami.Units.longDuration
                }
            }
        }

        // Task bar at the bottom of the output to give a hint of the orientation.
        // TODO use a single Rectangle with bottomLeftRadius/bottomRightRadius
        // once the API in Qt 6.7 has stabilized.
        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: outline.orientationPanelWidth
            radius: outline.radius
            color: outline.border.color

            // Undo the rounded top corners
            Rectangle {
                width: parent.width
                height: outline.radius
                color: outline.border.color
            }
        }
    }

    Item {
        anchors.fill: parent
        // So we can show a grabby hand cursor when hovered over
        HoverHandler {
            cursorShape: output.interactive ? Qt.SizeAllCursor : undefined
        }
        z: 2
    }

    Item {
        id: labelContainer
        anchors {
            fill: parent
            margins: outline.border.width
        }

        // so the text is drawn above orientationPanelContainer
        z: 1
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 0
            width: parent.width

            QQC2.Label {
                id: nameLabel
                Layout.fillWidth: true

                // Don't exceed our bounds
                Layout.maximumHeight: labelContainer.height

                text: model.display
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            QQC2.Label {
                id: resolutionLabel
                Layout.fillWidth: true

                // Only show if we have room for both
                visible: labelContainer.height >= implicitHeight + nameLabel.implicitHeight

                text: model.scale !== 1 ? textWithScale : textWithoutScale
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    }

    states: [
        State {
            name: "transposed"
            PropertyChanges {
                target: outline
                width: output.height
                height: output.width
            }
        },

        State {
            name: "rot0"
            when: model.rotation === 1
            PropertyChanges {
                target: labelContainer
                anchors.bottomMargin: outline.orientationPanelWidth + outline.border.width
            }
        },
        State {
            name: "rot90"
            extend: "transposed"
            when: model.rotation === 2
            PropertyChanges {
                target: outline
                rotation: 90
            }
            PropertyChanges {
                target: labelContainer
                anchors.leftMargin: outline.orientationPanelWidth + outline.border.width
            }
        },
        State {
            name: "rot180"
            when: model.rotation === 4
            PropertyChanges {
                target: outline
                rotation: 180
            }
            PropertyChanges {
                target: labelContainer
                anchors.topMargin: outline.orientationPanelWidth + outline.border.width
            }
        },
        State {
            name: "rot270"
            extend: "transposed"
            when: model.rotation === 8
            PropertyChanges {
                target: outline
                rotation: 270
            }
            PropertyChanges {
                target: labelContainer
                anchors.rightMargin: outline.orientationPanelWidth + outline.border.width
            }
        }
    ]

    Rectangle {
        id: posLabel

        y: 4
        x: 4
        width: childrenRect.width + 5
        height: childrenRect.height + 2
        radius: Kirigami.Units.cornerRadius

        opacity: model.enabled &&
                 (tapHandler.isLongPressed || output.isDragging) ? 0.9 : 0.0


        color: Kirigami.Theme.disabledTextColor

        Text {
            id: posLabelText

            y: 2
            x: 2

            text: i18n("%1, %2", model.normalizedPosition.x, model.normalizedPosition.y)
            color: "white"
        }

        Behavior on opacity {
            PropertyAnimation {
                duration: Kirigami.Units.longDuration
            }
        }
    }

    QQC2.ToolButton {
        id: replicas

        property int selectedReplica: -1

        height: output.height / 4
        width: output.width / 5
        anchors.top: output.top
        anchors.right: output.right
        anchors.margins: 5

        visible: model.replicasModel.length > 0
        icon.name: "osd-duplicate"

        QQC2.ToolTip {
            text: i18n("Replicas")
        }

        onClicked: {
            var index = selectedReplica + 1;
            if (index >= model.replicasModel.length) {
                index = 0;
            }
            if (root.selectedOutput !== model.replicasModel[index]) {
                root.selectedOutput = model.replicasModel[index];
            }
        }
    }

    property bool preciseDrag: false

    property point dragStartPosition

    TapHandler {
        id: tapHandler
        property bool isLongPressed: false
        gesturePolicy: TapHandler.WithinBounds

        onPressedChanged: {
            if (pressed) {
                root.selectedOutput = model.index;
                dragStartPosition = Qt.point(output.x, output.y)
            } else {
                isLongPressed = false;
            }
        }
        onLongPressed: isLongPressed = true;
        longPressThreshold: 0.3
    }

    readonly property bool isDragging: impreciseDragHandler.active || preciseDragHandler.active

    DragHandler {
        id: impreciseDragHandler
        enabled: output.interactive && !preciseDragHandler.active
        acceptedButtons: Qt.LeftButton
        acceptedModifiers: Qt.KeyboardModifierMask
        grabPermissions: PointerHandler.CanTakeOverFromAnything | PointerHandler.TakeOverForbidden
        target: null

        onTranslationChanged: {
            var newX = dragStartPosition.x + translation.x;
            var newY = dragStartPosition.y + translation.y;
            model.position = getAbsolutePosition(Qt.point(newX, newY));
        }
        onActiveChanged: {
            model.interactiveMove = active;
            if (!active) {
                screen.resetTotalSize();
            }
        }
    }

    DragHandler {
        id: preciseDragHandler
        enabled: output.interactive
        acceptedButtons: Qt.LeftButton
        acceptedModifiers: active ? Qt.KeyboardModifierMask : Qt.ControlModifier
        grabPermissions: PointerHandler.CanTakeOverFromAnything | PointerHandler.TakeOverForbidden
        target: null

        onTranslationChanged: {
            var newX = dragStartPosition.x + (translation.x / (screen.relativeFactor / 2));
            var newY = dragStartPosition.y + (translation.y / (screen.relativeFactor / 2));
            model.position = getAbsolutePosition(Qt.point(newX, newY));
        }
        onActiveChanged: {
            model.interactiveMove = active;
            if (!active) {
                screen.resetTotalSize();
            }
        }
    }
}

