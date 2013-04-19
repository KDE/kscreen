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
import "OutputView.js" as JS;

Flickable {

    id: root;

    signal outputsChanged();
    signal outputChanged();
    signal moveMouse(int x, int y);
    signal outputMousePressed();
    signal outputMouseReleased();

    property int maxContentWidth;
    property int maxContentHeight;

    property Item activeOutput;
    property bool snappingEnabled: true;

    contentWidth: width;
    contentHeight: height;
    focus: true;

    onWidthChanged: reorderOutputs();
    onHeightChanged: reorderOutputs();

    Timer {
        id: autoScrollTimer;

        interval: 50;
        running: false;
        repeat: true;
        onTriggered: JS.doAutoScroll(root);
    }

    Timer {
        id: autoResizeTimer;

        interval: 50;
        running: (activeOutput && activeOutput.isDragged) ? true : false;
        repeat: true;
        onTriggered: JS.doAutoResize(root);
    }

    Keys.onPressed: {
        if (event.modifiers & Qt.ControlModifier) {
            snappingEnabled = false;
        }
    }

    Keys.onReleased: {
        if (!snappingEnabled) {
            snappingEnabled = true;
        }
    }

    /**
     * Adds a new \p output to the OutputView
     *
     * @param KScreen::Output output A new KScreen::Output to wrap into QMLOutput
     *          and add to the scene
     */
    function addOutput(output) {
        var component = Qt.createComponent("Output.qml");
        if (component.status == Component.Error) {
            console.log("Error creating output '" + output.name + "': " + component.errorString());
            return;
        }
        var qmlOutput = component.createObject(root.contentItem, { "output": output, "outputView": root });
        qmlOutput.z = root.children.length;

        qmlOutput.clicked.connect(outputClicked);
        qmlOutput.changed.connect(outputChanged);
        qmlOutput.moved.connect(outputMoved);
        qmlOutput.enabledToggled.connect(outputEnabledToggled);
        qmlOutput.primaryTriggered.connect(primaryTriggered);
        qmlOutput.output.isConnectedChanged.connect(outputConnected);
        qmlOutput.mousePressed.connect(outputMousePressed);
        qmlOutput.mouseReleased.connect(outputMouseReleased);

        if (!output.connected) {
            return;
        }

        qmlOutput.updateRootProperties();

        /* Maybe enable dragging of outputs? */
        root.outputConnected();

        /* Notify outter world */
        root.outputsChanged();
    }

    /**
     * Reorders all visible outputs to be in the middle of current view
     */
    function reorderOutputs() {
        var disabledOffset = root.width;

        var rectX = 0, rectY = 0, rectWidth = 0, rectHeight = 0;
        var positionedOutputs = [];
        for (var i = 0; i < root.contentItem.children.length; i++) {
            var qmlOutput = root.contentItem.children[i];

            if (!qmlOutput.output.connected) {
                qmlOutput.x = 0;
                qmlOutput.y = 0;
                continue;
            }

            /* Don't reposition outputs that user has moved from their default
             * position */
            if (qmlOutput.hasMoved) {
                continue;
            }

            if (!qmlOutput.output.enabled) {
                disabledOffset -= qmlOutput.width;
                qmlOutput.x = disabledOffset;
                qmlOutput.y = 0;
                continue;
            }

            qmlOutput.x = qmlOutput.output.pos.x * qmlOutput.displayScale;
            qmlOutput.y = qmlOutput.output.pos.y * qmlOutput.displayScale;

            if (qmlOutput.x < rectX) {
                rectX = qmlOutput.x;
            }

            var visualWidth;
            var visualHeight;
            if (qmlOutput.output.isHorizontal()) {
                visualWidth = qmlOutput.width;
                visualHeight = qmlOutput.height;
            } else {
                visualWidth = qmlOutput.height;
                visualHeight = qmlOutput.width;
            }
            if (qmlOutput.x + visualWidth > rectWidth) {
                rectWidth = qmlOutput.x + visualWidth;
            }

            if (qmlOutput.y < rectY) {
                rectY = qmlOutput.y;
            }

            if (qmlOutput.y + visualHeight > rectHeight) {
                rectHeight = qmlOutput.y + visualHeight;
            }

            positionedOutputs.push(qmlOutput);
        }

        var offsetX = rectX + ((root.contentWidth - rectWidth) / 2);
        var offsetY = rectY + ((root.contentHeight - rectHeight) / 2);
        for (var i = 0; i < positionedOutputs.length; i++) {
            var positionedOutput = positionedOutputs[i];
            positionedOutput.x = offsetX + (positionedOutput.output.pos.x * positionedOutput.displayScale);
            positionedOutput.y = offsetY + (positionedOutput.output.pos.y * positionedOutput.displayScale);
        }
    }

    /**
     * @return: Returns current primary QMLOutput or null when there's no
     *          output with 'primary' flag set.
     */
    function getPrimaryOutput() {
        for (var i = 0; i < root.contentItem.children.length; i++) {
            var qmlOutput = root.contentItem.children[i];
            if (qmlOutput.output.primary) {
                return qmlOutput;
            }
        }

        return null;
    }

    /**
     * Slot to be called when an output is clicked
     *
     * @param string outputName Name of the clicked QMLOutput
     */
    function outputClicked(outputName) {
        var output = JS.findOutputByName(root, outputName);

        for (var i = 0; i < root.contentItem.children.length; i++) {
            var qmlOutput = root.contentItem.children[i];

            if (qmlOutput == output) {
                var z = qmlOutput.z;

                for (var j = 0; j < root.contentItem.children.length; j++) {
                    var otherZ = root.contentItem.children[j].z;
                    if (otherZ > z) {
                        root.contentItem.children[j].z = otherZ - 1;
                    }
                }

                qmlOutput.z = root.contentItem.children.length;
                qmlOutput.focus = true;
                root.activeOutput = qmlOutput;

                break;
            }
        }
    }

    /**
     * Slot to be called when a primary flag is triggered on an output
     *
     * @param string outputName Name of the QMLOutput that has been triggered
     */
    function primaryTriggered(outputName) {
            /* Unset primary flag on all other outputs */
        var output = JS.findOutputByName(root, outputName);
        for (var i = 0; i < root.contentItem.children.length; i++) {
            var otherOutput = root.contentItem.children[i];

            if (otherOutput != output) {
                otherOutput.output.primary = false;
            }
        }
    }

    /**
     * Slot to be called when an output is moved (dragged) within the scene
     *
     * @param string outputName Name of the QMLOutput that is being dragged
     */
    function outputMoved(outputName) {
        var output = JS.findOutputByName(root, outputName);
        var x = output.x;
        var y = output.y;
        var width = output.width;
        var height = output.height;

        if (root.snappingEnabled) {
            JS.snapOutput(outputView, output);
        }

        if (output.cloneOf != null) {
            /* Reset position of the cloned screen and current screen and
             * don't care about any further positioning */
            output.outputX = 0;
            output.outputY = 0;
            output.cloneOf.outputX = 0;
            output.cloneOf.outputY = 0;

            return;
        }

        /* Left-most and top-most outputs. Other outputs are positioned
         * relatively to these */
        var cornerOutputs = JS.findCornerOutputs(outputView);
        if (cornerOutputs["left"] != null) {
            cornerOutputs["left"].outputX = 0
        }
        if (cornerOutputs["top"] != null) {
            cornerOutputs["top"].outputY = 0;
        }

        /* Only run autoscroll timer if there's anywhere to scroll */
        if ((root.contentX > 0) ||
            (root.contentY > 0) ||
            (root.contentWidth > root.width) ||
            (root.contentHeight > root.height)) {

            if ((output.x < root.contentX + 50) || /* left */
                (output.x > root.contentX + root.width - 50) || /* right */
                (output.y < root.contentY + 50) || /* top */
                (output.y > root.contentY + root.height - 50)) { /* bottom */

                if (!autoScrollTimer.running) {
                    JS._autoScrollStep = 0;
                    autoScrollTimer.start();
                }
            }
        }

        JS.updateVirtualPosition(outputView, output, cornerOutputs);
    }

    /**
     * Slot to be called when an output is enabled or disabled
     *
     * @param string outputName NAme of the QMLOutput that has been enabled or disabled
     */
    function outputEnabledToggled(outputName)
    {
        // Simulate a 'move'. This will recalculate virutal positions of all
        // outputs. See https://bugs.kde.org/show_bug.cgi?id=313027#c16
        outputMoved(outputName);


        var enabledCount = 0;
        for (var i = 0; i < root.contentItem.children.length; i++) {
            var output = root.contentItem.children[i];

            if (output.output.enabled) {
                enabledCount++;
                if (enabledCount > 0) {
                    break;
                }
            }
        }

        noActiveOutputsWarning.opacity = (enabledCount > 0) ? 0.0 : 1.0;
    }

    /**
     * Slot to be called whenever an output is (dis)connected.
     */
    function outputConnected()
    {
        var connectedCount = 0;

        for (var i = 0; i < root.contentItem.children.length; i++) {
            var output = root.contentItem.children[i];

            if (output.output.connected) {
                connectedCount++;
                if (connectedCount > 1) {
                    break;
                }
            }
        }

        for (var i = 0; i < root.contentItem.children.length; i++) {
            var output = root.contentItem.children[i];

            output.isDragEnabled = (connectedCount > 1);
            output.isToggleButtonVisible = (connectedCount > 1);
        }
    }
}
