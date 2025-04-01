/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.kcmutils as KCM

ColumnLayout {
    id: root

    property KSortFilterProxyModel enabledOutputs
    property int selectedOutput

    signal reorder()

    StackLayout {
        id: panelView
        currentIndex: root.selectedOutput

        Layout.fillWidth: true

        Repeater {
            model: kcm.outputModel
            OutputPanel {
                twinFormLayouts: globalSettingsLayout
                enabledOutputs: root.enabledOutputs
                onReorder: root.reorder()
            }

            // "Since 6.5, inserting/removing a new Item at an index less than or equal to the current
            // index will increment/decrement the current index, but keep the current Item."
            // This causes BUG: 490586 and the following works around it:
            onItemAdded: panelView.currentIndex = Qt.binding(() => root.selectedOutput)
            onItemRemoved: panelView.currentIndex = Qt.binding(() => root.selectedOutput)
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

            QQC2.Slider {
                id: globalScaleSlider

                Accessible.description: i18nc("@info accessible description of slider value", "in percent of regular scale")

                Layout.fillWidth: true
                from: 100
                to: 300
                stepSize: 25
                live: true
                value: kcm.globalScale * 100
                onMoved: kcm.globalScale = value / 100;
            }
            QQC2.SpinBox {
                id: spinbox
                Layout.minimumWidth: Kirigami.Units.gridUnit * 6

                // Because QQC2 SpinBox doesn't natively support decimal step
                // sizes: https://bugreports.qt.io/browse/QTBUG-67349
                readonly property real factor: 16.0
                readonly property real realValue: value / factor

                from: 1.0 * factor
                to: 3.0 * factor
                // On X11 We set the increment to this weird value to compensate
                // for inherent difficulties with floating-point math and this
                // Qt bug: https://bugreports.qt.io/browse/QTBUG-66036
                stepSize: 1
                value: kcm.globalScale * factor
                validator: DoubleValidator {
                    bottom: Math.min(spinbox.from, spinbox.to) * spinbox.factor
                    top:  Math.max(spinbox.from, spinbox.to) * spinbox.factor
                }
                textFromValue: (value, locale) =>
                    i18nc("Global scale factor expressed in percentage form", "%1%",
                        parseFloat(value * 1.0 / factor * 100.0))
                valueFromText: (text, locale) =>
                    Number.fromLocaleString(locale, text.replace("%", "")) * factor / 100.0

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

        QQC2.ButtonGroup {
            id: x11AppsScaling
            onClicked: kcm.xwaylandClientsScale = (button === x11ScalingApps)
        }

        RowLayout {
            visible: kcm.xwaylandClientsScaleSupported

            Kirigami.FormData.label: i18n("Legacy applications (X11):")
            spacing: Kirigami.Units.smallSpacing

            QQC2.RadioButton {
                id: x11ScalingApps
                text: i18nc("The apps themselves should scale to fit the displays", "Apply scaling themselves")
                checked: kcm.xwaylandClientsScale
                QQC2.ButtonGroup.group: x11AppsScaling
            }
            Kirigami.ContextualHelpButton {
                toolTipText: i18n("Legacy applications that support scaling will use it and look crisp, however those that don’t will not be scaled at all.")
            }
        }

        RowLayout {
            visible: kcm.xwaylandClientsScaleSupported

            spacing: Kirigami.Units.smallSpacing

            QQC2.RadioButton {
                Kirigami.FormData.label: i18n("Legacy applications (X11):")
                text: i18nc("The system will perform the x11 apps scaling", "Scaled by the system")
                checked: !kcm.xwaylandClientsScale
                QQC2.ButtonGroup.group: x11AppsScaling
            }
            Kirigami.ContextualHelpButton {
                toolTipText: i18n("All legacy applications will be scaled by the system to the correct size, however they will always look slightly blurry.")
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18nc("@label", "Screen tearing:")
            visible: kcm.tearingSupported
            QQC2.CheckBox {
                text: i18nc("@option:check The thing being allowed in fullscreen windows is screen tearing", "Allow in fullscreen windows")
                checked: kcm.tearingAllowed
                onToggled: kcm.tearingAllowed = checked
            }
            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "Screen tearing reduces latency with most displays. Note that not all graphics drivers support this setting.")
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
    }
}
