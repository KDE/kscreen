/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Window 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.Dialog {
    id: root
    location: PlasmaCore.Types.Floating
    type: PlasmaCore.Dialog.OnScreenDisplay
    outputOnly: true

    // OSD Timeout in msecs - how long it will stay on the screen
    property int timeout: 5000

    // Icon name to display
    property string icon
    property string infoText
    property string outputName
    property string modeName
    property bool animateOpacity: false
    property string itemSource
    property QtObject osdItem

    Behavior on opacity {
        SequentialAnimation {
            // prevent press and hold from flickering
            PauseAnimation { duration: root.timeout * 0.8 }

            NumberAnimation {
                duration: root.timeout * 0.2
                easing.type: Easing.InQuad
            }
        }
        enabled: root.timeout > 0 && root.animateOpacity
    }

    mainItem: Loader {
        source: itemSource
        onItemChanged: {
            if (item != undefined) {
                item.rootItem = root;
                root.osdItem = item
            }
        }

    }
}
