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
	property int iconSize: 22;

	onWidthChanged: {
		setSmallMode(width < 100);
		monitorName.font.pointSize = (width < 80 || height < 50) ? 8 : (width < 100 || height < 80) ? 10 : 15;
	}

	onHeightChanged: {
		setSmallMode(height < 80);
		monitorName.font.pointSize = (width < 80 || height < 50) ? 6 : (width < 100 || height < 80) ? 10 : 15;
	}

	function setSmallMode(smallMode)
	{
		iconSize = (smallMode) ? 16 : 22;
		resolutionLabel.visible = !smallMode;
	}

	Behavior on rotation {
		RotationAnimation {
			easing.type: "OutCubic"
			duration: 250;
			direction: (rotationDirection == RotationAnimation.Clockwise) ?
				RotationAnimation.Counterclockwise : RotationAnimation.Clockwise;
		}
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

	/* Output name */
	Text {
		id: monitorName;
		text: output.name;
		color: "white";
		font.pointSize: 15;
		width: controls.width;

		anchors {
			top: parent.top;
		}

		horizontalAlignment: Text.AlignHCenter;

		Behavior on font.pointSize {
			PropertyAnimation {
				duration: 100;
			}
		}
	}

	/* Resolution label */
	Text {
		id: resolutionLabel;
		text: output.connected && output.enabled ?
			output.mode(output.currentMode).name + " @ " +
				Math.round(output.mode(output.currentMode).refreshRate, 1) + "Hz" :
			"";

		wrapMode: Text.Wrap;
		color: "white";
		font.pointSize: 10;
		width: controls.width - 10;

		anchors {
			bottom: parent.bottom;
			horizontalCenter: parent.horizontalCenter;
		}

		horizontalAlignment: Text.AlignHCenter;
	}

	/* Enable/Disable output */
	PlasmaComponents.Switch {
		id: enabledButton;


		anchors {
			horizontalCenter: parent.horizontalCenter;
			verticalCenter: parent.verticalCenter;
		}


		checked: output.enabled;
		onCheckedChanged: {
			/* FIXME: This should be in KScreen */
			if (output.enabled != enabledButton.checked) {
			  output.enabled = enabledButton.checked;
			}
		}
	  }


	/* Rotation */
	IconButton {
		id: rotateButton;
		iconSize: parent.iconSize;
		enabledIcon: "object-rotate-left";

		y: 5;
		x: parent.width - rotateButton.width - 10;

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

	/* Primary toggle */
	IconButton {
		id: primaryButton;
		iconSize: parent.iconSize;
		enabledIcon: "bookmarks";
		enabled: (output.enabled && output.primary);

		x: 10;
		y: 5;

		onClicked: {
			if (output.enabled) {
				output.primary = !output.primary
			}
		}
	}

}