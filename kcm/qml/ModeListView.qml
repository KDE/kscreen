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

Rectangle {
	property alias model: listview.model;
	property alias currentItem: listview.currentItem;

	color: palette.base;
	focus: true;

	ListView {
		focus: true;
		id: listview;
		highlightFollowsCurrentItem: true;
		clip: true;
		anchors.fill: parent;

		delegate:
			PlasmaComponents.ListItem {
				property variant modelData: model;

				enabled: true;
				width: 150;

				Text {
					text: label;
					color: theme.textColor;
					font {
						family: theme.defaultFont.family;
						pointSize: theme.defaultFont.pointSize;
					}
				}

				checked: (index == listview.currentIndex);
				onClicked: listview.currentIndex = index;
			}
	}
}

