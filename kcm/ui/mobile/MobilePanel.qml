/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.kcmutils as KCM

import "../"

ColumnLayout {
    id: root
    spacing: 0

    property KSortFilterProxyModel enabledOutputs
    property int selectedOutput

    signal reorder()

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.gridUnit
        visible: kcm.multipleScreensAvailable

        FormCard.FormComboBoxDelegate {
            text: i18n("Device")

            model: kcm.outputModel
            textRole: "display"
            Component.onCompleted: currentIndex = Qt.binding(() => root.selectedOutput);
            onActivated: index => {
                root.selectedOutput = index;
                currentIndex = Qt.binding(() => root.selectedOutput);
            }
            onCountChanged: {
                // Temporary model resets can reset currentIndex to -1 and break the binding.
                if (currentIndex == -1 && root.selectedOutput < count) {
                    currentIndex = Qt.binding(() => root.selectedOutput);
                }
            }
        }
    }

    StackLayout {
        id: panelView
        currentIndex: root.selectedOutput

        Layout.fillWidth: true

        Repeater {
            model: kcm.outputModel
            MobileOutputPanel {
                enabledOutputs: root.enabledOutputs
                onReorder: root.reorder()
            }
        }
    }

    FormCard.FormCard {
        visible: !kcm.perOutputScaling

        FormCard.FormComboBoxDelegate {
            text: i18n("Global scale")

            model: [100, 125, 150, 175, 200, 225, 250, 275, 300]
            currentIndex: indexOfValue(kcm.globalScale * 100)
            onCurrentValueChanged: {
                kcm.globalScale = value / 100;
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

    FormCard.FormHeader {
        visible: kcm.xwaylandClientsScaleSupported
        title: i18n("Legacy applications (X11)")
    }

    FormCard.FormCard {
        visible: kcm.xwaylandClientsScaleSupported

        FormCard.FormRadioDelegate {
            id: x11ScalingApps
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            text: i18nc("The apps themselves should scale to fit the displays", "Apply scaling themselves")
            checked: kcm.xwaylandClientsScale
            QQC2.ButtonGroup.group: x11AppsScaling

            trailing: Kirigami.ContextualHelpButton {
                toolTipText: i18n("Legacy applications that support scaling will use it and look crisp, however those that don't will not be scaled at all.")
            }
        }

        FormCard.FormRadioDelegate {
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            text: i18nc("The system will perform the x11 apps scaling", "Scaled by the system")
            checked: !kcm.xwaylandClientsScale
            QQC2.ButtonGroup.group: x11AppsScaling

            trailing: Kirigami.ContextualHelpButton {
                toolTipText: i18n("All legacy applications will be scaled by the system to the correct size, however they will always look slightly blurry.")
            }
        }
    }

    FormCard.FormHeader {
        visible: kcm.tearingSupported
        title: i18n("Screen tearing")
    }

    FormCard.FormCard {
        visible: kcm.tearingSupported

        FormCard.FormSwitchDelegate {
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            text: i18nc("@option:check The thing being allowed in fullscreen windows is screen tearing", "Allow in fullscreen windows")
            checked: kcm.tearingAllowed
            onToggled: kcm.tearingAllowed = checked

            trailing: Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "Screen tearing reduces latency with most displays. Note that not all graphics drivers support this setting.")
            }
        }
    }

    Kirigami.InlineMessage {
        id: weirdScaleFactorMsg
        Layout.topMargin: Kirigami.Units.largeSpacing

        Kirigami.FormData.isSection: true
        Layout.fillWidth: true
        type: Kirigami.MessageType.Information
        text: i18n("The global scale factor is limited to multiples of 6.25% to minimize visual glitches in applications using the X11 windowing system.")
        visible: false
        showCloseButton: true
    }
}
