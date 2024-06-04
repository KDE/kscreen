/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts

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
            when: Plasmoid.connectedOutputCount < 2

            PropertyChanges {
                target: screenLayoutRow
                enabled: false
            }
            PropertyChanges {
                target: noScreenLabel
                visible: true
            }
        }
    ]

    Kirigami.Heading {
        Layout.fillWidth: true
        level: 3
        text: i18n("Screen Layout")
    }

    // Screen layout selector section
    Row {
        id: screenLayoutRow
        readonly property int buttonSize: Math.floor((width - spacing * (screenLayoutRepeater.count - 1)) / screenLayoutRepeater.count)
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            id: screenLayoutRepeater
            model: root.screenLayouts

            PlasmaComponents3.Button {
                id: screenLayoutDelegate

                width: screenLayoutRow.buttonSize
                height: screenLayoutRow.buttonSize

                onClicked: Plasmoid.applyLayoutPreset(modelData.action)

                Accessible.name: modelData.label
                PlasmaComponents3.ToolTip { text: modelData.label }

                // HACK otherwise the icon won't expand to full button size
                Kirigami.Icon {
                    anchors.centerIn: parent
                    width: height
                    // FIXME use proper FrameSvg margins and stuff
                    height: parent.height - Kirigami.Units.smallSpacing
                    source: modelData.iconName
                    active: screenLayoutDelegate.hovered
                }
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
