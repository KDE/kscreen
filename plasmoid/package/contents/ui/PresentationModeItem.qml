/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick.Controls as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3

ColumnLayout {
    spacing: Kirigami.Units.smallSpacing

    PlasmaComponents3.Switch {
        id: presentationModeSwitch
        Layout.fillWidth: true
        // Remove spacing between checkbox and the explanatory label below
        Layout.bottomMargin: -parent.spacing
        text: i18n("Enable Presentation Mode")

        onCheckedChanged: {
            if (checked === root.presentationModeEnabled) {
                return;
            }

            // disable Switch while job is running
            presentationModeSwitch.enabled = false;

            const service = pmSource.serviceForSource("PowerDevil");

            if (checked) {
                const op = service.operationDescription("beginSuppressingScreenPowerManagement");
                op.reason = i18n("User enabled presentation mode");

                const job = service.startOperationCall(op);
                job.finished.connect(job => {
                    presentationModeCookie = job.result;
                    presentationModeSwitch.enabled = true;
                });
            } else {
                const op = service.operationDescription("stopSuppressingScreenPowerManagement");
                op.cookie = presentationModeCookie;

                const job = service.startOperationCall(op);
                job.finished.connect(job => {
                    presentationModeCookie = -1;
                    presentationModeSwitch.enabled = true;
                });
            }
        }
    }

    QQC2.Label {
        Layout.fillWidth: true
        Layout.leftMargin: presentationModeSwitch.indicator.width + presentationModeSwitch.spacing
        font: Kirigami.Theme.smallFont
        text: i18n("This will prevent your screen and computer from turning off automatically.")
        wrapMode: Text.WordWrap
        opacity: 0.7
    }

    InhibitionHint {
        Layout.fillWidth: true
        Layout.leftMargin: presentationModeSwitch.indicator.width + presentationModeSwitch.spacing

        iconSource: pmSource.inhibitions.length > 0 ? pmSource.inhibitions[0].Icon || "" : ""
        text: {
            const inhibitions = pmSource.inhibitions;
            const inhibition = inhibitions[0];
            if (inhibitions.length > 1) {
                return i18ncp("Some Application and n others enforce presentation mode",
                              "%2 and %1 other application are enforcing presentation mode.",
                              "%2 and %1 other applications are enforcing presentation mode.",
                              inhibitions.length - 1, inhibition.Name) // plural only works on %1
            } else if (inhibitions.length === 1) {
                if (!inhibition.Reason) {
                    return i18nc("Some Application enforce presentation mode",
                                 "%1 is enforcing presentation mode.", inhibition.Name)
                } else {
                    return i18nc("Some Application enforce presentation mode: Reason provided by the app",
                                 "%1 is enforcing presentation mode: %2", inhibition.Name, inhibition.Reason)
                }
            } else {
                return "";
            }
        }
    }
}
