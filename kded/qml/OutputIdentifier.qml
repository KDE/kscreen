/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ColumnLayout {

    property QtObject rootItem

    property string outputName: rootItem ? rootItem.outputName : ""
    property string modeName: rootItem ? rootItem.modeName : ""

    PlasmaComponents.Label {
        id: displayName

        Layout.maximumWidth: Screen.width * 0.8
        Layout.maximumHeight: Screen.height * 0.8
        Layout.margins: units.largeSpacing
        Layout.bottomMargin: units.smallSpacing

        text: root.outputName
        font.pointSize: theme.defaultFont.pointSize * 3
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideLeft
    }

    PlasmaComponents.Label {
        id: modeLabel;

        Layout.fillWidth: true
        Layout.bottomMargin: units.largeSpacing

        text: root.modeName;
        horizontalAlignment: Text.AlignHCenter;
    }
}
