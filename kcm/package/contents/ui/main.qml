/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as Controls

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcm 1.2 as KCM

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 38

    property int selectedOutput: 0
    property int revertCountdown: 30

    ColumnLayout {
        Kirigami.InlineMessage {
            // Note1: There is an implicit height binding loop error on
            //        first invocation. Seems to be an issue in Kirigami.
            // Note2: This should maybe go in header component of the KCM,
            //        but there seems to be another issue in Kirigami then
            //        being always hidden. Compare Night Color KCM with
            //        the same issue.
            id: dangerousSaveMsg

            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            text: i18n("Are you sure you want to disable all outputs? This might render the device unusable.")
            showCloseButton: true

            actions: [
                Kirigami.Action {
                    iconName: "dialog-ok"
                    text: i18n("Disable All Outputs")
                    onTriggered: {
                        dangerousSaveMsg.visible = false;
                        kcm.forceSave();
                    }
                }
            ]
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
            text: i18n("New global scale applied. Change will come into effect after restart.")
            visible: false
            showCloseButton: true
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
            property bool keepConfig: false
            property bool userInteraction: false
            parent: root.parent
            title: i18n("Keep display configuration?")
            onSheetOpenChanged: {
                if (sheetOpen) {
                    revertButton.forceActiveFocus()
                    confirmMsg.keepConfig = false
                    confirmMsg.userInteraction = false
                } else {
                    if (!confirmMsg.keepConfig) {
                        kcm.revertSettings()
                        if (!confirmMsg.userInteraction) {
                            revertMsg.visible = true
                        }
                    }
                    revertTimer.stop()
                }
            }
            showCloseButton: false
            contentItem: Controls.Label {
                text: i18np("Will revert to previous configuration in %1 second.",
                            "Will revert to previous configuration in %1 seconds.",
                            revertCountdown);
                wrapMode: Qt.WordWrap
            }
            footer: RowLayout {
                Controls.Button {
                    id: acceptButton
                    Layout.fillWidth: true
                    action: Controls.Action {
                        icon.name: "dialog-ok"
                        text: i18n("&Keep")
                        onTriggered: {
                            confirmMsg.keepConfig = true
                            confirmMsg.userInteraction = true
                            confirmMsg.close()
                        }
                    }
                }
                Controls.Button {
                    id: revertButton
                    Layout.fillWidth: true
                    KeyNavigation.left: acceptButton
                    focus: true
                    action: Controls.Action {
                        icon.name: "edit-undo"
                        text: i18n("&Revert")
                        shortcut: "Escape"
                        onTriggered: {
                            confirmMsg.userInteraction = true
                            confirmMsg.close()
                        }
                    }
                }
            }
        }

        Connections {
            target: kcm
            function onDangerousSave() {
                dangerousSaveMsg.visible = true;
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
            }
            function onShowRevertWarning() {
                revertCountdown = 30;
                confirmMsg.open();
                revertTimer.restart();
            }
            function onChanged() {
                dangerousSaveMsg.visible = false;
                errSaveMsg.visible = false;
                scaleMsg.visible = false;
                revertMsg.visible = false;
            }
        }

        Screen {
            id: screen

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.max(root.width * 0.8, Kirigami.Units.gridUnit * 26)
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing * 2

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
                    this.stop();
                    kcm.revertSettings();
                    return;
                }
            }
        }
    }
}
