/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels

import org.kde.private.kcm.kscreen as KScreen

Kirigami.Form {
    id: root

    property KSortFilterProxyModel enabledOutputs
    property var element: model
    readonly property int comboboxWidth: Kirigami.Units.gridUnit * 12
    readonly property int sliderWidth: Kirigami.Units.gridUnit * 12
    readonly property int maxSpinboxWidth: Kirigami.Units.gridUnit * 7
    readonly property bool hdrAvailable: (element.capabilities & KScreen.Output.Capability.HighDynamicRange) && (element.capabilities & KScreen.Output.Capability.WideColorGamut)
    readonly property bool hdrActive: hdrAvailable && element.hdr
    readonly property var colorProfileSource: hdrActive ? element.hdrColorProfileSource : element.colorProfileSource

    signal reorder()

    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@label for a checkbox that says 'Enabled'", "Device:")
            visible: kcm.multipleScreensAvailable
            contentItem: QQC2.CheckBox {
                text: i18n("Enabled")
                checked: element.enabled
                onToggled: element.enabled = checked
            }
        }

        Kirigami.FormEntry {
            visible: root.enabledOutputs.count >= 2
            contentItem: RowLayout {
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
        }

        Kirigami.FormEntry {
            title: i18n("Resolution:")
            contentItem: RowLayout {
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
        }

        Kirigami.FormEntry {
            title: i18n("Scale:")
            visible: element.replicationSourceIndex == 0
            contentItem: RowLayout {
                Layout.fillWidth: true
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14

                Kirigami.FormData.buddyFor: scaleSlider

                QQC2.Slider {
                    id: scaleSlider

                    Accessible.description: i18nc("@info accessible description of slider value", "in percent of regular scale")

                    Kirigami.StyleHints.tickMarkStepSize: stepSize
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.sliderWidth
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

                    Layout.maximumWidth: root.maxSpinboxWidth

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
        }

        Kirigami.FormEntry {
            contentItem: Orientation {}
        }

        Kirigami.FormEntry {
            title: i18n("Refresh rate:")
            contentItem: RowLayout {
                Layout.fillWidth: false
                Kirigami.FormData.buddyFor: refreshRateCombobox.visible ? refreshRateCombobox : singleRefreshRateLabel

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
            }
            trailingItems: Kirigami.ContextualHelpButton {
                visible: refreshRateCombobox.count <= 1
                toolTipText: resolutionCombobox.count <= 1 ? i18nc("@info", "“%1” is the only refresh rate supported by this display.", singleRefreshRateLabel.text)
                                                       : i18nc("@info", "“%1” is the only refresh rate supported by this display at the current resolution.", singleRefreshRateLabel.text)
            }
        }

        Kirigami.FormEntry {
            title: i18n("Adaptive sync:")
            visible: element.capabilities & KScreen.Output.Capability.Vrr
            contentItem: QQC2.ComboBox {
                Layout.minimumWidth: root.comboboxWidth
                model: [
                    { label: i18n("Never"), value: KScreen.Output.VrrPolicy.Never },
                    { label: i18n("Automatic"), value: KScreen.Output.VrrPolicy.Automatic },
                    { label: i18n("Always"), value: KScreen.Output.VrrPolicy.Always },
                ]
                textRole: "label"
                valueRole: "value"

                onActivated: element.vrrPolicy = currentValue;
                Component.onCompleted: currentIndex = indexOfValue(element.vrrPolicy);
            }
        }

        Kirigami.FormEntry {
            title: i18n("Overscan:")
            visible: element.capabilities & KScreen.Output.Capability.Overscan
            contentItem: QQC2.SpinBox {
                id: overscanSpinbox

                Layout.maximumWidth: root.maxSpinboxWidth

                from: 0
                to: 100
                value: element.overscan
                onValueModified: element.overscan = value
                textFromValue: (value, locale) =>
                i18nc("Overscan expressed in percentage form", "%1%", value)
                valueFromText: (text, locale) =>
                Number.fromLocaleString(locale, text.replace("%", ""))
            }

            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: xi18nc("@info", "Determines how much padding is put around the image sent to the display to compensate for part of the content being cut off around the edges.<nl/><nl/>This is sometimes needed when using a TV as a screen.")
            }
        }

        Kirigami.FormEntry {
            title: i18n("RGB range:")
            visible: element.capabilities & KScreen.Output.Capability.RgbRange
            contentItem: QQC2.ComboBox {
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

            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: xi18nc("@info", "Determines whether the range of possible color values needs to be limited for the display. This should only be changed if the colors on the screen look washed out.")
            }
        }

        // for SDR
        Kirigami.FormEntry {
            visible: ((element.capabilities & KScreen.Output.Capability.IccProfile) || (element.capabilities & KScreen.Output.Capability.BuiltInColorProfile)) && !root.hdrActive
            contentItem: ColorProfileSelector {
                colorProfileSource: element.colorProfileSource
                onSourceChanged: element.colorProfileSource = colorProfileSource

                supportsNoProfile: true
                supportsIccProfile: (element.capabilities & KScreen.Output.Capability.IccProfile)
                supportsBuiltInProfile: (element.capabilities & KScreen.Output.Capability.BuiltInColorProfile)
                comboboxWidth: root.comboboxWidth
            }
        }

        Kirigami.FormEntry {
            visible: (element.capabilities & KScreen.Output.Capability.IccProfile)
                  && (element.colorProfileSource == KScreen.Output.ColorProfileSource.ICC)
                  && !root.hdrActive
            contentItem: IccSelector {
                iccProfilePath: element.iccProfilePath
                onPathChanged: element.iccProfilePath = iccProfilePath
            }
        }

        // for HDR
        Kirigami.FormEntry {
            visible: ((element.capabilities & KScreen.Output.Capability.HdrIccProfile) || (element.capabilities & KScreen.Output.Capability.BuiltInColorProfile)) && root.hdrActive
            contentItem: ColorProfileSelector {
                colorProfileSource: element.hdrColorProfileSource
                onSourceChanged: element.hdrColorProfileSource = colorProfileSource

                supportsNoProfile: false
                supportsIccProfile: (element.capabilities & KScreen.Output.Capability.HdrIccProfile)
                supportsBuiltInProfile: (element.capabilities & KScreen.Output.Capability.BuiltInColorProfile)
                comboboxWidth: root.comboboxWidth
            }
        }

        Kirigami.FormEntry {
            visible: (element.capabilities & KScreen.Output.Capability.HdrIccProfile)
                  && (element.hdrColorProfileSource == KScreen.Output.ColorProfileSource.ICC)
                  && root.hdrActive
            contentItem: IccSelector {
                iccProfilePath: element.hdrIccProfilePath
                onPathChanged: element.hdrIccProfilePath = iccProfilePath
            }
        }

        Kirigami.FormEntry {
            title: i18nc("@label", "High Dynamic Range:")
            visible: root.hdrAvailable
            contentItem: QQC2.CheckBox {
                id: hdrCheckbox
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                text: i18nc("@option:check", "Enable &HDR")
                checked: element.hdr
                onToggled: element.hdr = checked
            }

            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "HDR allows compatible applications to show brighter and more vivid colors.")
            }
        }

        Kirigami.FormEntry {
            visible: root.hdrActive && element.colorProfileSource != KScreen.Output.ColorProfileSource.ICC
            contentItem: QQC2.Button {
                id: hdrCalibrationButton
                text: i18nc("@action:button", "Calibrate HDR Brightness…")
                onClicked: kcm.startHdrCalibrator(element.name);
                enabled: !kcm.needsSave

                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14

                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                Accessible.role: Accessible.Button
                Accessible.name: text
                Accessible.description: i18n("Opens a window to calibrate HDR brightness")
                Accessible.onPressAction: onClicked();
            }
            trailingItems: Kirigami.ContextualHelpButton {
                visible: hdrCalibrationButton.visible && !hdrCalibrationButton.enabled
                toolTipText: xi18nc("@info:tooltip", "HDR calibration can only be started if all settings are applied.")
            }
        }

        Kirigami.FormEntry {
            title: i18nc("@label:listbox", "Color accuracy:")
            visible: element.capabilities & KScreen.Output.Capability.IccProfile
            contentItem: QQC2.ComboBox {
                id: colorAccuracyCombobox
                Layout.minimumWidth: root.comboboxWidth
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                model: [
                    { label: i18nc("@item:inlistbox tradeoff between efficiency and color accuracy", "Prefer efficiency"), value: KScreen.Output.ColorPowerTradeoff.PreferEfficiency },
                    { label: i18nc("@item:inlistbox tradeoff between efficiency and color accuracy", "Prefer color accuracy"), value: KScreen.Output.ColorPowerTradeoff.PreferAccuracy }
                ]
                textRole: "label"
                valueRole: "value"

                onActivated: element.colorPowerPreference = currentValue;
                Component.onCompleted: currentIndex = indexOfValue(element.colorPowerPreference);
            }

            trailingItems: [
                Kirigami.ContextualHelpButton {
                    visible: element.colorPowerPreference == KScreen.Output.ColorPowerTradeoff.PreferAccuracy
                    toolTipText: xi18nc("@info:tooltip", "Preferring color accuracy limits potentially inaccurate offloading of color operations to the display driver and increases the maximum color resolution.<nl/><nl/>\
Note that this setting can have a large impact on performance.")
                },
                Kirigami.ContextualHelpButton {
                    visible: element.colorPowerPreference == KScreen.Output.ColorPowerTradeoff.PreferEfficiency
                        && root.colorProfileSource == KScreen.Output.ColorProfileSource.ICC
                    toolTipText: xi18nc("@info:tooltip", "Preferring efficiency simplifies the ICC profile to matrix+shaper, improving performance at the cost of color accuracy.<nl/><nl/>\
Note that changing this setting can have a large impact on performance.")
                }
            ]
        }

        Kirigami.FormEntry {
            title: i18nc("@label:listbox", "Limit color resolution to:")
            visible: (element.capabilities & KScreen.Output.Capability.MaxBitsPerColor) && element.minSupportedMaxBitsPerColor != element.maxSupportedMaxBitsPerColor
            contentItem: QQC2.ComboBox {
                id: colorResolutionCombobox
                Layout.minimumWidth: root.comboboxWidth
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
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

            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: {
                    // Keep the weird indentation so multiline strings get extracted properly for translation
                    if (element.automaticMaxBitsPerColorLimit != 0 && element.maxBitsPerColor != 0 && element.maxSupportedMaxBitsPerColor > 8) {
                        return xi18nc("@info:tooltip", "Limits the color resolution of the image that is sent to the display. This does not affect screenshots or recordings.<nl/><nl/>\
Because the display is currently connected through a dock, automatic color resolution has been temporarily reduced to 8 bits to avoid common dock issues.<nl/><nl/>\
Due to graphics driver limitations, the actually used resolution cannot be known.")
                    } else {
                        return xi18nc("@info:tooltip", "Limits the color resolution of the image that is sent to the display. This does not affect screenshots or recordings.<nl/><nl/>\
Limiting color resolution can be useful to work around display or graphics driver issues.<nl/><nl/>\
Due to graphics driver limitations, the actually used resolution cannot be known.")
                    }
                }
            }
        }

        Kirigami.FormEntry {
            title: i18nc("@label", "sRGB color intensity:")
            visible: root.hdrActive || (element.colorProfileSource != KScreen.Output.ColorProfileSource.sRGB)
            contentItem: RowLayout {
                Layout.fillWidth: true
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                spacing: Kirigami.Units.smallSpacing

                Kirigami.FormData.buddyFor: sdrGamutSlider

                QQC2.Slider {
                    id: sdrGamutSlider
                    Kirigami.StyleHints.tickMarkStepSize: stepSize
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.sliderWidth
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

                    Layout.maximumWidth: root.maxSpinboxWidth

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
            }
            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "Increases the intensity of sRGB content on the screen.")
            }
        }

        Kirigami.FormEntry {
            title: i18nc("@label", "Adaptive Backlight Modulation:")
            visible: (element.capabilities & KScreen.Output.Capability.AbmLevel)
                  && element.colorPowerPreference == KScreen.Output.ColorPowerTradeoff.PreferEfficiency
            contentItem: RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Kirigami.FormData.buddyFor: abmLevelSlider

                QQC2.Slider {
                    id: abmLevelSlider
                    Kirigami.StyleHints.tickMarkStepSize: stepSize
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.sliderWidth
                    from: 0
                    to: 4
                    stepSize: 1
                    live: true
                    value: element.abmLevel
                    onMoved: element.abmLevel = value
                }
                QQC2.SpinBox {
                    from: 0
                    to: 4
                    stepSize: 1
                    value: element.abmLevel
                    onValueModified: element.abmLevel = value
                }
            }
            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "Adaptive backlight modulation reduces power use at the cost of color accuracy")
            }
        }

        Kirigami.FormEntry {
            id: ddcCiAllowedEntry
            title: i18nc("@label", "Brightness:")
            visible: (element.capabilities & KScreen.Output.Capability.DdcCi) && !(root.hdrAvailable && element.hdr)
            contentItem: QQC2.CheckBox {
                id: ddcCiAllowedCheckbox
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                text: i18nc("@option:check", "Control hardware brightness with DDC/CI")
                checked: element.ddcCiAllowed
                onToggled: element.ddcCiAllowed = checked
            }

            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "DDC/CI is a feature supported by many monitors. Plasma can use it to adjust screen brightness with desktop controls, as if using the monitor's own hardware buttons and OSD menu.")
            }
        }

        Kirigami.FormEntry {
            id: brightnessEntry
            visible: root.hdrActive || (element.capabilities & KScreen.Output.Capability.BrightnessControl)
            contentItem: RowLayout {
                id: brightnessRow
                Layout.fillWidth: true
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                spacing: Kirigami.Units.smallSpacing

                Kirigami.FormData.label: ddcCiAllowedEntry.visible ? "" : ddcCiAllowedEntry.title
                Kirigami.FormData.buddyFor: brightnessSlider

                QQC2.Slider {
                    id: brightnessSlider
                    Kirigami.StyleHints.tickMarkStepSize: stepSize
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.sliderWidth
                    from: 0
                    to: 100
                    stepSize: 5
                    live: true
                    value: Math.round(element.brightness * 100.0)
                    onMoved: element.brightness = value / 100.0
                }
                QQC2.SpinBox {
                    Layout.maximumWidth: root.maxSpinboxWidth

                    from: 0
                    to: 100
                    stepSize: 5
                    value: Math.round(element.brightness * 100.0)
                    onValueModified: element.brightness = value / 100.0
                    textFromValue: (value, locale) => i18nc("Brightness expressed in percentage form", "%1%", value)
                    valueFromText: (text, locale) => Number.fromLocaleString(locale, text.replace("%", ""))
                }
            }
        }

        Kirigami.FormEntry {
            visible: brightnessEntry.visible && (element.capabilities & KScreen.Output.Capability.AutomaticBrightness)
            contentItem: QQC2.CheckBox {
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                text: i18n("Automatically adapt to environment")
                checked: element.automaticBrightness
                onToggled: element.automaticBrightness = !element.automaticBrightness
            }
            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "Automatically adjust the screen’s brightness based on the lighting level of the environment, as detected by the device’s ambient light sensor. If you adjust the brightness manually, those adjustments will be remembered and taken into account.")
            }
        }

        /* Sharpness Slider and Spinbox */
        Kirigami.FormEntry {
            title: i18nc("@label", "Sharpness:")
            visible: element.capabilities & KScreen.Output.Capability.SharpnessControl
            contentItem: RowLayout {
                Layout.fillWidth: true
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                spacing: Kirigami.Units.smallSpacing

                Kirigami.FormData.buddyFor: sharpnessSlider

                QQC2.Slider {
                    id: sharpnessSlider
                    Kirigami.StyleHints.tickMarkStepSize: stepSize
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.sliderWidth
                    from: 0
                    to: 100
                    stepSize: 5
                    live: true
                    value: element.sharpness * 100.0
                    onMoved: element.sharpness = value / 100.0
                }
                QQC2.SpinBox {
                    from: 0
                    to: 100
                    stepSize: 5
                    value: element.sharpness * 100.0
                    onValueModified: element.sharpness = value / 100.0
                }
            }
        }

        Kirigami.FormEntry {
            title: i18nc("@label", "Extended Dynamic Range:")
            visible: !root.hdrAvailable && (element.capabilities & KScreen.Output.Capability.ExtendedDynamicRange)
            contentItem: QQC2.CheckBox {
                id: edrCheckbox
                // Set the same limit as the device ComboBox
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                text: i18nc("@option:check", "Enable EDR")
                checked: element.edrPolicy == KScreen.Output.EdrPolicy.Always
                onToggled: element.edrPolicy = (checked ? KScreen.Output.EdrPolicy.Always : KScreen.Output.EdrPolicy.Never)
            }

            trailingItems: Kirigami.ContextualHelpButton {
                toolTipText: xi18nc("@info:tooltip", "EDR allows viewing HDR content on SDR displays by dynamically adjusting the backlight.<nl/><nl/>Note that this increases battery usage while viewing HDR content.")
            }
        }

        Kirigami.FormEntry {
            title: i18n("Replica of:")
            visible: contentItem.count > 0
            contentItem: QQC2.ComboBox {
                Layout.minimumWidth: root.comboboxWidth
                Layout.maximumWidth: Kirigami.Units.gridUnit * 14
                model: element.replicationSourceModelWithNumbers
                textRole: "display"

                onModelChanged: enabled = (count > 1);
                onCountChanged: enabled = (count > 1);

                Component.onCompleted: currentIndex = element.replicationSourceIndex;
                onActivated: element.replicationSourceIndex = currentIndex;

                delegate: QQC2.ItemDelegate {
                    width: parent.width
                    contentItem: RowLayout {
                        spacing: Kirigami.Units.largeSpacing
                        OutputNumberBadge {
                            number: modelData.number
                        }

                        QQC2.Label {
                            text: modelData.display
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    highlighted: index === parent.currentIndex
                }
            }
        }
    }
}
