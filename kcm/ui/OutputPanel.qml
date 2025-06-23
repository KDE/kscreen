/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtCore
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.private.kcm.kscreen 1.0 as KScreen

Kirigami.FormLayout {
    id: root

    property KSortFilterProxyModel enabledOutputs
    property var element: model
    readonly property int comboboxWidth: Kirigami.Units.gridUnit * 12
    readonly property int spinboxWidth: Kirigami.Units.gridUnit * 4
    readonly property bool hdrAvailable: (element.capabilities & KScreen.Output.Capability.HighDynamicRange) && (element.capabilities & KScreen.Output.Capability.WideColorGamut)

    signal reorder()

    QQC2.CheckBox {
       Kirigami.FormData.label: i18nc("@label for a checkbox that says 'Enabled'", "Device:")
       text: i18n("Enabled")
       checked: element.enabled
       onToggled: element.enabled = checked
       visible: kcm.multipleScreensAvailable
    }

    RowLayout {
        visible: kcm.primaryOutputSupported && root.enabledOutputs.count >= 2

        QQC2.Button {
            visible: root.enabledOutputs.count >= 3
            text: i18n("Change Screen Priorities…")
            icon.name: "document-edit"
            onClicked: root.reorder();
        }

        QQC2.RadioButton {
            visible: root.enabledOutputs.count === 2
            text: i18n("Primary")
            checked: element.priority === 1
            onToggled: element.priority = 1
        }

        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info", "This determines which screen your main desktop appears on, along with any Plasma Panels in it. Some older games also use this setting to decide which screen to appear on.<nl/><nl/>It has no effect on what screen notifications or other windows appear on.")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Resolution:")
        Kirigami.FormData.buddyFor: resolutionCombobox

        QQC2.ComboBox {
            id: resolutionCombobox
            Layout.minimumWidth: root.comboboxWidth
            visible: count > 1
            model: element.resolutions
            onActivated: element.resolutionIndex = currentIndex;
            Component.onCompleted: currentIndex = Qt.binding(() => element.resolutionIndex);
        }
        // When the combobox is has only one item, it's basically non-interactive
        // and is serving purely in a descriptive role, so make this explicit by
        // using a label instead
        QQC2.Label {
            id: singleResolutionLabel
            visible: resolutionCombobox.count <= 1
            text: element.resolutions[0] || ""
        }
        Kirigami.ContextualHelpButton {
            visible: resolutionCombobox.count <= 1
            toolTipText: i18nc("@info", "“%1” is the only resolution supported by this display.", singleResolutionLabel.text)
        }
    }

    RowLayout {
        Layout.fillWidth: true
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16

        visible: kcm.perOutputScaling
        Kirigami.FormData.label: i18n("Scale:")
        Kirigami.FormData.buddyFor: scaleSlider

        QQC2.Slider {
            id: scaleSlider

            Accessible.description: i18nc("@info accessible description of slider value", "in percent of regular scale")

            Layout.fillWidth: true
            from: 50
            to: 300
            stepSize: 25
            live: true
            value: element.scale * 100
            onMoved: element.scale = value / 100
        }
        QQC2.SpinBox {
            id: spinbox
            // Because QQC2 SpinBox doesn't natively support decimal step
            // sizes: https://bugreports.qt.io/browse/QTBUG-67349
            // 120 is from the Wayland fractional scale protocol
            readonly property real factor: 120.0
            readonly property real realValue: value / factor

            Layout.minimumWidth: root.spinboxWidth
            Layout.maximumWidth: root.spinboxWidth

            from: 0.5 * factor
            to: 3.0 * factor
            stepSize: factor * 0.05 // 5% steps
            value: element.scale * factor
            validator: DoubleValidator {
                bottom: Math.min(spinbox.from, spinbox.to) * spinbox.factor
                top:  Math.max(spinbox.from, spinbox.to) * spinbox.factor
            }
            textFromValue: (value, locale) =>
                i18nc("Global scale factor expressed in percentage form", "%1%",
                    parseFloat(value * 1.0 / factor * 100.0))
            valueFromText: (text, locale) =>
                Number.fromLocaleString(locale, text.replace("%", "")) * factor / 100.0

            onValueModified: element.scale = realValue
        }
    }

    Orientation {}

    RowLayout {
        Kirigami.FormData.label: i18n("Refresh rate:")
        Kirigami.FormData.buddyFor: refreshRateCombobox

        QQC2.ComboBox {
            id: refreshRateCombobox
            Layout.minimumWidth: root.comboboxWidth
            visible: count > 1
            model: element.refreshRates
            onActivated: element.refreshRateIndex = currentIndex;
            Component.onCompleted: currentIndex = Qt.binding(() => element.refreshRateIndex);
        }
        // When the combobox is has only one item, it's basically non-interactive
        // and is serving purely in a descriptive role, so make this explicit by
        // using a label instead
        QQC2.Label {
            id: singleRefreshRateLabel
            visible: refreshRateCombobox.count <= 1
            text: element.refreshRates[0] || ""
        }
        Kirigami.ContextualHelpButton {
            visible: refreshRateCombobox.count <= 1
            toolTipText: i18nc("@info", "“%1” is the only refresh rate supported by this display.", singleRefreshRateLabel.text)
        }
    }

    QQC2.ComboBox {
        Kirigami.FormData.label: i18n("Adaptive sync:")
        Layout.minimumWidth: root.comboboxWidth
        model: [
            { label: i18n("Never"), value: KScreen.Output.VrrPolicy.Never },
            { label: i18n("Automatic"), value: KScreen.Output.VrrPolicy.Automatic },
            { label: i18n("Always"), value: KScreen.Output.VrrPolicy.Always },
        ]
        textRole: "label"
        valueRole: "value"
        visible: element.capabilities & KScreen.Output.Capability.Vrr

        onActivated: element.vrrPolicy = currentValue;
        Component.onCompleted: currentIndex = indexOfValue(element.vrrPolicy);
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Overscan:")
        Kirigami.FormData.buddyFor: overscanSpinbox
        visible: element.capabilities & KScreen.Output.Capability.Overscan

        QQC2.SpinBox {
            id: overscanSpinbox

            Layout.minimumWidth: root.spinboxWidth
            Layout.maximumWidth: root.spinboxWidth

            from: 0
            to: 100
            value: element.overscan
            onValueModified: element.overscan = value
            textFromValue: (value, locale) =>
            i18nc("Overscan expressed in percentage form", "%1%", value)
            valueFromText: (text, locale) =>
            Number.fromLocaleString(locale, text.replace("%", ""))
        }

        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info", "Determines how much padding is put around the image sent to the display to compensate for part of the content being cut off around the edges.<nl/><nl/>This is sometimes needed when using a TV as a screen.")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("RGB range:")
        Kirigami.FormData.buddyFor: rgbRangeCombobox
        visible: element.capabilities & KScreen.Output.Capability.RgbRange

        QQC2.ComboBox {
            id: rgbRangeCombobox
            Layout.minimumWidth: root.comboboxWidth
            model: [
                { label: i18n("Automatic"), value: KScreen.Output.RgbRange.Automatic },
                { label: i18n("Full"), value: KScreen.Output.RgbRange.Full },
                { label: i18n("Limited"), value: KScreen.Output.RgbRange.Limited }
            ]
            textRole: "label"
            valueRole: "value"

            onActivated: element.rgbRange = currentValue;
            Component.onCompleted: currentIndex = indexOfValue(element.rgbRange);
        }

        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info", "Determines whether the range of possible color values needs to be limited for the display. This should only be changed if the colors on the screen look washed out.")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18nc("@label:listbox", "Color profile:")
        Kirigami.FormData.buddyFor: colorProfileCombobox
        visible: (element.capabilities & (KScreen.Output.Capability.IccProfile | KScreen.Output.Capability.BuiltInColorProfile)) && !(element.hdr && root.hdrAvailable)
        spacing: Kirigami.Units.smallSpacing

        QQC2.ComboBox {
            id: colorProfileCombobox
            Layout.minimumWidth: root.comboboxWidth
            model: [
                {
                    text: i18nc("@item:inlistbox color profile", "None"),
                    value: KScreen.Output.ColorProfileSource.sRGB,
                    available: true
                },
                {
                    text: i18nc("@item:inlistbox color profile", "ICC profile"),
                    value: KScreen.Output.ColorProfileSource.ICC,
                    available: element.capabilities & KScreen.Output.Capability.IccProfile
                },
                {
                    text: i18nc("@item:inlistbox color profile", "Built-in"),
                    value: KScreen.Output.ColorProfileSource.EDID,
                    available: element.capabilities & KScreen.Output.Capability.BuiltInColorProfile
                }
            ]
            textRole: "text"
            valueRole: "value"

            onActivated: element.colorProfileSource = currentValue;
            Component.onCompleted: currentIndex = indexOfValue(element.colorProfileSource);

            delegate: QQC2.ItemDelegate {
                width: colorProfileCombobox.width
                text: modelData.text
                enabled: modelData.available
            }
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "Use the color profile built into the screen itself, if present. Note that built-in color profiles are sometimes wrong, and often inaccurate. For optimal color fidelity, calibration using a colorimeter is recommended.")
            visible: (!element.hdr || !root.hdrAvailable) && element.colorProfileSource == KScreen.Output.ColorProfileSource.EDID
        }
    }

    RowLayout {
        visible: (element.capabilities & KScreen.Output.Capability.IccProfile) && (element.colorProfileSource == KScreen.Output.ColorProfileSource.ICC)
        spacing: Kirigami.Units.smallSpacing

        Kirigami.ActionTextField {
            id: iccProfileField
            onTextChanged: element.iccProfilePath = text
            onTextEdited: element.iccProfilePath = text
            placeholderText: i18nc("@info:placeholder", "Enter ICC profile path…")
            enabled: !root.hdrAvailable || !element.hdr

            rightActions: Kirigami.Action {
                icon.name: "edit-clear-symbolic"
                visible: iccProfileField.text !== ""
                onTriggered: {
                    iccProfileField.text = ""
                }
            }

            Component.onCompleted: text = element.iccProfilePath;
        }

        QQC2.Button {
            icon.name: "document-open-symbolic"
            text: i18nc("@action:button", "Select ICC profile…")
            display: QQC2.AbstractButton.IconOnly
            onClicked: fileDialogComponent.incubateObject(root);
            enabled: !root.hdrAvailable || !element.hdr

            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

            Accessible.role: Accessible.Button
            Accessible.name: text
            Accessible.description: i18n("Opens a file picker for the ICC profile")
            Accessible.onPressAction: onClicked();
        }

        Component {
            id: fileDialogComponent

            FileDialog {
                id: fileDialog
                title: i18nc("@title:window", "Select ICC Profile")
                currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
                nameFilters: ["ICC profiles (*.icc *.icm)"]

                onAccepted: {
                    iccProfileField.text = urlToProfilePath(selectedFile);
                    destroy();
                }
                onRejected: destroy();
                Component.onCompleted: open();

                function urlToProfilePath(qmlUrl) {
                    const url = new URL(qmlUrl);
                    let path = decodeURIComponent(url.pathname);
                    // Remove the leading slash from the url
                    if (url.protocol === "file:" && path.charAt(1) === ':') {
                        path = path.substring(1);
                    }
                    return path;
                }
            }
        }

        Kirigami.ContextualHelpButton {
            visible: root.hdrAvailable && element.hdr
            toolTipText: i18nc("@info:tooltip", "ICC profiles aren’t compatible with HDR yet.")
        }
    }

    RowLayout {
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        Kirigami.FormData.label: i18nc("@label", "High Dynamic Range:")
        Kirigami.FormData.buddyFor: hdrCheckbox
        visible: root.hdrAvailable
        spacing: Kirigami.Units.smallSpacing

        QQC2.CheckBox {
            id: hdrCheckbox
            text: i18nc("@option:check", "Enable &HDR")
            checked: element.hdr
            onToggled: element.hdr = checked
        }

        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "HDR allows compatible applications to show brighter and more vivid colors.")
        }
    }

    QQC2.Button {
        id: hdrCalibrationButton
        text: i18nc("@action:button", "Calibrate HDR Brightness…")
        onClicked: kcm.startHdrCalibrator(element.name);

        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        visible: root.hdrAvailable && element.hdr

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        Accessible.role: Accessible.Button
        Accessible.name: text
        Accessible.description: i18n("Opens a window to calibrate HDR brightness")
        Accessible.onPressAction: onClicked();
    }

    RowLayout {
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        Kirigami.FormData.label: i18nc("@label:listbox", "Color accuracy:")
        Kirigami.FormData.buddyFor: colorAccuracyCombobox
        visible: element.capabilities & KScreen.Output.Capability.IccProfile
        spacing: Kirigami.Units.smallSpacing

        QQC2.ComboBox {
            id: colorAccuracyCombobox
            Layout.minimumWidth: root.comboboxWidth
            model: [
                { label: i18nc("@item:inlistbox tradeoff between efficiency and color accuracy", "Prefer efficiency"), value: KScreen.Output.ColorPowerTradeoff.PreferEfficiency },
                { label: i18nc("@item:inlistbox tradeoff between efficiency and color accuracy", "Prefer color accuracy"), value: KScreen.Output.ColorPowerTradeoff.PreferAccuracy }
            ]
            textRole: "label"
            valueRole: "value"

            onActivated: element.colorPowerPreference = currentValue;
            Component.onCompleted: currentIndex = indexOfValue(element.colorPowerPreference);
        }
        Kirigami.ContextualHelpButton {
            visible: element.colorPowerPreference == KScreen.Output.ColorPowerTradeoff.PreferAccuracy
            toolTipText: xi18nc("@info:tooltip", "Preferring color accuracy limits potentially inaccurate offloading of color operations to the display driver and increases the maximum color resolution.<nl/><nl/>
                                                  Note that this setting can have a large impact on performance.")
        }
        Kirigami.ContextualHelpButton {
            visible: element.colorPowerPreference == KScreen.Output.ColorPowerTradeoff.PreferEfficiency
                  && element.colorProfileSource == KScreen.Output.ColorProfileSource.ICC
                  && !(root.hdrAvailable && element.hdr)
            toolTipText: xi18nc("@info:tooltip", "Preferring efficiency simplifies the ICC profile to matrix+shaper, improving performance at the cost of color accuracy.<nl/><nl/>
                                                  Note that changing this setting can have a large impact on performance.")
        }
    }

    RowLayout {
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        Kirigami.FormData.label: i18nc("@label:listbox", "Limit color resolution to:")
        Kirigami.FormData.buddyFor: colorResolutionCombobox
        visible: (element.capabilities & KScreen.Output.Capability.MaxBitsPerColor) && element.minSupportedMaxBitsPerColor != element.maxSupportedMaxBitsPerColor
        spacing: Kirigami.Units.smallSpacing

        QQC2.ComboBox {
            id: colorResolutionCombobox
            Layout.minimumWidth: root.comboboxWidth
            model: element.colorPowerPreference == KScreen.Output.ColorPowerTradeoff.PreferEfficiency ? element.bitsPerColorOptionsPreferEfficiency : element.bitsPerColorOptionsPreferAccuracy
            readonly property var automaticMaxBpc: {
                var ret = element.maxSupportedMaxBitsPerColor;
                if (element.colorPowerPreference == KScreen.Output.ColorPowerTradeoff.PreferEfficiency) {
                    ret = Math.min(ret, 10);
                }
                if (element.automaticMaxBitsPerColorLimit != 0) {
                    ret = Math.min(ret, element.automaticMaxBitsPerColorLimit);
                }
                return ret;
            }
            displayText: {
                if (element.maxBitsPerColor == 0) {
                    return i18nc("@item:inlistbox color resolution", "Automatic  (%1 bits per color)", colorResolutionCombobox.automaticMaxBpc)
                } else {
                    return i18nc("@item:inlistbox color resolution", "%1 bits per color", element.maxBitsPerColor)
                }
            }

            onActivated: element.maxBitsPerColor = currentValue;
            Component.onCompleted: currentIndex = indexOfValue(element.maxBitsPerColor);

            delegate: QQC2.ItemDelegate {
                width: colorResolutionCombobox.width
                text: {
                    if (modelData == 0) {
                        return i18nc("@item:inlistbox color resolution", "Automatic (%1 bits per color)", colorResolutionCombobox.automaticMaxBpc)
                    } else {
                        return i18nc("@item:inlistbox color resolution", "%1 bits per color", modelData)
                    }
                }
                highlighted: colorResolutionCombobox.highlightedIndex == index
            }
        }
        Kirigami.ContextualHelpButton {
            toolTipText: {
                if (element.automaticMaxBitsPerColorLimit != 0 && element.maxBitsPerColor != 0 && element.maxSupportedMaxBitsPerColor > 8) {
                    return xi18nc("@info:tooltip", "Limits the color resolution of the image that is sent to the display. This does not affect screenshots or recordings.<nl/><nl/>
                                                    Because the display is currently connected through a dock, automatic color resolution has been temporarily reduced to 8 bits to avoid common dock issues.<nl/><nl/>
                                                    Due to graphics driver limitations, the actually used resolution is not known")
                } else {
                    return xi18nc("@info:tooltip", "Limits the color resolution of the image that is sent to the display. This does not affect screenshots or recordings.<nl/><nl/>
                                                    Limiting color resolution can be useful to work around display or graphics driver issues.<nl/><nl/>
                                                    Due to graphics driver limitations, the actually used resolution is not known")
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        spacing: Kirigami.Units.smallSpacing

        visible: (root.hdrAvailable && element.hdr) || (element.colorProfileSource != KScreen.Output.ColorProfileSource.sRGB)
        Kirigami.FormData.label: i18nc("@label", "sRGB color intensity:")
        Kirigami.FormData.buddyFor: sdrGamutSlider

        QQC2.Slider {
            id: sdrGamutSlider
            Layout.fillWidth: true
            from: 0
            to: 100
            stepSize: 10
            live: true
            value: element.sdrGamutWideness * 100
            onMoved: element.sdrGamutWideness = value / 100.0
        }
        QQC2.SpinBox {
            // Because QQC2 SpinBox doesn't natively support decimal step
            // sizes: https://bugreports.qt.io/browse/QTBUG-67349
            readonly property real factor: 20.0
            readonly property real realValue: value / factor

            Layout.minimumWidth: root.spinboxWidth
            Layout.maximumWidth: root.spinboxWidth

            from: 0
            to: 1.0 * factor
            stepSize: 1
            value: element.sdrGamutWideness * factor
            validator: DoubleValidator {
                bottom: Math.min(spinbox.from, spinbox.to) * spinbox.factor
                top:  Math.max(spinbox.from, spinbox.to) * spinbox.factor
            }
            textFromValue: (value, locale) =>
            i18nc("Color intensity factor expressed in percentage form", "%1%",
                  parseFloat(value * 1.0 / factor * 100.0))
            valueFromText: (text, locale) =>
            Number.fromLocaleString(locale, text.replace("%", "")) * factor / 100.0

            onValueModified: element.sdrGamutWideness = realValue
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "Increases the intensity of sRGB content on the screen.")
        }
    }

    RowLayout {
        id: ddcCiAllowedContainer
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        Kirigami.FormData.buddyFor: ddcCiAllowedCheckbox
        Kirigami.FormData.label: i18nc("@label", "Brightness:")
        visible: element.capabilities & KScreen.Output.Capability.DdcCi
        spacing: Kirigami.Units.smallSpacing

        QQC2.CheckBox {
            id: ddcCiAllowedCheckbox
            text: i18nc("@option:check", "Control hardware brightness with DDC/CI")
            checked: element.ddcCiAllowed
            onToggled: element.ddcCiAllowed = checked
        }

        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "DDC/CI is a feature supported by many monitors. Plasma can use it to adjust screen brightness with desktop controls, as if using the monitor's own hardware buttons and OSD menu.")
        }
    }

    RowLayout {
        Layout.fillWidth: true
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        spacing: Kirigami.Units.smallSpacing

        visible: (root.hdrAvailable && element.hdr) || (element.capabilities & KScreen.Output.Capability.BrightnessControl)
        Kirigami.FormData.label: ddcCiAllowedContainer.visible ? "" : ddcCiAllowedContainer.Kirigami.FormData.label
        Kirigami.FormData.buddyFor: brightnessSlider

        QQC2.Slider {
            id: brightnessSlider
            Layout.fillWidth: true
            from: 0
            to: 100
            stepSize: 5
            live: true
            value: element.brightness * 100.0
            onMoved: element.brightness = value / 100.0
        }
        QQC2.SpinBox {
            Layout.minimumWidth: root.spinboxWidth
            Layout.maximumWidth: root.spinboxWidth

            from: 0
            to: 100
            stepSize: 5
            value: element.brightness * 100.0
            onValueModified: element.brightness = value / 100.0
            textFromValue: (value, locale) => i18nc("Brightness expressed in percentage form", "%1%", value)
            valueFromText: (text, locale) => Number.fromLocaleString(locale, text.replace("%", ""))
        }
    }

    RowLayout {
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        spacing: Kirigami.Units.smallSpacing

        visible: !root.hdrAvailable && (element.capabilities & KScreen.Output.Capability.ExtendedDynamicRange)
        Kirigami.FormData.label: i18nc("@label", "Extended Dynamic Range:")
        Kirigami.FormData.buddyFor: edrCheckbox

        QQC2.CheckBox {
            id: edrCheckbox
            text: i18nc("@option:check", "Enable EDR")
            checked: element.edrPolicy == KScreen.Output.EdrPolicy.Always
            onToggled: element.edrPolicy = (checked ? KScreen.Output.EdrPolicy.Always : KScreen.Output.EdrPolicy.Never)
        }

        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info:tooltip", "EDR allows viewing HDR content on SDR displays by dynamically adjusting the backlight.<nl/><nl/>Note that this increases battery usage while viewing HDR content.")
        }
    }

    QQC2.ComboBox {
        Kirigami.FormData.label: i18n("Replica of:")
        Layout.minimumWidth: root.comboboxWidth
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        model: element.replicationSourceModel
        visible: kcm.outputReplicationSupported && kcm.multipleScreensAvailable

        onModelChanged: enabled = (count > 1);
        onCountChanged: enabled = (count > 1);

        Component.onCompleted: currentIndex = element.replicationSourceIndex;
        onActivated: element.replicationSourceIndex = currentIndex;
    }
}
