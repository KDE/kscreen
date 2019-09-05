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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.4 as Kirigami
import org.kde.private.kcm.kscreen 1.0 as KScreen

ColumnLayout {
    Kirigami.FormLayout {
        twinFormLayouts: globalSettingsLayout
        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Output settings")

            Rectangle {
                anchors.fill: parent
                opacity: 0.5
            }
        }
    }

    Controls.SwipeView {
        id: panelView
        currentIndex: root.selectedOutput

        onCurrentIndexChanged: root.selectedOutput =
                               Qt.binding(function() { return currentIndex; });

        Layout.fillWidth: true

        Repeater {
            model: kcm.outputModel
            OutputPanel {}
        }
    }

    Controls.PageIndicator {
        id: indicator

        Layout.alignment: Qt.AlignHCenter
        opacity: count > 1 ? 1 : 0

        count: panelView.count
        currentIndex: panelView.currentIndex
        interactive: true
        onCurrentIndexChanged: root.selectedOutput = currentIndex
    }

    Kirigami.FormLayout {
        id: globalSettingsLayout
        Layout.fillWidth: true
        Layout.topMargin: 20
        Kirigami.Separator {
            Layout.fillWidth: true
            Kirigami.FormData.isSection: true
        }
        Item {
            Layout.fillWidth: true
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Arrangement settings")
        }

        ColumnLayout {
            Layout.fillWidth: true
            Kirigami.FormData.label: i18n("Global scale:")

            visible: !kcm.perOutputScaling

            Controls.Slider {
                id: globalScaleSlider

                Layout.fillWidth: true
                from: 1
                to: 3
                stepSize: 0.1
                live: true
                value: kcm.globalScale
                onMoved: kcm.globalScale = value
            }
            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: globalScaleSlider.value.toLocaleString(Qt.locale(), "f", 1)
            }
        }

        Controls.ButtonGroup {
            buttons: retentionSelector.children
        }

        ColumnLayout {
            id: retentionSelector

            Kirigami.FormData.label: i18n("Save values of an output:")
            Kirigami.FormData.buddyFor: globalRetentionRadio
            spacing: Kirigami.Units.smallSpacing

            Controls.RadioButton {
                id: globalRetentionRadio
                text: i18n("For this and other combination of outputs")
                checked: !individualRetentionRadio.checked
                onClicked: kcm.outputRetention = KScreen.Control.Global
            }

            Controls.RadioButton {
                id: individualRetentionRadio
                text: i18n("For this specific setup independently of others")
                checked: kcm.outputRetention === KScreen.Control.Individual
                onClicked: kcm.outputRetention = KScreen.Control.Individual
            }
        }
    }
}
