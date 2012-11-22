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
import org.kde.plasma.core 0.1

QMLOutput {
	id: root;

	property Item viewport;

	signal moved(bool snap);
	signal clicked();
	signal primaryTriggered();

	width: monitorMouseArea.width;
	height: monitorMouseArea.height;

	visible: (opacity > 0);
	opacity: output.connected ? 1.0 : 0.0;
	Behavior on opacity {
		PropertyAnimation {
			duration: 200;
			easing.type: "OutCubic";
		}
	}

	MouseArea {
		id: monitorMouseArea;

		onClicked: root.clicked();

		anchors {
			centerIn: parent;
		}

		drag {
			target: root;
			axis: Drag.XandYAxis;
			minimumX: 0;
			maximumX: viewport.width - root.width;
			minimumY: 0;
			maximumY: viewport.height - root.height;
			filterChildren: false;
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
			/* Don't snap the outputs when holding Ctrl or when
			 * they are disabled */
			root.moved(!(mouse.modifiers & Qt.ControlModifier) && output.enabled);
		}

		/* When button is pressed, emit clicked() signal
		* which is cought by QMLOutputView */
		onPressed: root.clicked();


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


		scale: (output.enabled) ? 1.0 : 0.6;
		Behavior on scale {
			PropertyAnimation {
				property: "scale";
				easing.type: "OutElastic";
				duration: 350;
			}
		}

		transformOrigin: Item.Center;
		rotation: (output.rotation == Output.None) ? 0 :
				(output.rotation == Output.Left) ? 90 :
				(output.rotation == Output.Inverted) ? 180 : 270;
		Behavior on rotation {
			RotationAnimation {
				easing.type: "OutCubic"
				duration: 250;
				direction: monitor.rotationDirection;
			}
		}
		onRotationChanged: updateRootProperties();

		width: root.currentOutputWidth * root.displayScale;
		Behavior on width {
			PropertyAnimation {
				property: "width";
				easing.type: "OutCubic";
				duration: 150;
			}
		}

		height: root.currentOutputHeight * root.displayScale;
		Behavior on height {
			PropertyAnimation {
				property: "height";
				easing.type: "OutCubic";
				duration: 150;
			}
		}

		FrameSvgItem {
			id: monitor;

			imagePath: "widgets/monitor";

			property bool enabled: output.enabled;
			property bool connected: output.connected;
			property bool primary: output.primary;
			property int currentModeId: output.currentMode;
			property int rotationDirection;

			x: 2;
			y: 2;
			width: parent.width - 4;
			height: parent.height - 4;


			OutputControls {
				id: controls;
				parentItem: root;
				rotationDirection: parent.rotationDirection;

				anchors {
					centerIn: parent;
				}

				onPrimaryTriggered: root.primaryTriggered();
			}
		}
	}


	/* Transormation of an item (rotation of the MouseArea) is only visual.
	 * The coordinates and dimensions are still the same (when you rotated
	 * 100x500 rectangle by 90 deg, it will still be 100x500, although
	 * visually it will be 500x100).
	 *
	 * This method calculates the real-visual coordinates and dimentions of
	 * the MouseArea and updates root item to match them. This makes snapping
	 * works correctly ragrdless on visual rotation of the output */
	function updateRootProperties()
	{
	      var transformedX, transformedY, transformedWidth, transformedHeight;

	      if ((output.rotation == Output.Left) || (output.rotation == Output.Right)) {
		    transformedWidth = monitorMouseArea.height;
		    transformedHeight = monitorMouseArea.width;
	      } else {
		    transformedWidth = monitorMouseArea.width;
		    transformedHeight = monitorMouseArea.height;
	      }

	      transformedX = root.x + (root.width / 2) - (transformedWidth / 2);
	      transformedY = root.y + (root.height / 2) - (transformedHeight / 2);

	      root.x = transformedX;
	      root.y = transformedY;
	      root.width = transformedWidth;
	      root.height = transformedHeight;
	}
}
