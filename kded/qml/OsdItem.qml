/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Layouts 1.10

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    PlasmaCore.IconItem {
        source: rootItem.icon

        Layout.preferredHeight: PlasmaCore.Units.gridUnit * 10
        Layout.preferredWidth: implicitHeight
    }

    PlasmaExtras.Heading {
        text: rootItem.infoText

        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        maximumLineCount: 2

        Layout.fillWidth: true
    }
}
