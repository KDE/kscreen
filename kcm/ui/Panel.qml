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

        QQC2.ButtonGroup {
            id: x11AppsScaling
            onClicked: kcm.xwaylandClientsScale = (checkedButton === x11ScalingApps)
        }

        RowLayout {
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
        }
    }
}
