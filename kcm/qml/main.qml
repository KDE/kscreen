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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
	id: root;
	objectName: "root";
	focus: true;

	SystemPalette {
		id: palette;
	}
	/* Don't use colors from theme, we want to be consistent with KCM, not
	 * with Plasma here. Font sizes are the same though */
	PlasmaCore.Theme {
		id: theme;
	}

	FocusScope {
		id: outputViewFocusScope;
		anchors {
			left: parent.left;
			right: parent.right;
			top: parent.top;
		}

		height: root.height / 2;

		OutputView {
			id: outputView;
			objectName: "outputView";
			root: parent;

			anchors.fill: parent;

			Text {
				id: tip;
				anchors {
					left: parent.left;
					bottom: parent.bottom;
					margins: 5;
				}
				color: palette.text;
				text: i18n("Tip: Hold Ctrl while dragging the displays to disable snapping");
			}
		}
	}

	FocusScope {
		anchors {
			top: outputViewFocusScope.bottom;
			left: parent.left;
			right: parent.right;
			bottom: parent.bottom;
			margins: 20;
		}

		OutputSettings {
			id: settings;
			focus: true;
			anchors.fill: parent;
			output: outputView.activeOutput;
		}
	}
}
