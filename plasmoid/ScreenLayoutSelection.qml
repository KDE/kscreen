/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

ColumnLayout {
    id: root

    // Screen layouts model.
    //
    // type: [{
    //  iconName: string,
    //  label: string,
    //  action: enum<OsdAction::Action>,
    // }]
    property var screenLayouts

    spacing: Kirigami.Units.smallSpacing * 2

    states: [
        State {
            // only makes sense to offer screen layout setup if there's more than one screen connected
            when: Plasmoid.connectedOutputCount < 2 // qmllint disable missing-property

            PropertyChanges {
                screenLayoutRow.enabled: false
            }
            PropertyChanges {
                noScreenLabel.visible: true
            }
        }
    ]

    Kirigami.Heading {
        Layout.fillWidth: true
        level: 3
        text: i18n("Screen Layout")
    }

    // Screen layout selector section
    RowLayout {
        id: screenLayoutRow

        // The buttons are square, so this doubles as their height. Rounding keeps
        // it an integer; uniformCellSizes takes care of the sub-pixel remainder of
        // the widths so that the row always ends flush with its right edge.
        readonly property int buttonSize: screenLayoutRepeater.count > 0
            ? Math.round((width - spacing * (screenLayoutRepeater.count - 1)) / screenLayoutRepeater.count)
            : 0

        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing
        // Give every button the same width instead of letting the last one keep
        // whatever the division left over
        uniformCellSizes: true

        Repeater {
            id: screenLayoutRepeater
            model: root.screenLayouts

            PlasmaComponents3.Button {
                id: screenLayoutDelegate

                required property /*KScreen.OsdAction*/var modelData

                Layout.fillWidth: true
                // Natural size of a button; the row stretches them from here
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: screenLayoutRow.buttonSize

                display: T.Button.IconOnly
                icon.name: modelData.iconName
                icon.width: availableWidth
                icon.height: availableHeight
                text: modelData.label

                onClicked: Plasmoid.applyLayoutPreset(modelData.action) // qmllint disable missing-property

                Accessible.name: text
                PlasmaComponents3.ToolTip { text: screenLayoutDelegate.text }
            }
        }
    }

    PlasmaExtras.DescriptiveLabel {
        id: noScreenLabel
        Layout.fillWidth: true
        Layout.maximumWidth: Math.min(Kirigami.Units.gridUnit * 20, implicitWidth)
        wrapMode: Text.Wrap
        text: i18n("You can only apply a different screen layout when there is more than one display device plugged in.")
        font: Kirigami.Theme.smallFont
        visible: false
    }
}
