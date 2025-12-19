/*
    SPDX-FileCopyrightText: 2017 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.config as KConfig
import org.kde.kcmutils as KCMUtils
import org.kde.ksvg as KSvg
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

import org.kde.kscreen.osd

Control {
    id: root

    readonly property string kcmName: "kcm_kscreen"
    readonly property bool kcmAllowed: KConfig.KAuthorized.authorizeControlModule(kcmName)

    property string infoText
    property var actions
    signal clicked(int actionId)

    function moveRight() {
        actionRepeater.currentIndex = content.wrappingAdd(actionRepeater.count,
            actionRepeater.currentIndex, 1, false);
    }

    leftPadding: shadow.margins.left + background.margins.left
    rightPadding: shadow.margins.right + background.margins.right
    topPadding: shadow.margins.top + background.margins.top
    bottomPadding: shadow.margins.bottom + background.margins.bottom

    component OsdButton : PlasmaComponents.Button {
        required property int index
        display: PlasmaComponents.Button.IconOnly

        // use checked only indirectly, since its binding will break
        readonly property bool current: index === actionRepeater.currentIndex
        onCurrentChanged: {
            if (current) {
                checked = true;
                root.infoText = text;
                forceActiveFocus();
            } else {
                checked = false;
            }
        }

        onHoveredChanged: {
            if (hovered) {
                actionRepeater.currentIndex = index;
            }
        }
        onActiveFocusChanged: {
            if (activeFocus) {
                actionRepeater.currentIndex = index;
            }
        }
    }

    contentItem : ColumnLayout {
        id: content

        RowLayout {
            Repeater {
                id: actionRepeater
                property int currentIndex: 0
                model: root.actions
                delegate: OsdButton {
                    required property var modelData
                    property int actionId: modelData.action

                    text: modelData.label
                    icon.name: modelData.iconName
                    icon.height: Kirigami.Units.gridUnit * 8
                    icon.width: Kirigami.Units.gridUnit * 8

                    onClicked: root.clicked(actionId)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Math.floor(Kirigami.Units.smallSpacing / 2)
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                Layout.fillWidth: true
                // Keep heading visually centered
                Layout.leftMargin: configureButton.visible ? (configureButton.width + parent.spacing) : 0
                text: root.infoText
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 2
                wrapMode: Text.WordWrap
            }

            OsdButton {
                id: configureButton
                index: actionRepeater.count
                text: i18nc("@info:tooltip", "Configure displays in more detail")
                icon.name: "configure"
                visible: root.kcmAllowed
                onClicked: {
                    KCMUtils.KCMLauncher.openSystemSettings(root.kcmName);
                    root.clicked(OsdAction.NoAction);
                }
            }
        }

        // Shift current by delta, but do not wrap around when repeat is true.
        function wrappingAdd(count: int, current: int, delta: int, repeat: bool): int {
            const next = current + delta;
            // Rule out invalid states.
            if (count === 0 || current < 0 || current >= count) {
                return current;
            }
            // Don't wrap on autorepeat.
            if (repeat && (next < 0 || next >= count)) {
                return current;
            }
            // Add an extra `count`, so that wrapping % works predictably with positive values only.
            // This assumes that delta is not smaller than `-count` (usually just -1, 0 or +1).
            return (next + count) % count;
        }

        function move(event) {
            actionRepeater.currentIndex = wrappingAdd(actionRepeater.count, actionRepeater.currentIndex,
                (event.key === Qt.Key_Left) ? -1 : 1, event.isAutoRepeat);
        }

        Keys.onPressed: (event) => {
            switch (event.key) {
                case Qt.Key_Return:
                case Qt.Key_Enter:
                    root.clicked(actionRepeater.itemAt(actionRepeater.currentIndex).actionId)
                    break
                case Qt.Key_Right:
                case Qt.Key_Left:
                    move(event)
                    break
                case Qt.Key_Up:
                    if (actionRepeater.currentIndex === actionRepeater.count) {
                        actionRepeater.currentIndex = actionRepeater.count - 1;
                    }
                    break;
                case Qt.Key_Down:
                    // TODO Always allow going down and remember which button was selected
                    // when going back up.
                    if (actionRepeater.currentIndex === actionRepeater.count - 1) {
                        actionRepeater.currentIndex = actionRepeater.count;
                    }
                    break;

                case Qt.Key_Escape:
                    root.clicked(OsdAction.NoAction)
                    break
            }
        }
    }

      background: KSvg.FrameSvgItem {
        id: shadow
        imagePath: "dialogs/background"
        prefix: "shadow"

        KSvg.FrameSvgItem {
            id: background
            anchors.leftMargin: shadow.margins.left
            anchors.rightMargin: shadow.margins.right
            anchors.topMargin: shadow.margins.top
            anchors.bottomMargin: shadow.margins.bottom
            anchors.fill: parent
            imagePath: "solid/dialogs/background"
        }
    }
}
