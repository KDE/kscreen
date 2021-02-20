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
import QtQuick.Layouts 1.10

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PlasmaComponents

import org.kde.KScreen 1.0

PlasmaCore.Dialog {
    id: root
    location: PlasmaCore.Types.Floating
    type: PlasmaCore.Dialog.Normal
    property string infoText
    property var screenGeometry

    onScreenGeometryChanged: {
        root.x = screenGeometry.x + (screenGeometry.width - mainItem.width) / 2
        root.y = screenGeometry.y + (screenGeometry.height - mainItem.height) / 2
    }

    signal clicked(int actionId)

    mainItem: ColumnLayout {
        RowLayout {
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
                    })
                }
                delegate: PlasmaComponents.Button {
                    property int actionId: modelData.action

                    Accessible.name: modelData.label

                    icon.name: modelData.iconSource
                    icon.height: PlasmaCore.Units.gridUnit * 8
                    icon.width: PlasmaCore.Units.gridUnit * 8

                    onClicked: root.clicked(actionId)
                    onHoveredChanged: {
                        actionRepeater.currentIndex = index
                    }

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
            text: root.infoText
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
            wrapMode: Text.WordWrap

            Layout.fillWidth: true
            Layout.margins: Math.floor(PlasmaCore.Units.smallSpacing / 2)
        }

        function move(delta) {
            actionRepeater.currentIndex = ((actionRepeater.currentIndex + delta) + actionRepeater.count) % actionRepeater.count
        }

        Keys.onPressed: {
            switch (event.key) {
                case Qt.Key_Return:
                case Qt.Key_Enter:
                    clicked(actionRepeater.itemAt(actionRepeater.currentIndex).actionId)
                    break
                case Qt.Key_Right:
                    move(1)
                    break
                case Qt.Key_Left:
                    move(-1)
                    break
                case Qt.Key_Escape:
                    clicked(OsdAction.NoAction)
                    break
            }
        }
    }
}

