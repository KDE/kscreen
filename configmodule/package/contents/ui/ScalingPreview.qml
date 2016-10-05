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
import org.kde.kscreen 2.0

Rectangle {
    id: scalingPreview

    property real scalingFactor: 1.0

    color: palette.window
    opacity: 0
    visible: (opacity > 0);

    Behavior on opacity { NumberAnimation { duration: units.longDuration } }

    onScalingFactorChanged: {
        scalingPreview.opacity = 1;
        opacityTimer.running = true;
    }

    Timer {
        id: opacityTimer
        interval: 5000
        onTriggered: scalingPreview.opacity = 0
    }

    PlasmaExtras.Heading {
        id: sheading
        text: i18n("Scaling Preview")
        z: spi.z + 1
        anchors {
            top: parent.top
            right: parent.right
        }
    }
    ScalingPreviewItem {
        id: spi
        scalingFactor: scalingPreview.scalingFactor
        anchors {
            left: parent.left
            top: sheading.top
            right: parent.right
            bottom: parent.bottom
        }
    }
}
