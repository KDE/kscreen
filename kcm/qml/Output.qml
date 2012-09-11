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

QMLOutput {
	id: root;

	property Item viewport;

	signal moved();
	signal clicked();

	/* +1 because of the border */
	width: 1 + ((rotationTransformation.angle == 90 || rotationTransformation.angle == 270) ? monitor.height : monitor.width) * scaleTransformation.xScale;
	height: 1 + ((rotationTransformation.angle == 90 || rotationTransformation.angle == 270) ? monitor.width : monitor.height) * scaleTransformation.yScale;

	Rectangle {
		id: monitor;

		property bool enabled: output.enabled;
		property bool connected: output.connected;
		property int currentModeId: output.currentMode;
		property int rotationDirection;

		x: 0;
		y: 0;
		border.width: 1;
		border.color: "black";
		color: output.enabled ? "#B6D7A8" : "#E7EAEE";

		transform: [
			Scale {
				id: scaleTransformation;
				origin.x: monitor.width / 2;
				origin.y: monitor.height / 2;
				xScale: output.enabled ? 1.0 : 0.6;
				yScale: output.enabled ? 1.0 : 0.6;
			},
			Rotation {
				id: rotationTransformation;
				origin.x: monitor.width / 2;
				origin.y : monitor.height / 2;
				angle: (output.rotation == Output.None) ? 0 :
					(output.rotation == Output.Left) ? 90 :
					(output.rotation == Output.Inverted) ? 180 : 270;
			}
		]

		anchors.centerIn: parent;
		visible: output.connected;

		   /* Some nifty animation when changing backround color */
		Behavior on color {
			PropertyAnimation {
				duration: 150;
				easing.type: "OutCubic";
			}
		}

		Behavior on scale {
			PropertyAnimation {
				property: "scale";
				easing.type: "OutElastic";
				duration: 350;
			}
		}

		Behavior on width {
			PropertyAnimation {
				property: "width";
			}
		}

		Behavior on height {
			PropertyAnimation {
				property: "height";
			}
		}

		Behavior on rotation {
			RotationAnimation {
				easing.type: "OutCubic"
				duration: 250;
				direction: monitor.rotationDirection;
			}
		}

		/* FIXME: We need a much better math */
		onCurrentModeIdChanged: {
			monitor.width = output.mode(output.currentMode).size.width / 5;
			monitor.height = output.mode(output.currentMode).size.height / 5;
		}

		/* Don't use bindings, would cause binding loop */
		Component.onCompleted:
			monitor.state = output.primary ?
				"primary" :
				(output.enabled ? "enabled" : "disabled");

		states: [
			State {
				name: "primary";
				PropertyChanges {
					target: stateButton;
					icon: "bookmarks";
				}
				PropertyChanges {
					target: output;
					primary: true;
				}
			},
			State {
				name: "enabled";
				PropertyChanges {
					target: stateButton;
					icon: "dialog-ok-apply";
				}
				PropertyChanges {
					target: output;
					enabled: true;
				}
			},
			State {
				name: "disabled";
				PropertyChanges {
					target: stateButton;
					icon: "edit-delete";
				}
				PropertyChanges {
					target: output;
					enabled: false;
				}
			}
		]

		/* This must be created before stateButton */
		MouseArea {
			id: monitorMouseArea;
			anchors.fill: parent;
			drag {
				target: root;
				axis: Drag.XandYAxis;
				minimumX: 0;
				maximumX: viewport.width - root.width;
				minimumY: 0;
				maximumY: viewport.height - root.height;
				filterChildren: true;
			}

			drag.onActiveChanged: {
				/* If the drag is shorter then the animation then make sure
				* we won't end up in an inconsistent state */
				if (dragActiveChangedAnimation.running) {
				dragActiveChangedAnimation.complete();
				}
				
				dragActiveChangedAnimation.running = true;
			}

			onPositionChanged: root.moved();

			/* When button is pressed, emit clicked() signal
			 * which is caught by FocusGroup */
			onPressed: {
				mouse.accepted = true;
				root.clicked();
			}


			/* FIXME: This could be in 'Behavior', but MouseArea had
			 * some complaints...to tired to investigate */
			PropertyAnimation {
				id: dragActiveChangedAnimation;
				target: monitor;
				property: "opacity";
				from: monitorMouseArea.drag.active ? 0.7 : 1.0
				to: monitorMouseArea.drag.active ? 1.0 : 0.7
				duration: 100;
				easing.type: "OutCubic";
			}

			Item {
				id: controls;
				property int rotationDirection;

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

					icon: "dialog-ok-apply";

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

			Rectangle {
				id: rotationBar;

				width: parent.width - 20;
				height: 2;
				color: "black";

				anchors {
					left: parent.left;
					right: parent.right;
					bottom: parent.bottom;
					margins: 10;
				}
			}
		}
	}
}