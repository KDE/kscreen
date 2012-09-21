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
import org.kde.qtextracomponents 0.1

MouseArea {
	id: mouseArea;

	property int iconSize: 22;
	property bool enabled: true;
	property string enabledIcon;
	property string disabledIcon;

	width: iconSize;
	height: iconSize;

	QIconItem {
		id: button;
		icon: parent.enabled ?
			parent.enabledIcon :
			(parent.disabledIcon == "") ?
				parent.enabledIcon :
				parent.disabledIcon;

		enabled: parent.enabled;
		anchors.fill: parent;
	}

	Behavior on width {
		PropertyAnimation {
			duration: 100;
		}
	}

	Behavior on height {
		PropertyAnimation {
			duration: 100;
		}
	}
}