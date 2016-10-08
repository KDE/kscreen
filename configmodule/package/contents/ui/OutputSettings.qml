/*
    Copyright 2016 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

import QtQuick 2.1
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrols 2.0
import org.kde.kscreen 2.0

GridLayout {

    property KScreenOutput output: kcm.modeSelector.output

    focus: true
    columns: 2
    //rowSpacing: units.smallSpacing

    Layout.maximumHeight: childrenRect.height

    onOutputChanged: {
        print("Output is now: " + output.id);
        var cmi = kcm.modeSelector.currentModeIndex();
        var maxi = Math.max(0, kcm.modeSelector.modeSizes.length - 1);

        // This bit is tricky, Slider fails to apply values above a maximumvalue
        // and also can't set a maximumValue above the current value, so make sure
        // we set these properties in the right order
        if (resolutionSlider.value > maxi) {
            resolutionSlider.value = cmi;
            resolutionSlider.maximumValue = maxi;
        } else {
            resolutionSlider.maximumValue = maxi;
            resolutionSlider.value = cmi;
        }
    }

    PlasmaExtras.Heading {
        id: outputLabel
        level: 2
        text: {
            if (output === null) return "null";
            if (output.type == KScreenOutput.Panel) {
                return i18n("Laptop Screen");
            }
            return output.edid.vendor + " " + output.edid.name + " (" + output.name + ")"
        }
        Layout.columnSpan: 2
    }

    Label {
        Layout.fillWidth: true
        Layout.columnSpan: 2
        visible: root.perOutputScaling
        text: i18nc("value label of scaling slider", "%1x", scalingSlider.value)
        horizontalAlignment: Text.AlignRight
    }

    Label {
        visible: root.perOutputScaling
        text: i18n("Scaling factor:")
        Layout.alignment: Qt.AlignTop | Qt.AlignRight
    }

    LabeledSlider {
        id: scalingSlider
        visible: root.perOutputScaling
        minimumValue: 1
        maximumValue: 3
        stepSize: .1
        minimumLabel: "1.0x"
        maximumLabel: "3.0x"
        onValueChanged: root.scalingPreview.scalingFactor = value
    }

    Label {
        Layout.fillWidth: true
        Layout.columnSpan: 2
        text: {
            if (true || qmlOutput) {
                kcm.modeSelector.selectedMode ? kcm.modeSelector.selectedMode.size.width + "x" + kcm.modeSelector.selectedMode.size.height + "@" + kcm.modeSelector.selectedMode.refreshRate.toFixed(2) : ""
            } else ""
        }
        horizontalAlignment: Text.AlignRight
    }

    Label {
        text: i18n("Resolution and refresh rate:")
        Layout.alignment: Qt.AlignTop | Qt.AlignRight
    }

    ColumnLayout {
        id: modecol
        property KScreenMode selectedMode: null
        spacing: units.smallSpacing

        LabeledSlider {
            id: resolutionSlider
            minimumLabel: kcm.modeSelector.modeLabelMin
            maximumLabel: kcm.modeSelector.modeLabelMax
            onValueChanged: {
                //print("res index setting to " + value)
                refreshCombo.currentIndex = 0;
                kcm.modeSelector.setSelectedResolutionIndex(value)
                refreshCombo.currentIndex = kcm.modeSelector.preferredRefreshIndexForSizeIndex(value);
            }
        }
        ComboBox {
            id: refreshCombo
            model: kcm.modeSelector.refreshRatesLabels
            onCurrentIndexChanged: {
                //print("Selecting refresh" + currentText);
                kcm.modeSelector.setSelectedRefreshRate(currentIndex)
            }
        }
    }
    Item {
        Layout.preferredHeight: units.largeSpacing
    }
}
