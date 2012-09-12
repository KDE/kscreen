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
					target: controls;
					stateIcon: "bookmarks";
				}
				PropertyChanges {
					target: output;
					primary: true;
				}
			},
			State {
				name: "enabled";
				PropertyChanges {
					target: controls;
					stateIcon: "dialog-ok-apply";
				}
				PropertyChanges {
					target: output;
					enabled: true;
				}
			},
			State {
				name: "disabled";
				PropertyChanges {
					target: controls;
					stateIcon: "edit-delete";
				}
				PropertyChanges {
					target: output;
					enabled: false;
				}
			}
		]

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
			 * which is cought by QMLOutputView */
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

			OutputControls {
				id: controls;
				parentItem: root;
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