/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels

import org.kde.kcmutils as KCM

ColumnLayout {
    id: root

    property KSortFilterProxyModel enabledOutputs
    property int selectedOutput

    signal reorder()

    Kirigami.FormAlignmentGroup {
        id: alignmentGroup
    }

    StackLayout {
        id: panelView
        currentIndex: root.selectedOutput

        Layout.fillWidth: true

        Repeater {
            id: panelsRepeater
            model: kcm.outputModel
            OutputPanel {
                Kirigami.FormAlignmentGroup.group: alignmentGroup
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

    Kirigami.Separator {
        Layout.alignment: Qt.AlignCenter
        implicitWidth: globalSettingsGroup.width
    }

    Kirigami.Form {
        id: globalSettingsForm
        Kirigami.FormAlignmentGroup.group: alignmentGroup
        Layout.fillWidth: true
        visible: panelsRepeater.count > 0

        QQC2.ButtonGroup {
            id: x11AppsScaling
            onClicked: kcm.xwaylandClientsScale = (button === x11ScalingApps)
        }

        Kirigami.FormGroup {
            id: globalSettingsGroup

            Kirigami.FormEntry {
                title: i18n("Legacy applications (X11):")
                visible: kcm.xwaylandClientsScaleSupported
                contentItem: QQC2.RadioButton {
                    id: x11ScalingApps
                    text: i18nc("The apps themselves should scale to fit the displays", "Apply scaling themselves")
                    checked: kcm.xwaylandClientsScale
                    QQC2.ButtonGroup.group: x11AppsScaling
                }
                trailingItems: Kirigami.ContextualHelpButton {
                    toolTipText: i18n("Legacy applications that support scaling will use it and look crisp, however those that don’t will not be scaled at all.")
                }
            }

            Kirigami.FormEntry {
                visible: kcm.xwaylandClientsScaleSupported
                contentItem: QQC2.RadioButton {
                    text: i18nc("The system will perform the x11 apps scaling", "Scaled by the system")
                    checked: !kcm.xwaylandClientsScale
                    QQC2.ButtonGroup.group: x11AppsScaling
                }
                trailingItems: Kirigami.ContextualHelpButton {
                    toolTipText: i18n("All legacy applications will be scaled by the system to the correct size, however they will always look slightly blurry.")
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@label", "Screen tearing:")
                visible: kcm.tearingSupported
                contentItem: QQC2.CheckBox {
                    text: i18nc("@option:check The thing being allowed in fullscreen windows is screen tearing", "Allow in fullscreen windows")
                    checked: kcm.tearingAllowed
                    onToggled: kcm.tearingAllowed = checked
                }
                trailingItems: Kirigami.ContextualHelpButton {
                    toolTipText: i18nc("@info:tooltip", "Screen tearing reduces latency with most displays. Note that not all graphics drivers support this setting.")
                }
            }

            Item {}
        }
    }
}
