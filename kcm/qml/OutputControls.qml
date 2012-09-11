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

import QtQuick 1.0
import org.kde.plasma.components 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1
import KScreen 1.0

Item {
	id: controls;
	property int rotationDirection;
	property Output output;
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
		}
	]

	/* FIXME The Output widget is not animated atm, so don't animate the
	 * controls either
	transitions: [
		Transition {
			RotationAnimation {
				direction:
					monitor.rotationDirection == RotationAnimation.Clockwise ?
						RotationAnimation.Counterclockwise :
						RotationAnimation.Clockwise;
				duration: 250;
				easing.type: "OutCubic";
			}
			PropertyAnimation {
				target: controls;
				properties: "width,height";
				duration: 250;
				easing.type: "OutCubic";
			}
		}
	]
	*/

	Text {
		id: monitorName;
		text: output.name;
		color: "black";
		font.pointSize: 15;

		anchors {
			top: parent.top;
			horizontalCenter: parent.horizontalCenter;
			margins: 10;
		}

		horizontalAlignment: Text.AlignHCenter;
	}

	QIconItem {
		id: stateButton;

		icon: stateIcon;

		/* Keep the button always big */
		scale: 1 / monitor.scale;

		anchors {
			horizontalCenter: parent.horizontalCenter;
			verticalCenter: parent.verticalCenter;
		}

		width: 30;
		height: 30;

		MouseArea {
			id: buttonMouseArea;
			anchors.fill: parent;
			acceptedButtons: Qt.LeftButton | Qt.RightButton;

			onClicked: {
				if (mouse.button == Qt.LeftButton) {
					if (monitor.state == "enabled") {
						monitor.state = "primary";
					} else if (monitor.state == "primary") {
						monitor.state = "disabled";
					} else if (monitor.state == "disabled") {
						monitor.state = "enabled";
					}
				} else if (mouse.button == Qt.RightButton) {
					if (monitor.state == "enabled") {
						monitor.state = "disabled";
					} else if (monitor.state == "disabled") {
						monitor.state = "primary";
					} else if (monitor.state == "primary") {
						monitor.state = "enabled";
					}
				}
			}
		}
	}

	Text {
		id: resolutionLabel;
		text: monitor.connected ?
			output.mode(output.currentMode).name :
			"";

		color: "black";
		font.pointSize: 10;

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
			margins: 10;
		}

		width: 30;
		height: 30;

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
}
