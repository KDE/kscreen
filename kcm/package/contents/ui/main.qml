/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

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
import QtQuick 2.9
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.4 as Kirigami

import org.kde.kcm 1.2 as KCM

KCM.SimpleKCM {
    id: root

    implicitWidth: units.gridUnit * 30
    implicitHeight: units.gridUnit * 38

    property int selectedOutput: 0

    ColumnLayout {
        Kirigami.InlineMessage {
            // Note1: There is an implicit height binding loop error on
            //        first invokation. Seems to be an issue in Kirigami.
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

        Connections {
            target: kcm
            onDangerousSave: dangerousSaveMsg.visible = true;
            onErrorOnSave: errSaveMsg.visible = true;
            onGlobalScaleWritten: scaleMsg.visible = true;
            onOutputConnect: {
                if (connected) {
                    connectMsg.text = i18n("A new output has been added. Settings have been reloaded.");
                } else {
                    connectMsg.text = i18n("An output has been removed. Settings have been reloaded.");
                }
                connectMsg.visible = true;
            }
            onBackendError: errBackendMsg.visible = true;

            onChanged: {
                dangerousSaveMsg.visible = false;
                errSaveMsg.visible = false;
                scaleMsg.visible = false;
            }
        }

        Screen {
            id: screen

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.max(root.width * 0.8, units.gridUnit * 26)
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing * 2

            enabled: kcm.outputModel && kcm.backendReady
            outputs: kcm.outputModel
        }

        Panel {
            enabled: kcm.outputModel && kcm.backendReady
            Layout.fillWidth: true
        }
    }
}
