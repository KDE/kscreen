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
import org.kde.private.kcm.kscreen 1.0 as KScreen

ColumnLayout {
    id: outputPanel
    property var element: model

    Kirigami.FormLayout {
        twinFormLayouts: globalSettingsLayout

        Controls.CheckBox {
           text: i18n("Enabled")
           checked: element.enabled
           onClicked: element.enabled = checked
           visible: kcm.outputModel.rowCount() > 1
        }

        Controls.CheckBox {
           text: i18n("Primary")
           checked: element.primary
           onClicked: element.primary = checked
           visible: kcm.primaryOutputSupported && kcm.outputModel.rowCount() > 1
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Resolution:")
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: element.resolutions
            currentIndex: element.resolutionIndex !== undefined ?
                              element.resolutionIndex : -1
            onActivated: element.resolutionIndex = currentIndex
        }

        RowLayout {
            Layout.fillWidth: true
            // Set the same limit as the device ComboBox
            Layout.maximumWidth: Kirigami.Units.gridUnit * 16

            visible: kcm.perOutputScaling
            Kirigami.FormData.label: i18n("Scale:")

            Controls.Slider {
                id: scaleSlider

                Layout.fillWidth: true
                from: 0.5
                to: 3
                stepSize: 0.25
                live: true
                value: element.scale
                onMoved: element.scale = value
            }
            Controls.SpinBox {
                id: spinbox
                // Because QQC2 SpinBox doesn't natively support decimal step
                // sizes: https://bugreports.qt.io/browse/QTBUG-67349
                property real factor: 20.0
                property real realValue: value / factor

                from : 0.5 * factor
                to : 3.0 * factor
                stepSize: 0.05 * factor
                value: element.scale * factor
                validator: DoubleValidator {
                    bottom: Math.min(spinbox.from, spinbox.to) * spinbox.factor
                    top:  Math.max(spinbox.from, spinbox.to) * spinbox.factor
                }
                textFromValue: function(value, locale) {
                    return i18nc("Global scale factor expressed in percentage form", "%1%", parseFloat(value * 1.0 / factor * 100.0));
                }
                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text.replace("%", "")) * factor / 100.0
                }
                onValueModified: element.scale = realValue
            }
        }

        Orientation {}

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Refresh rate:")
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: element.refreshRates
            currentIndex: element.refreshRateIndex ?
                              element.refreshRateIndex : 0
            onActivated: element.refreshRateIndex = currentIndex
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Adaptive sync:")
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: [
                { label: i18n("Never"), value: KScreen.Output.VrrPolicy.Never },
                { label: i18n("Always"), value: KScreen.Output.VrrPolicy.Always },
                { label: i18n("Automatic"), value: KScreen.Output.VrrPolicy.Automatic }
            ]
            textRole: "label"
            valueRole: "value"
            visible: element.capabilities & KScreen.Output.Capability.Vrr

            onActivated: element.vrrPolicy = currentValue
            Component.onCompleted: currentIndex = indexOfValue(element.vrrPolicy);
        }

        Controls.SpinBox {
            Kirigami.FormData.label: i18n("Overscan:")
            from: 0
            to: 100
            value: element.overscan
            onValueModified: element.overscan = value
            visible: element.capabilities & KScreen.Output.Capability.Overscan
            textFromValue: function(value, locale) {
                return value + '%';
            }
            valueFromText: function(text, locale) {
                return parseInt(text.replace("%", ""))
            }
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Replica of:")
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: element.replicationSourceModel
            visible: kcm.outputReplicationSupported && kcm.outputModel && kcm.outputModel.rowCount() > 1

            onModelChanged: enabled = (count > 1);
            onCountChanged: enabled = (count > 1);

            currentIndex: element.replicationSourceIndex
            onActivated: element.replicationSourceIndex = currentIndex
        }
    }
}
