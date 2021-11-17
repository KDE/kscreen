/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.9
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.4 as Kirigami

import org.kde.kcm 1.6 as KCM
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

        RowLayout {
            Kirigami.FormData.label: i18n("Resolution:")

            Controls.ComboBox {
                id: resolutionCombobox
                Layout.minimumWidth: Kirigami.Units.gridUnit * 11
                visible: count > 1
                model: element.resolutions
                currentIndex: element.resolutionIndex !== undefined ?
                                element.resolutionIndex : -1
                onActivated: element.resolutionIndex = currentIndex
            }
            // When the combobox is has only one item, it's basically non-interactive
            // and is serving purely in a descriptive role, so make this explicit by
            // using a label instead
            Controls.Label {
                id: singleResolutionLabel
                visible: !resolutionCombobox.visible
                text: element.resolutions[0]
            }
            KCM.ContextualHelpButton {
                visible: singleResolutionLabel.visible
                toolTipText: xi18nc("@info", "\"%1\" is the only resolution supported by this display.<nl/><nl/>Using unsupported resolutions was possible in the Plasma X11 session, but they were never guaranteed to work and are not available in this Plasma Wayland session.", singleResolutionLabel.text)
            }
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

        RowLayout {
            Kirigami.FormData.label: i18n("Refresh rate:")

            Controls.ComboBox {
                id: refreshRateCombobox
                Layout.minimumWidth: Kirigami.Units.gridUnit * 11
                visible: count > 1
                model: element.refreshRates
                currentIndex: element.refreshRateIndex ?
                                element.refreshRateIndex : 0
                onActivated: element.refreshRateIndex = currentIndex
            }
            // When the combobox is has only one item, it's basically non-interactive
            // and is serving purely in a descriptive role, so make this explicit by
            // using a label instead
            Controls.Label {
                id: singleRefreshRateLabel
                visible: !refreshRateCombobox.visible
                text: element.refreshRates[0]
            }
            KCM.ContextualHelpButton {
                visible: singleRefreshRateLabel.visible
                toolTipText: i18n("\"%1\" is the only refresh rate supported by this display.", singleRefreshRateLabel.text)
            }
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
            Kirigami.FormData.label: i18n("RGB Range:")
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: [
                { label: i18n("Automatic"), value: KScreen.Output.RgbRange.Automatic },
                { label: i18n("Full"), value: KScreen.Output.RgbRange.Full },
                { label: i18n("Limited"), value: KScreen.Output.RgbRange.Limited }
            ]
            textRole: "label"
            valueRole: "value"
            visible: element.capabilities & KScreen.Output.Capability.RgbRange

            onActivated: element.rgbRange = currentValue
            Component.onCompleted: currentIndex = indexOfValue(element.rgbRange);
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Color bit depth:")
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: [
                { label: i18n("5"), value: 5 },
                { label: i18n("8"), value: 8 },
                { label: i18n("10"), value: 10 },
                { label: i18n("16"), value: 16 },
            ]
            textRole: "label"
            valueRole: "value"
            visible: element.capabilities & KScreen.Output.Capability.Bpc

            onActivated: element.bpc = currentValue
            Component.onCompleted: currentIndex = indexOfValue(element.bpc);
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
