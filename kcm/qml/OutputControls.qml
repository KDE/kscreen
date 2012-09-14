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
import org.kde.plasma.components 0.1 as PlasmaComponents;
import org.kde.plasma.core 0.1 as PlasmaCore
import KScreen 1.0

Item {
	id: controls;
	property int rotationDirection;
	property Item parentItem;
	property string stateIcon: "dialog-ok-apply";

	anchors {
		verticalCenter: parent.verticalCenter;
		horizontalCenter: parent.horizontalCenter;
	}

	state: "normal";
	states: [
		State {
			name: "normal";
			when: output.rotation == Output.None;
			PropertyChanges {
				target: controls;
				rotation: 0;
				width: parent.width - 20;
				height: parent.height - 20;
			}
			AnchorChanges {
				target: monitorName;
				anchors {
					top: parent.top;
				}
			}
		},
		State {
			name: "left";
			when: output.rotation == Output.Left;
			PropertyChanges {
				target: controls;
				rotation: 270;
				width: parent.height - 20;
				height: parent.width - 20;
			}
			AnchorChanges {
				target: monitorName;
				anchors {
					top: rotateButton.bottom;
				}
			}
		},
		State {
			name: "inverted";
			when: output.rotation == Output.Inverted;
			PropertyChanges {
				target: controls;
				rotation: 180;
				width: parent.width - 20;
				height: parent.height - 20;
			}
			AnchorChanges {
				target: monitorName;
				anchors {
					top: parent.top;
				}
			}
		},
		State {
			name: "right";
			when: output.rotation == Output.Right;
			PropertyChanges {
				target: controls;
				rotation: 90;
				width: parent.height - 20;
				height: parent.width - 20;
			}
			AnchorChanges {
				target: monitorName;
				anchors {
					top: rotateButton.bottom;
				}
			}
		}
	]

	Text {
		id: monitorName;
		text: output.name;
		color: "black";
		font.pointSize: 15;
		width: parent.width;

		anchors {
			top: parent.top;
			margins: 10;
		}

		horizontalAlignment: Text.AlignHCenter;
	}

	QIconItem {
		id: stateButton;

		icon: output.enabled ? "dialog-ok-apply" : "edit-delete"

		/* Keep the button always big */
		scale: 1 / monitor.scale;

		anchors {
			horizontalCenter: parent.horizontalCenter;
			verticalCenter: parent.verticalCenter;
		}

		width: 22;
		height: 22;

		MouseArea {
			id: buttonMouseArea;
			anchors.fill: parent;

			onClicked: output.enabled = !output.enabled;
		}
	}

	Text {
		id: resolutionLabel;
		text: output.connected && output.enabled ?
			output.mode(output.currentMode).name + " @ " +
				Math.round(output.mode(output.currentMode).refreshRate, 1) + "Hz" :
			"";

		wrapMode: Text.Wrap;
		color: "black";
		font.pointSize: 10;
		width: parent.width - 20;

		anchors {
			bottom: parent.bottom;
			horizontalCenter: parent.horizontalCenter;
			margins: 10;
			bottomMargin: 15;
		}

		horizontalAlignment: Text.AlignHCenter;
	}

	QIconItem {
		id: rotateButton;

		icon: "object-rotate-left";

		anchors {
			right: parent.right;
			top: parent.top;
			margins: 5;
		}

		width: 22;
		height: 22;

		MouseArea {
			id: rotateButtonMouseArea;
			anchors.fill: parent;
			acceptedButtons: Qt.LeftButton | Qt.RightButton;

			onClicked: {
				if (mouse.button == Qt.LeftButton) {
					monitor.rotationDirection = RotationAnimation.Counterclockwise;
					if (output.rotation == Output.None) {
						output.rotation = Output.Right;
					} else if (output.rotation == Output.Right) {
						output.rotation = Output.Inverted;
					} else if (output.rotation == Output.Inverted) {
						output.rotation = Output.Left;
					} else if (output.rotation == Output.Left) {
						output.rotation = Output.None;
					}
				} else {
					monitor.rotationDirection = RotationAnimation.Clockwise;
					if (output.rotation == Output.None) {
						output.rotation = Output.Left;
					} else if (output.rotation == Output.Left) {
						output.rotation = Output.Inverted;
					} else if (output.rotation == Output.Inverted) {
						output.rotation = Output.Right;
					} else if (output.rotation == Output.Right) {
						output.rotation = Output.None;
					}
				}
			}
		}
	}

	MouseArea {
		id: primaryButton;

		anchors {
			left: parent.left;
			top: parent.top;
			margins: 5;
		}

		width: 22;
		height: 22;

		QIconItem {
			id: primaryButtonIcon;

			icon: "bookmarks";
			state: QIconItem.DisabledState;
			enabled: (output.enabled && output.primary);
			anchors.fill: parent;
		}

		onClicked: {
			if (output.enabled) {
				output.primary = !output.primary
			}
		}
	}

}