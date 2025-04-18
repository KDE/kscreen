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

Kirigami.Dialog {
    id: reorderDialog

    title: i18nc("@title:window", "Change Priorities")
    showCloseButton: true
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
    padding: 0

    contentItem: ListView {
        id: reorderView

        implicitWidth: Math.min(root.width * 0.75, Kirigami.Units.gridUnit * 32)
        implicitHeight: contentHeight

        reuseItems: true
        model: KSortFilterProxyModel {
            id: enabledOutputsModel
            sourceModel: kcm.outputModel
            filterRoleName: "enabled"
            filterString: "true"
            sortRoleName: "priority"
            sortOrder: Qt.AscendingOrder
        }
        delegate: Kirigami.SwipeListItem {
            id: delegate

            property var output: model

            width: ListView.view.width

            background: null
            contentItem: KD.TitleSubtitle {
                title: delegate.output.display
                subtitle: (delegate.output.priority === 1) ? i18n("Primary") : ""
            }
            actions: [
                Kirigami.Action {
                    icon.name: "arrow-up"
                    text: i18n("Raise priority")
                    enabled: delegate.output.priority > 1
                    onTriggered: {
                        if (enabled) {
                            delegate.output.priority -= 1;
                        }
                    }
                },
                Kirigami.Action {
                    icon.name: "arrow-down"
                    text: i18n("Lower priority")
                    enabled: delegate.output.priority < reorderView.count
                    onTriggered: {
                        if (enabled) {
                            delegate.output.priority += 1;
                        }
                    }
                }
            ]
        }
    }
}
