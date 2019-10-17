/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 2.9
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.4 as Kirigami

import org.kde.kcm 1.2 as KCM

ColumnLayout {
    id: outputPanel
    property var element: model

    Kirigami.FormLayout {
        twinFormLayouts: globalSettingsLayout

        Controls.CheckBox {
           text: i18n("Enabled")
           checked: element.enabled
           onClicked: element.enabled = checked
           visible: kcm.outputModel.rowCount() > 1
        }

        Controls.CheckBox {
           text: i18n("Primary")
           checked: element.primary
           onClicked: element.primary = checked
           visible: kcm.primaryOutputSupported && kcm.outputModel.rowCount() > 1
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Resolution:")
            model: element.resolutions
            currentIndex: element.resolutionIndex !== undefined ?
                              element.resolutionIndex : -1
            onActivated: element.resolutionIndex = currentIndex
        }

        RowLayout {
            Layout.fillWidth: true

            visible: kcm.perOutputScaling
            Kirigami.FormData.label: i18n("Scale:")

            Controls.Slider {
                id: scaleSlider

                Layout.fillWidth: true
                from: 0.5
                to: 3
                stepSize: 0.1
                live: true
                value: element.scale
                onMoved: element.scale = value
            }
            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: i18nc("Scale factor (e.g. 1.0x, 1.5x, 2.0x)","%1x", scaleSlider.value.toLocaleString(Qt.locale(), "f", 1))
            }
        }

        Controls.ButtonGroup {
            buttons: orientation.children
        }

        RowLayout {
           id: orientation
           Kirigami.FormData.label: i18n("Orientation:")

           RotationButton {
               value: 0
           }
           RotationButton {
               value: 90
           }
           RotationButton {
               value: 180
           }
           RotationButton {
               value: 270
           }
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Refresh rate:")
            model: element.refreshRates
            currentIndex: element.refreshRateIndex ?
                              element.refreshRateIndex : 0
            onActivated: element.refreshRateIndex = currentIndex
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Replica of:")
            model: element.replicationSourceModel
            visible: kcm.outputReplicationSupported && kcm.outputModel && kcm.outputModel.rowCount() > 1

            onModelChanged: enabled = (count > 1);
            onCountChanged: enabled = (count > 1);

            currentIndex: element.replicationSourceIndex
            onActivated: element.replicationSourceIndex = currentIndex
        }
    }
}
