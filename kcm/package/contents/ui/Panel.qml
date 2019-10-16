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
        currentIndex: panelView.currentIndex
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
                stepSize: 0.25
                live: true
                value: kcm.globalScale
                onMoved: kcm.globalScale = value;
            }
            Controls.SpinBox {
                id: spinbox
                // Because QQC2 SpinBox doesn't natively support decimal step
                // sizes: https://bugreports.qt.io/browse/QTBUG-67349
                property real factor: 20.0
                property real realValue: value / factor

                from : 1.0 * factor
                to : 3.0 * factor
                stepSize: 0.05 * factor
                value: kcm.globalScale * factor
                validator: DoubleValidator {
                    bottom: Math.min(spinbox.from, spinbox.to)*spinbox.factor
                    top:  Math.max(spinbox.from, spinbox.to)*spinbox.factor
                }
                textFromValue: function(value, locale) {
                    return i18nc("Global scale factor expressed in percentage form", "%1%", parseFloat(value * 1.0 / factor * 100.0));
                }
                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text) * factor / 100.0
                }
                onValueModified: {
                    kcm.globalScale = realValue;
                    if (kcm.globalScale % 0.25) {
                        weirdScaleFactorMsg.visible = true;
                    } else {
                        weirdScaleFactorMsg.visible = false;
                    }
                }
            }
        }

        Kirigami.InlineMessage {
            id: weirdScaleFactorMsg
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            text: i18n("Scale factors that are not a multiple of 25% may cause visual glitches in applications. Consider setting the scale factor to a multiple of 25% and adjusting the font size instead.")
            visible: false
            showCloseButton: true
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
