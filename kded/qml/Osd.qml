/*
 * Copyright 2014 Martin Klapetek <mklapetek@kde.org>
 * Copyright 2016 Sebastian Kügler <sebas@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
