/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.8
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    spacing: units.smallSpacing

    PlasmaComponents.CheckBox {
        id: checkBox
        Layout.fillWidth: true
        // Remove spacing between checkbox and the explanatory label below
        Layout.bottomMargin: -parent.spacing
        text: i18n("Enable Presentation Mode")

        onCheckedChanged: {
            if (checked === root.presentationModeEnabled) {
                return;
            }

            // disable CheckBox while job is running
            checkBox.enabled = false;

            var service = pmSource.serviceForSource("PowerDevil");

            if (checked) {
                var op = service.operationDescription("beginSuppressingScreenPowerManagement");
                op.reason = i18n("User enabled presentation mode");

                var job = service.startOperationCall(op);
                job.finished.connect(function (job) {
                    presentationModeCookie = job.result;
                    checkBox.enabled = true;
                });
            } else {
                var op = service.operationDescription("stopSuppressingScreenPowerManagement");
                op.cookie = presentationModeCookie;

                var job = service.startOperationCall(op);
                job.finished.connect(function (job) {
                    presentationModeCookie = -1;
                    checkBox.enabled = true;
                });
            }
        }
    }

    // so we can align the labels below with the checkbox
    PlasmaComponents.CheckBox {
        id: checkBoxMetrics
        visible: false
    }

    PlasmaExtras.DescriptiveLabel {
        Layout.fillWidth: true
        Layout.leftMargin: checkBoxMetrics.width
        font.pointSize: theme.smallestFont.pointSize
        text: i18n("This will prevent your screen and computer from turning off automatically.")
        wrapMode: Text.WordWrap
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.leftMargin: checkBoxMetrics.width
        spacing: units.smallSpacing

        PlasmaCore.IconItem {
            Layout.preferredWidth: units.iconSizes.medium
            Layout.preferredHeight: units.iconSizes.medium
            source: pmSource.inhibitions[0] ? pmSource.inhibitions[0].Icon || "" : ""
            visible: valid
        }

        PlasmaComponents.Label {
            Layout.fillWidth: true
            Layout.maximumWidth: Math.min(units.gridUnit * 20, implicitWidth)
            font.pointSize: theme.smallestFont.pointSize
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            textFormat: Text.PlainText
            text: {
                var inhibitions = pmSource.inhibitions;
                if (inhibitions.length > 1) {
                    return i18ncp("Some Application and n others enforce presentation mode",
                                  "%2 and %1 other application are enforcing presentation mode.",
                                  "%2 and %1 other applications are enforcing presentation mode.",
                                  inhibitions.length - 1, inhibitions[0].Name) // plural only works on %1
                } else if (inhibitions.length === 1) {
                    if (!inhibitions[0].Reason) {
                        return i18nc("Some Application enforce presentation mode",
                                     "%1 is enforcing presentation mode.", inhibitions[0].Name)
                    } else {
                        return i18nc("Some Application enforce presentation mode: Reason provided by the app",
                                     "%1 is enforcing presentation mode: %2", inhibitions[0].Name, inhibitions[0].Reason)
                    }
                } else {
                    return "";
                }
            }
        }
    }
}
