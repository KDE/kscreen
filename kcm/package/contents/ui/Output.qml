/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>
Copyright © 2012 Dan Vratil <dvratil@redhat.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as Controls
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.4 as Kirigami

Item {
    id: output

    property bool isSelected: root.selectedOutput === model.index
    property size outputSize: model.size

    onIsSelectedChanged: {
        if (isSelected) {
            z = 89;
        } else {
            z = 0;
        }
    }

    function getAbsolutePosition(pos) {
        return Qt.point((pos.x - screen.xOffset) * screen.relativeFactor,
                        (pos.y - screen.yOffset) * screen.relativeFactor) ;
    }

    visible: model.enabled && model.replicationSourceIndex === 0

    onVisibleChanged: screen.resetTotalSize()
    onOutputSizeChanged: screen.resetTotalSize()

    x: model.position ? model.position.x / screen.relativeFactor + screen.xOffset : 0
    y: model.position ? model.position.y / screen.relativeFactor + screen.yOffset : 0

    width: model.size ? model.size.width / screen.relativeFactor : 1
    height: model.size ? model.size.height / screen.relativeFactor : 1

    SystemPalette {
        id: palette
    }

    Rectangle {
        id: outline
        radius: 4
        color: palette.window

        anchors.fill: parent

        border {
            color: isSelected ? palette.highlight : palette.shadow
            width: 1

            Behavior on color {
                PropertyAnimation {
                    duration: 150
                }
            }
        }
    }

    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 0
            width: parent.width
            Layout.maximumHeight: parent.height

            Controls.Label {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing

                text: model.display
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            Controls.Label {
                Layout.fillWidth: true
                Layout.bottomMargin: Kirigami.Units.smallSpacing

                text: "(" + model.size.width + "x" + model.size.height + ")"
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }
        }
    }

    Rectangle {
        id: posLabel

        y: 4
        x: 4
        width: childrenRect.width + 5
        height: childrenRect.height + 2
        radius: 4

        opacity: model.enabled &&
                 (tapHandler.isLongPressed || dragHandler.active) ? 0.9 : 0.0


        color: palette.shadow

        Text {
            id: posLabelText

            y: 2
            x: 2

            text: model.normalizedPosition.x + "," + model.normalizedPosition.y
            color: "white"
        }

        Behavior on opacity {
            PropertyAnimation {
                duration: 100;
            }
        }
    }

    Controls.ToolButton {
        id: replicas

        property int selectedReplica: -1

        height: output.height / 4
        width: output.width / 5
        anchors.top: output.top
        anchors.right: output.right
        anchors.margins: 5

        visible: model.replicasModel.length > 0
        icon.name: "osd-duplicate"

        Controls.ToolTip {
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

    Item {
        id: orientationPanelContainer

        anchors.fill: output
        visible: false

        Rectangle {
            id: orientationPanel
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            height: 10
            color: isSelected ? palette.highlight : palette.shadow
            smooth: true

            Behavior on color {
                PropertyAnimation {
                    duration: 150
                }
            }
        }
    }

    states: [
        State {
            name: "rot90"
            when: model.rotation === 2
            PropertyChanges {
                target: orientationPanel
                height: undefined
                width: 10
            }
            AnchorChanges {
                target: orientationPanel
                anchors.right: undefined
                anchors.top: orientationPanelContainer.top
            }
        },
        State {
            name: "rot180"
            when: model.rotation === 4
            AnchorChanges {
                target: orientationPanel
                anchors.top: orientationPanelContainer.top
                anchors.bottom: undefined
            }
        },
        State {
            name: "rot270"
            when: model.rotation === 8
            PropertyChanges {
                target: orientationPanel
                height: undefined
                width: 10
            }
            AnchorChanges {
                target: orientationPanel
                anchors.left: undefined
                anchors.top: orientationPanelContainer.top
            }
        }
    ]

    OpacityMask {
        anchors.fill: orientationPanelContainer
        source: orientationPanelContainer
        maskSource: outline
    }

    property point dragStartPosition

    TapHandler {
        id: tapHandler
        property bool isLongPressed: false

        function bindSelectedOutput() {
            root.selectedOutput
                    = Qt.binding(function() { return model.index; });
        }
        onPressedChanged: {
            if (pressed) {
                bindSelectedOutput();
                dragStartPosition = Qt.point(output.x, output.y)
            } else {
                isLongPressed = false;
            }
        }
        onLongPressed: isLongPressed = true;
        longPressThreshold: 0.3
    }
    DragHandler {
        id: dragHandler
        enabled: kcm.outputModel && kcm.outputModel.rowCount() > 1
        acceptedButtons: Qt.LeftButton
        target: null

        onTranslationChanged: {
            var newX = dragStartPosition.x + translation.x;
            var newY = dragStartPosition.y + translation.y;
            model.position = getAbsolutePosition(Qt.point(newX, newY));
        }
        onActiveChanged: {
            if (!active) {
                screen.resetTotalSize();
            }
        }
    }

    // So we can show a grabby hand cursor when hovered over
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeAllCursor
        acceptedButtons: Qt.NoButton // Otherwise it interferes with the drag handler
        visible: kcm.outputModel && kcm.outputModel.rowCount() > 1
    }
}

