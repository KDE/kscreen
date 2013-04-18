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
import org.kde.plasma.core 0.1 as PlasmaCore;

Item {
    id: root;

    property string outputName;
    property string modeName;

    width: childrenRect.width;
    height: childrenRect.height;

    SystemPalette {
        id: palette;
    }

    PlasmaCore.Theme {
        id: theme;
    }

    Rectangle {
        width: column.width + 80;
        height: column.height + 40;
        color: palette.base;
        border {
                width: 2;
                color: palette.shadow;
        }

        Column {

            id: column;

            anchors.centerIn: parent;
            spacing: 10;

            Text {

                id: nameLabel;

                anchors.horizontalCenter: parent.horizontalCenter;

                text: root.outputName;
                font.pointSize: 50;
                color: palette.text;
                horizontalAlignment: Text.AlignHCenter;
            }

            Text {

                id: modeLabel;

                anchors.horizontalCenter: parent.horizontalCenter;

                text: root.modeName;
                font.pointSize: theme.defaultFont.pointSize;
                color: palette.text;
                horizontalAlignment: Text.AlignHCenter;
            }
        }
    }
}
