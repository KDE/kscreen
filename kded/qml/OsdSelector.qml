/*
 * Copyright 2017 Daniel Vrátil <dvratil@kde.org>
  *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.5
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root
    property QtObject rootItem

    signal clicked(string actionId)

    height: Math.min(units.gridUnit * 15, Screen.desktopAvailableHeight / 5)
    width: buttonRow.width

    PlasmaComponents.ButtonRow {
        id: buttonRow

        exclusive: false

        height: parent.height - label.height - ((units.smallSpacing/2) * 3)
        width: (actionRepeater.model.length * height) + ((actionRepeater.model.length - 1) * buttonRow.spacing);

        Repeater {
            id: actionRepeater
            model: [
                    {
                        iconSource: "osd-shutd-screen",
                        label: qsTr("Switch to external screen"),
                        action: "external-only"
                    },
                    {
                        iconSource: "osd-shutd-laptop",
                        label: qsTr("Switch to laptop screen"),
                        action: "internal-only"
                    },
                    {
                        iconSource: "osd-duplicate",
                        label: qsTr("Duplicate outputs"),
                        action: "clone"
                    },
                    {
                        iconSource: "osd-sbs-left",
                        label: qsTr("Extend to left"),
                        action: "extend-left"
                    },
                    {
                        iconSource: "osd-sbs-sright",
                        label: qsTr("Extend to right"),
                        action: "extend-right"
                    },
                    {
                        iconSource: "dialog-cancel",
                        label: qsTr("Do nothing"),
                        action: "cancel"
                    }
            ]
            delegate: PlasmaComponents.Button {
                PlasmaCore.IconItem {
                    source: modelData.iconSource
                    height: buttonRow.height - ((units.smallSpacing / 2) * 3)
                    width: height
                    anchors.centerIn: parent
                }
                height: parent.height
                width: height

                onHoveredChanged: rootItem.infoText = (hovered ? modelData.label : "")

                onClicked: root.clicked(modelData.action)
            }
        }
    }

    // TODO: keep? remove?
    PlasmaExtras.Heading {
        id: label
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: Math.floor(units.smallSpacing / 2)
        }

        text: rootItem.infoText
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideLeft
        minimumPointSize: theme.defaultFont.pointSize
        fontSizeMode: Text.HorizontalFit
    }

    Component.onCompleted: print("OsdSelector loaded...");
}

