/*
    SPDX-FileCopyrightText: 2012 Dan Vratil <dvratil@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Rectangle {
    id: root;

    property string outputName;
    property string modeName;

    color: theme.backgroundColor
    border {
        color: "red"
        width: units.smallSpacing * 2
    }

    width: childrenRect.width + 2 * childrenRect.x
    height: childrenRect.height + 2 * childrenRect.y

    PlasmaComponents.Label {
        id: displayName
        x: units.largeSpacing * 2
        y: units.largeSpacing
        font.pointSize: theme.defaultFont.pointSize * 3
        text: root.outputName;
        wrapMode: Text.WordWrap;
        horizontalAlignment: Text.AlignHCenter;
    }

    PlasmaComponents.Label {
        id: modeLabel;
        anchors {
            horizontalCenter: displayName.horizontalCenter
            top: displayName.bottom
        }
        text: root.modeName;
        horizontalAlignment: Text.AlignHCenter;
    }
}
