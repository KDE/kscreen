/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents;
import org.kde.plasma.core 0.1 as PlasmaCore
import KScreen 1.0

PlasmaCore.Dialog {

    id: dialog;

    property Item parentItem;
    property Item visualParent;
    property int status: PlasmaComponents.DialogStatus.Closed;

    visible: false;
    windowFlags: Qt.Popup;
    onVisibleChanged: {
        if (visible) {
            status = PlasmaComponents.DialogStatus.Open;
        } else {
            status = PlasmaComponents.DialogStatus.Closed;
        }
    }

    mainItem: ModeSelectionWidget {
        id: contentItem;
        output: parentItem;
        width: 300;
    }

    function open() {
        var parent = dialog.visualParent ? dialog.visualParent : dialog.parent;
        var pos = dialog.popupPosition(parent, Qt.alignCenter);
        dialog.x = pos.x;
        dialog.y = pos.y;

        dialog.visible = true;
        dialog.activateWindow();
    }

    function close() {
        dialog.visible = false;
    }
}
