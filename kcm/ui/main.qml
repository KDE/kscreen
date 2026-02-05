/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2025 Oliver Beard <olib141@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kitemmodels 1.0

import org.kde.kcmutils as KCM
import org.kde.private.kcm.kscreen 1.0 as KScreen

KCM.AbstractKCM {
    id: root

    // This will be overridden on selection, but ensures we start with an
    // enabled display as the initial selection
    property int selectedOutput: firstEnabledDisplayIndex()

    function firstEnabledDisplayIndex() {
        if (!(kcm.outputModel && kcm.backendReady)) return -1; // Wait for model

        for (let i = 0; i < kcm.outputModel.rowCount(); ++i) {
            // Return index of first enabled display
            if (kcm.outputModel.data(kcm.outputModel.index(i, 0), KScreen.OutputModel.EnabledRole)) {
                return i;
            }
        }

        return 0; // Otherwise, select the first display
    }

    property int revertCountdown: 15

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 30

    actions: Kirigami.Action {
        text: i18nc("@action:button Briefly show the display name in a popup label on each screen", "Identify Screens")
        icon.name: "documentinfo-symbolic"
        visible: kcm.multipleScreensAvailable
        onTriggered: kcm.identifyOutputs()
    }

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    Kirigami.PromptDialog {
        id: confirmMsg
        title: i18n("Keep display configuration?")
        // Force user interaction to accept; don't auto-close and accept on random clicks
        closePolicy: QQC2.Popup.NoAutoClose
        onVisibleChanged: {
            if (visible) {
                revertButton.forceActiveFocus()
            } else {
                revertTimer.stop();
            }
        }
        showCloseButton: false
        subtitle: i18np("Will revert to previous configuration in %1 second.",
                        "Will revert to previous configuration in %1 seconds.",
                        revertCountdown);

        parent: root.parent?.contentItem ?? root.parent

        footer: QQC2.DialogButtonBox {
            id: confirmDialogButtonBox
            QQC2.Button {
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                        event.accepted = true;
                        clicked();
                    }
                }
                icon.name: "dialog-ok"
                text: i18n("&Keep")
            }
            QQC2.Button {
                id: revertButton
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                        event.accepted = true;
                        clicked();
                    }
                }
                action: QQC2.Action {
                    icon.name: "edit-undo"
                    text: i18n("&Revert")
                    shortcut: "Escape"
                    enabled: confirmMsg.visible
                }
            }
            onAccepted: {
                confirmMsg.close();
            }
            onRejected: {
                revertTimer.stop();
                kcm.setStopUpdatesFromBackend(false);
                kcm.revertSettings();
            }
        }
    }

    Connections {
        target: kcm
        function onInvalidConfig(reason) {
            if (reason === KScreen.KCMKScreen.NoEnabledOutputs) {
                invalidConfigMsg.text = i18nc("@info", "All displays are disabled. Enable at least one.")
            } else if (reason === KScreen.KCMKScreen.ConfigHasGaps) {
                invalidConfigMsg.text = i18nc("@info", "Gaps between displays are not supported. Make sure all displays are touching.")
            }
            invalidConfigMsg.visible = true;
        }
        function onErrorOnSave(reason) {
            errSaveMsg.text = i18nc("The argument contains the reason for the failure", "Couldn’t apply display configuration: %1", reason)
            errSaveMsg.visible = true;
        }
        function onGlobalScaleWritten() {
            scaleMsg.visible = true;
        }
        function onOutputConnect(connected) {
            root.selectedOutput = Qt.binding(firstEnabledDisplayIndex);
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

    Timer {
        id: revertTimer
        interval: 1000
        running: false
        repeat: true

        onTriggered: {
            revertCountdown -= 1;
            if (revertCountdown < 1) {
                confirmDialogButtonBox.rejected();
            }
        }
    }

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: ColumnLayout {
        spacing: 0

        Kirigami.InlineMessage {
            id: invalidConfigMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Error
            showCloseButton: true
        }
        Kirigami.InlineMessage {
            id: errBackendMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Error
            text: i18n("No KScreen backend found. Please check your KScreen installation.")
            visible: false
            showCloseButton: false
        }
        Kirigami.InlineMessage {
            id: errSaveMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Error
            text: i18n("Outputs could not be saved due to error.")
            visible: false
            showCloseButton: true
        }
        Kirigami.InlineMessage {
            id: scaleMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Information
            text: i18n("Global scale changes will come into effect after the system is restarted.")
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
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Information
            visible: false
            showCloseButton: true
        }
        Kirigami.InlineMessage {
            id: revertMsg
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Information
            text: i18n("Display configuration reverted.")
            visible: false
            showCloseButton: true
        }
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        Kirigami.Dialog {
            id: reorderDialog

            title: i18nc("@title:window", "Change Priorities")
            showCloseButton: true
            standardButtons: Kirigami.Dialog.Ok
            padding: 0

            contentItem: ListView {
                id: reorderView

                implicitWidth: Math.min(root.width * 0.75, Kirigami.Units.gridUnit * 32)
                implicitHeight: contentHeight

                reuseItems: true
                model: KSortFilterProxyModel {
                    id: enabledOutputsModel
                    sourceModel: kcm.outputModel
                    filterRoleName: "enabled"
                    filterString: "true"
                    sortRoleName: "priority"
                    sortOrder: Qt.AscendingOrder
                }
                delegate: Kirigami.SwipeListItem {
                    id: delegate

                    property var output: model

                    width: ListView.view.width

                    background: null
                    contentItem: KD.TitleSubtitle {
                        title: delegate.output.display
                        subtitle: (delegate.output.priority === 1) ? i18n("Primary") : ""
                    }
                    actions: [
                        Kirigami.Action {
                            icon.name: "arrow-up"
                            text: i18n("Raise priority")
                            enabled: delegate.output.priority > 1
                            onTriggered: {
                                if (enabled) {
                                    delegate.output.priority -= 1;
                                }
                            }
                        },
                        Kirigami.Action {
                            icon.name: "arrow-down"
                            text: i18n("Lower priority")
                            enabled: delegate.output.priority < reorderView.count
                            onTriggered: {
                                if (enabled) {
                                    delegate.output.priority += 1;
                                }
                            }
                        }
                    ]
                }
            }
        }

        QQC2.ScrollView {
            id: navigationScrollView
            Layout.fillWidth: true

            visible: kcm.multipleScreensAvailable

            RowLayout {
                spacing: 0

                width: Math.max(implicitWidth, navigationScrollView.width)
                enabled: kcm.outputModel && kcm.backendReady

                // Surrounding items around layout provide centering when
                // excessive free space is available, without taking extra
                // space when not available due to uniformCellSizes.

                Item {
                    Layout.fillWidth: true
                }

                RowLayout {
                    spacing: 0
                    uniformCellSizes: true

                    Repeater {
                        model: kcm.outputModel
                        delegate: Kirigami.NavigationTabButton {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.minimumWidth: Kirigami.Units.gridUnit * 8
                            Layout.maximumWidth: Kirigami.Units.gridUnit * 12
                            Layout.fillWidth: true

                            text: model.display
                            icon.name: "monitor-symbolic"

                            Binding {
                                target: contentItem
                                when: !model.enabled
                                property: "opacity"
                                value: 0.6
                            }

                            checked: root.selectedOutput === model.index
                            onClicked: root.selectedOutput = model.index
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true

            visible: navigationScrollView.visible
        }

        Rectangle {
            id: screenContainer
            Layout.preferredHeight: Math.floor(Math.max(root.height * 0.4, Kirigami.Units.gridUnit * 14) * heightMultiplier)
            Layout.fillWidth: true
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View

            color: Kirigami.Theme.backgroundColor

            // Animating height directly would occur during window resize
            property real heightMultiplier: screen.interactive ? 1 : 0.5
            Behavior on heightMultiplier {
                PropertyAnimation {
                    easing.type: Easing.InOutQuad
                    duration: Kirigami.Units.shortDuration
                }
            }

            visible: kcm.multipleScreensAvailable

            ScreenView {
                id: screen
                anchors.fill: parent

                enabled: kcm.outputModel && kcm.backendReady
                outputs: kcm.outputModel

                interactive: false
            }

            QQC2.ToolButton {
                id: screenEdit
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: Kirigami.Units.smallSpacing

                enabled: kcm.outputModel && kcm.backendReady

                Accessible.description: text
                text: screen.interactive ? i18nc("@action:button Shrink the height of a view", "Collapse")
                                         : i18nc("@action:button Edit the arrangement of display outputs", "Edit Arrangement")

                icon.name: screen.interactive ? "collapse-symbolic"
                                              : "edit-symbolic"

                onClicked: screen.interactive = !screen.interactive
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true

            visible: screenContainer.visible
        }

        QQC2.ScrollView {
            id: panelScrollView
            Layout.fillWidth: true
            Layout.fillHeight: true

            contentHeight: panel.height + Kirigami.Units.smallSpacing * 2

            Item {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing

                Panel {
                    id: panel
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right

                    enabled: kcm.outputModel && kcm.backendReady
                    enabledOutputs: enabledOutputsModel
                    selectedOutput: root.selectedOutput
                    onReorder: reorderDialog.open()
                }
            }
        }
    }
}
