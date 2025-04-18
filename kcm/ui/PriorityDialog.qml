/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2025 Oliver Beard <olib141@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kitemmodels

import org.kde.private.kcm.kscreen 1.0 as KScreen

Kirigami.Dialog {
    id: reorderDialog

    title: i18nc("@title:window", "Change Priorities")
    showCloseButton: true
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
    padding: 0

    onAccepted: {
        for (let i = 0; i < enabledOutputsModel.count; i++) {
            const display = enabledOutputsModel.get(i);
            const index = kcm.outputModel.index(display.modelIndex.row, 0);
            kcm.outputModel.setData(index, display.priority, KScreen.OutputModel.PriorityRole);
        }
    }

    ListModel {
        id: enabledOutputsModel

        Component.onCompleted: {
            let displays = new Array(kcm.outputModel.rowCount())

            for (let i = 0; i < kcm.outputModel.rowCount(); ++i) {
                const index = kcm.outputModel.index(i, 0);
                const enabled = kcm.outputModel.data(index, KScreen.OutputModel.EnabledRole);
                const name = kcm.outputModel.data(index, Qt.DisplayRole);
                const priority = kcm.outputModel.data(index, KScreen.OutputModel.PriorityRole);

                // displays[priority - 1] - auto sort
                displays[priority - 1] = {"name": name, "priority": priority, "modelIndex": index, "enabled": enabled};
            }

            for (let i = 0; i < displays.length; ++i) {
                // Add only enabled displays
                if (displays[i].enabled) {
                    append(displays[i])
                }
            }
        }
    }

    contentItem: ListView {
        id: reorderView

        implicitWidth: Math.min(root.width * 0.75, Kirigami.Units.gridUnit * 32)
        implicitHeight: contentHeight

        reuseItems: false
        model: enabledOutputsModel

        Transition {
            id: transition
            NumberAnimation { properties: "y"; duration: Kirigami.Units.longDuration }
        }

        move: transition
        moveDisplaced: transition

        delegate: Item {
            id: itemDelegate

            implicitWidth: ListView.view.width
            implicitHeight: delegate.height

            required property string name
            required property int priority

            readonly property var view: ListView.view
            required property int index

            function move(oldIndex, newIndex): void {
                enabledOutputsModel.get(oldIndex).priority = newIndex + 1;
                enabledOutputsModel.get(newIndex).priority = oldIndex + 1;
                enabledOutputsModel.move(oldIndex, newIndex, 1);
            }

            Kirigami.SwipeListItem {
                id: delegate
                implicitWidth: itemDelegate.width

                // There's no need for a list item to ever be selected
                down: false
                highlighted: false

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.ListItemDragHandle {
                        listItem: delegate
                        listView: itemDelegate.view
                        onMoveRequested: (oldIndex, newIndex) => itemDelegate.move(oldIndex, newIndex)
                        visible: itemDelegate.view.count > 1
                    }

                    Kirigami.TitleSubtitle {
                        title: itemDelegate.name
                        subtitle: (itemDelegate.index === 0) ? i18n("Primary") : ""
                        font.bold: itemDelegate.index === 0
                        reserveSpaceForSubtitle: true
                        Layout.fillWidth: true
                    }
                }

                actions: [
                    Kirigami.Action {
                        icon.name: "arrow-up"
                        text: i18n("Raise priority")
                        enabled: itemDelegate.index > 0
                        onTriggered: {
                            if (enabled) {
                                itemDelegate.move(itemDelegate.index, itemDelegate.index - 1);
                            }
                        }
                    },
                    Kirigami.Action {
                        icon.name: "arrow-down"
                        text: i18n("Lower priority")
                        enabled: itemDelegate.index < reorderView.count - 1
                        onTriggered: {
                            if (enabled) {
                                itemDelegate.move(itemDelegate.index, itemDelegate.index + 1);
                            }
                        }
                    }
                ]
            }
        }
    }
}
