/*
    SPDX-FileCopyrightText: 2026 Xaver Hugl <xaver.hugl@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

import org.kde.private.kcm.kscreen as KScreen

RowLayout {
    id: selector

    required property var iccProfilePath
    signal pathChanged()

    spacing: Kirigami.Units.smallSpacing

    Kirigami.ActionTextField {
        id: iccProfileField
        onTextChanged: {
            iccProfilePath = text
            selector.pathChanged();
        }
        onTextEdited: {
            iccProfilePath = text
            selector.pathChanged();
        }
        placeholderText: i18nc("@info:placeholder", "Enter ICC profile path…")

        rightActions: Kirigami.Action {
            icon.name: "edit-clear-symbolic"
            visible: iccProfileField.text !== ""
            onTriggered: {
                iccProfileField.text = ""
            }
        }

        Component.onCompleted: text = iccProfilePath;
    }

    QQC2.Button {
        icon.name: "document-open-symbolic"
        text: i18nc("@action:button", "Select ICC profile…")
        display: QQC2.AbstractButton.IconOnly
        onClicked: fileDialogComponent.incubateObject(root);

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
}
