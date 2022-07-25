/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.9
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.3 as Controls
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.4 as Kirigami

import org.kde.kcm 1.6 as KCM
import org.kde.private.kcm.kscreen 1.0 as KScreen

ColumnLayout {
    Kirigami.FormLayout {
        twinFormLayouts: globalSettingsLayout
        visible: kcm.outputModel && kcm.outputModel.rowCount() > 1

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Device:")
            Layout.maximumWidth: Kirigami.Units.gridUnit * 16
            model: kcm.outputModel
            textRole: "display"
            currentIndex: root.selectedOutput
            onActivated: {
                root.selectedOutput = index
                currentIndex = Qt.binding(function() {
                    return root.selectedOutput;
                });
            }
        }
    }

    StackLayout {
        id: panelView
        currentIndex: root.selectedOutput

        onCurrentIndexChanged: root.selectedOutput =
                               Qt.binding(function() { return currentIndex; });

        Layout.fillWidth: true

        Repeater {
            model: kcm.outputModel
            OutputPanel {}
        }
    }

    Kirigami.FormLayout {
        id: globalSettingsLayout
        Layout.fillWidth: true

        Kirigami.Separator {
            Layout.fillWidth: true
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Layout.fillWidth: true
            Kirigami.FormData.label: i18n("Global scale:")

            visible: !kcm.perOutputScaling

            Controls.Slider {
                id: globalScaleSlider

                Layout.fillWidth: true
                from: 1
                to: 3
                stepSize: 0.25
                live: true
                value: kcm.globalScale
                onMoved: kcm.globalScale = value;
            }
            Controls.SpinBox {
                id: spinbox
                Layout.minimumWidth: Kirigami.Units.gridUnit * 6

                // Because QQC2 SpinBox doesn't natively support decimal step
                // sizes: https://bugreports.qt.io/browse/QTBUG-67349
                property real factor: 16.0
                property real realValue: value / factor

                from : 1.0 * factor
                to : 3.0 * factor
                // On X11 We set the increment to this weird value to compensate
                // for inherent difficulties with floating-point math and this
                // Qt bug: https://bugreports.qt.io/browse/QTBUG-66036
                stepSize: 0.0625 * factor
                value: kcm.globalScale * factor
                validator: DoubleValidator {
                    bottom: Math.min(spinbox.from, spinbox.to)*spinbox.factor
                    top:  Math.max(spinbox.from, spinbox.to)*spinbox.factor
                }
                textFromValue: function(value, locale) {
                    return i18nc("Global scale factor expressed in percentage form", "%1%", parseFloat(value * 1.0 / factor * 100.0));
                }
                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text.replace("%", "")) * factor / 100.0
                }
                onValueModified: {
                    kcm.globalScale = realValue;
                    if (kcm.globalScale % 0.25) {
                        weirdScaleFactorMsg.visible = true;
                    } else {
                        weirdScaleFactorMsg.visible = false;
                    }
                }
            }
        }

        Controls.ButtonGroup {
            id: x11AppsScaling
            onClicked: kcm.xwaylandClientsScale = (button === x11ScalingApps)
        }

        RowLayout {
            visible: kcm.xwaylandClientsScaleSupported

            Kirigami.FormData.label: i18n("Legacy Applications (X11):")
            spacing: Kirigami.Units.smallSpacing

            Controls.RadioButton {
                Kirigami.FormData.label: i18n("Legacy Applications (X11):")
                text: i18nc("The system will perform the x11 apps scaling", "Scaled by the system")
                checked: !kcm.xwaylandClientsScale
                Controls.ButtonGroup.group: x11AppsScaling
            }
            KCM.ContextualHelpButton {
                toolTipText: i18n("All legacy applications will be scaled by the system to the correct size, however they will always look slightly blurry.")
            }
        }

        RowLayout {
            visible: kcm.xwaylandClientsScaleSupported

            spacing: Kirigami.Units.smallSpacing

            Controls.RadioButton {
                id: x11ScalingApps
                text: i18nc("The apps themselves should scale to fit the displays", "Apply scaling themselves")
                checked: kcm.xwaylandClientsScale
                Controls.ButtonGroup.group: x11AppsScaling
            }
            KCM.ContextualHelpButton {
                toolTipText: i18n("Legacy applications that support scaling will use it and look crisp, however those that don't will not be scaled at all.")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
            visible: kcm.xwaylandClientsScaleSupported
        }

        Kirigami.InlineMessage {
            id: weirdScaleFactorMsg
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: i18n("The global scale factor is limited to multiples of 6.25% to minimize visual glitches in applications using the X11 windowing system.")
            visible: false
            showCloseButton: true
        }

        Controls.ButtonGroup {
            buttons: retentionSelector.children
        }

        ColumnLayout {
            id: retentionSelector

            Kirigami.FormData.label: i18n("Save displays' properties:")
            Kirigami.FormData.buddyFor: globalRetentionRadio
            spacing: Kirigami.Units.smallSpacing

            Controls.RadioButton {
                id: globalRetentionRadio
                text: i18n("For any display arrangement")
                checked: !individualRetentionRadio.checked
                onToggled: kcm.outputRetention = KScreen.Control.Global
            }

            Controls.RadioButton {
                id: individualRetentionRadio
                text: i18n("For only this specific display arrangement")
                checked: kcm.outputRetention === KScreen.Control.Individual
                onToggled: kcm.outputRetention = KScreen.Control.Individual
            }
        }
    }
}
