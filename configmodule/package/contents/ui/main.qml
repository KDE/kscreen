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
import QtQuick.Controls 1.1 as Controls
import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrols 2.0
import org.kde.kscreen 2.0
import org.kde.kcm 1.0


ColumnLayout {
    id: root
    objectName: "root"

    property variant virtualScreen: null
    property KScreenOutput focusedOutput: null
    property bool perOutputScaling: false


    implicitHeight: units.gridUnit * 15
    implicitWidth: units.gridUnit * 30

    spacing: units.largeSpacing

    focus: true

    SystemPalette {
        id: palette;
    }

    OutputsView {
        Layout.fillWidth: true
        Layout.fillHeight: true

    }

    OutputSettings {
        id: outputSettings
    }

    Component.onCompleted: print("Completed main.qml")
}
