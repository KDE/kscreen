/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.8
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

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

    spacing: PlasmaCore.Units.smallSpacing * 2

    states: [
        State {
            // only makes sense to offer screen layout setup if there's more than one screen connected
            when: Plasmoid.nativeInterface.connectedOutputCount < 2

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

    PlasmaExtras.Heading {
        Layout.fillWidth: true
        level: 3
        text: i18n("Screen Layout")
    }

    // Screen layout selector section
    Row {
        id: screenLayoutRow
        readonly property int buttonSize: Math.floor((width - spacing * (screenLayoutRepeater.count - 1)) / screenLayoutRepeater.count)
        Layout.fillWidth: true
        spacing: PlasmaCore.Units.smallSpacing

        Repeater {
            id: screenLayoutRepeater
            model: root.screenLayouts

            PlasmaComponents.Button {
                width: screenLayoutRow.buttonSize
                height: width
                tooltip: modelData.label
                Accessible.name: tooltip
                onClicked: Plasmoid.nativeInterface.applyLayoutPreset(modelData.action)

                // HACK otherwise the icon won't expand to full button size
                PlasmaCore.IconItem {
                    anchors.centerIn: parent
                    width: height
                    // FIXME use proper FrameSvg margins and stuff
                    height: parent.height - PlasmaCore.Units.smallSpacing
                    source: modelData.iconName
                    active: parent.hovered
                }
            }
        }
    }

    PlasmaExtras.DescriptiveLabel {
        id: noScreenLabel
        Layout.fillWidth: true
        Layout.maximumWidth: Math.min(PlasmaCore.Units.gridUnit * 20, implicitWidth)
        wrapMode: Text.Wrap
        text: i18n("You can only apply a different screen layout when there is more than one display device plugged in.")
        font.pointSize: theme.smallestFont.pointSize
        visible: false
    }
}
