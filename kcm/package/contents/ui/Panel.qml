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
    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        spacing: 0
        visible: kcm.outputModel && kcm.outputModel.rowCount() > 1

        Kirigami.Heading {
            horizontalAlignment: Text.AlignHCenter
            level: 2
            // FIXME i18n change text in master
            text: i18n("Settings for %1", " ")
        }

        Controls.ComboBox {
            model: kcm.outputModel
            textRole: "display"
            currentIndex: root.selectedOutput
            onActivated: {
                root.selectedOutput = index
                currentIndex = Qt.binding(function() {
                    return root.selectedOutput;
                });
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
        visible: count > 1

        count: panelView.count
        currentIndex: root.selectedOutput
        interactive: true
        onCurrentIndexChanged: root.selectedOutput = currentIndex
    }

    Kirigami.FormLayout {
        id: globalSettingsLayout
        Layout.fillWidth: true

        Kirigami.Separator {
            Layout.fillWidth: true
            Kirigami.FormData.isSection: true
        }

        RowLayout {
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
                text: i18nc("Scale factor (e.g. 1.0x, 1.5x, 2.0x)","%1x", globalScaleSlider.value.toLocaleString(Qt.locale(), "f", 1))
            }
        }

        Controls.ButtonGroup {
            buttons: retentionSelector.children
        }

        ColumnLayout {
            id: retentionSelector

            Kirigami.FormData.label: i18n("Save displays' properties:")
            Kirigami.FormData.buddyFor: globalRetentionRadio
            spacing: Kirigami.Units.smallSpacing

            Controls.RadioButton {
                id: globalRetentionRadio
                text: i18n("For any display arrangement")
                checked: !individualRetentionRadio.checked
                onClicked: kcm.outputRetention = KScreen.Control.Global
            }

            Controls.RadioButton {
                id: individualRetentionRadio
                text: i18n("For only this specific display arrangement")
                checked: kcm.outputRetention === KScreen.Control.Individual
                onClicked: kcm.outputRetention = KScreen.Control.Individual
            }
        }
    }
}
