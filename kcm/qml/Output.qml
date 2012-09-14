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

	signal moved(bool snap);
	signal clicked();

	/* +1 because of the border */
	width: 1 + ((rotationTransformation.angle == 90 || rotationTransformation.angle == 270) ? monitor.height : monitor.width) * monitor.scale;
	height: 1 + ((rotationTransformation.angle == 90 || rotationTransformation.angle == 270) ? monitor.width : monitor.height) * monitor.scale;

	Rectangle {
		id: monitor;

		property bool enabled: output.enabled;
		property bool connected: output.connected;
		property bool primary: output.primary;
		property int currentModeId: output.currentMode;
		property int rotationDirection;

		x: 0;
		y: 0;
		border.width: 1;
		border.color: parent.focus ? "white" : "black";
		color: output.enabled ? "#B6D7A8" : "#E7EAEE";
		scale: (output.enabled) ? 1.0 : 0.6;

		transform: [
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
				easing.type: "OutElastic";
				duration: 350;
			}
		}

		Behavior on height {
			PropertyAnimation {
				property: "height";
				easing.type: "OutElastic";
				duration: 350;
			}
		}

		Behavior on rotation {
			RotationAnimation {
				easing.type: "OutCubic"
				duration: 250;
				direction: monitor.rotationDirection;
			}
		}

		/* Default size */
		width: 1000 / 8;
		height: 1000 / 8;

		/* FIXME: We need a much better math */
		onCurrentModeIdChanged: {
			var mode = output.mode(output.currentMode);
			if (mode == null) {
				monitor.width = 1000 / 8;
				monitor.height = 1000 / 8;
			} else {
				monitor.width = mode.size.width / 8;
				monitor.height = mode.size.height / 8;
			}
		}

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

			onPositionChanged: {
				/* Don't snap the outputs when holding Ctrl */
				root.moved(!(mouse.modifiers & Qt.ControlModifier));
			}

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