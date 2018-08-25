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

import org.kde.KScreen 1.0

PlasmaCore.Dialog {
    id: root
    location: PlasmaCore.Types.Floating
    type: PlasmaCore.Dialog.Normal
    property string infoText

    signal clicked(int actionId)

    mainItem: Item {
        height: Math.min(units.gridUnit * 15, Screen.desktopAvailableHeight / 5)
        width: buttonRow.width

        Row {
            id: buttonRow
            spacing: theme.defaultFont.pointSize

            height: parent.height - label.height - ((units.smallSpacing/2) * 3)
            width: (actionRepeater.count * height) + ((actionRepeater.count - 1) * buttonRow.spacing);

            Repeater {
                id: actionRepeater
                property int currentIndex: 0
                model: {
                    return OsdAction.actionOrder().map(function (layout) {
                        return {
                            iconSource: OsdAction.actionIconName(layout),
                            label: OsdAction.actionLabel(layout),
                            action: layout
                        }
                    });
                }
                delegate: PlasmaComponents.Button {
                    property var action: modelData.action
                    Accessible.name: modelData.label
                    PlasmaCore.IconItem {
                        source: modelData.iconSource
                        height: buttonRow.height - ((units.smallSpacing / 2) * 3)
                        width: height
                        anchors.centerIn: parent
                    }
                    height: parent.height
                    width: height

                    onHoveredChanged: {
                        actionRepeater.currentIndex = index
                    }

                    onClicked: root.clicked(action)
                    activeFocusOnTab: true

                    // use checked only indirectly, since its binding will break
                    property bool current: index == actionRepeater.currentIndex
                    onCurrentChanged: {
                        if (current) {
                            checked = true
                            root.infoText = modelData.label
                            forceActiveFocus()
                        } else {
                            checked = false
                        }
                    }
                    onActiveFocusChanged: {
                        if (activeFocus) {
                            actionRepeater.currentIndex = index
                        }
                    }
                }
            }
        }

        PlasmaExtras.Heading {
            id: label
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: Math.floor(units.smallSpacing / 2)
            }

            text: root.infoText
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideLeft
            minimumPointSize: theme.defaultFont.pointSize
            fontSizeMode: Text.HorizontalFit
        }

        Component.onCompleted: print("OsdSelector loaded...");

        function next() {
            var index = actionRepeater.currentIndex + 1
            if (index >= actionRepeater.count) {
                index = 0
            }
            actionRepeater.currentIndex = index
        }
        function previous() {
            var index = actionRepeater.currentIndex - 1
            if (index < 0) {
                index = actionRepeater.count - 1
            }
            actionRepeater.currentIndex = index
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                event.accepted = true
                clicked(buttonRow.children[actionRepeater.currentIndex].action)
                return
            }
            if (event.key === Qt.Key_Right) {
                event.accepted = true
                next()
                return
            }
            if (event.key === Qt.Key_Left) {
                event.accepted = true
                previous()
                return
            }
            if (event.key === Qt.Key_Escape) {
                event.accepted = true
                clicked(OsdAction.NoAction)
                return
            }
        }
    }
}

