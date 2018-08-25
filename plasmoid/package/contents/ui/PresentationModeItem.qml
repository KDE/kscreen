/*
 * Copyright (c) 2018 Kai Uwe Broulik <kde@broulik.de>
 *                    Work sponsored by the LiMux project of
 *                    the city of Munich.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
