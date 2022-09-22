/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

import org.kde.kcm 1.6 as KCM

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 30

    property int selectedOutput: 0
    property int revertCountdown: 15

    // This is to fix Output dragging
    flickable.interactive: Kirigami.Settings.hasTransientTouchInput

    ColumnLayout {
        Kirigami.InlineMessage {
            // Note1: There is an implicit height binding loop error on
            //        first invocation. Seems to be an issue in Kirigami.
            // Note2: This should maybe go in header component of the KCM,
            //        but there seems to be another issue in Kirigami then
            //        being always hidden. Compare Night Color KCM with
            //        the same issue.
            id: invalidConfigMsg

            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: i18nc("@info", "All displays are disabled. Enable at least one.")
            showCloseButton: true

        }
        Kirigami.InlineMessage {
            id: errBackendMsg
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: i18n("No KScreen backend found. Please check your KScreen installation.")
            visible: false
            showCloseButton: false
        }
        Kirigami.InlineMessage {
            id: errSaveMsg
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: i18n("Outputs could not be saved due to error.")
            visible: false
            showCloseButton: true
        }
        Kirigami.InlineMessage {
            id: scaleMsg
            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            text: i18n("New global scale applied. Changes will come into effect after the system is restarted.")
            visible: false
            showCloseButton: true
            actions: [
                Kirigami.Action {
                    icon.name: "system-reboot"
                    text: i18n("Restart")
                    onTriggered: kcm.requestReboot();
                }
            ]
        }
        Kirigami.InlineMessage {
            id: connectMsg
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            visible: false
            showCloseButton: true
        }
        Kirigami.InlineMessage {
            id: revertMsg
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: i18n("Display configuration reverted.")
            visible: false
            showCloseButton: true
        }
        Kirigami.OverlaySheet {
            id: confirmMsg
            parent: root
            title: i18n("Keep display configuration?")
            onSheetOpenChanged: {
                if (sheetOpen) {
                    revertButton.forceActiveFocus()
                } else {
                    revertTimer.stop()
                }
            }
            showCloseButton: false
            contentItem: ColumnLayout {
                spacing: 0 // we manually add spacing in the Label item

                QQC2.Label {
                    Layout.fillWidth: true
                    Layout.maximumWidth: Math.round(root.width * 0.75)
                    Layout.topMargin: Kirigami.Units.largeSpacing * 2
                    Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                    text: i18np("Will revert to previous configuration in %1 second.",
                                "Will revert to previous configuration in %1 seconds.",
                                revertCountdown);
                    wrapMode: Text.WordWrap
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    QQC2.Button {
                        id: acceptButton
                        Keys.onPressed: event => {
                            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                                event.accepted = true
                                acceptButton.action.trigger()
                            }
                        }
                        action: QQC2.Action {
                            icon.name: "dialog-ok"
                            text: i18n("&Keep")
                            onTriggered: {
                                confirmMsg.close()
                            }
                        }
                    }
                    QQC2.Button {
                        id: revertButton
                        KeyNavigation.left: acceptButton
                        focus: true
                        Keys.onPressed: event => {
                            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                                event.accepted = true
                                revertButton.action.trigger()
                            }
                        }
                        action: QQC2.Action {
                            icon.name: "edit-undo"
                            text: i18n("&Revert")
                            shortcut: "Escape"
                            enabled: confirmMsg.sheetOpen
                            onTriggered: {
                                revertTimer.stop()
                                kcm.setStopUpdatesFromBackend(false)
                                kcm.revertSettings()
                            }
                        }
                    }
                }
            }
        }

        Connections {
            target: kcm
            function onInvalidConfig() {
                invalidConfigMsg.visible = true;
            }
            function onErrorOnSave() {
                errSaveMsg.visible = true;
            }
            function onGlobalScaleWritten() {
                scaleMsg.visible = true;
            }
            function onOutputConnect(connected) {
                if (connected) {
                    connectMsg.text = i18n("A new output has been added. Settings have been reloaded.");
                } else {
                    connectMsg.text = i18n("An output has been removed. Settings have been reloaded.");
                }
                connectMsg.visible = true;
            }
            function onBackendError() {
                errBackendMsg.visible = true;
            }
            function onSettingsReverted() {
                confirmMsg.close();
                revertMsg.visible = true;
            }
            function onShowRevertWarning() {
                revertCountdown = 15;
                confirmMsg.open();
                revertTimer.restart();
            }
            function onChanged() {
                invalidConfigMsg.visible = false;
                errSaveMsg.visible = false;
                scaleMsg.visible = false;
                revertMsg.visible = false;
            }
        }

        Screen {
            id: screen

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.smallSpacing

            enabled: kcm.outputModel && kcm.backendReady
            outputs: kcm.outputModel
        }

        Panel {
            enabled: kcm.outputModel && kcm.backendReady
            Layout.fillWidth: true
        }

        Timer {
            id: revertTimer
            interval: 1000
            running: false
            repeat: true

            onTriggered: {
                revertCountdown -= 1;
                if (revertCountdown < 1) {
                    revertTimer.stop();
                    kcm.revertSettings();
                }
            }
        }
    }
}
