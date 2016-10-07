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

Item {
    Layout.fillWidth: true
    Layout.minimumHeight: childrenRect.height

    property alias value: internalSlider.value
    property alias minimumValue: internalSlider.minimumValue
    property alias maximumValue: internalSlider.maximumValue
    property alias stepSize: internalSlider.stepSize

    // FIXME: LTR awareness
    property alias minimumLabel: leftLabel.text
    property alias maximumLabel: rightLabel.text

    Slider {
        id: internalSlider
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        minimumValue: 0
        tickmarksEnabled: true
        stepSize: 1
    }
    Label {
        id: leftLabel
        font.pointSize: theme.smallestFont.pointSize
        opacity: 0.8
        anchors {
            left: parent.left
            top: internalSlider.bottom
        }
    }
    Label {
        id: rightLabel
        font.pointSize: theme.smallestFont.pointSize
        opacity: 0.8
        anchors {
            right: parent.right
            top: internalSlider.bottom
        }
    }
}
