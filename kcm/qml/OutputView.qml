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

Flickable {

    id: root;

    signal outputsChanged();
    signal outputChanged();
    signal moveMouse(int x, int y);

    property int maxContentWidth;
    property int maxContentHeight;

    property Item activeOutput;
//     //property alias outputs: contentItem.children;

    property int _autoScrollStep: 0;

    contentWidth: width;
    contentHeight: height;

    onWidthChanged: reorderOutputs(false);
    onHeightChanged: reorderOutputs(false);

    Timer {
        id: autoScrollTimer;

        interval: 50;
        running: false;
        repeat: true;
        onTriggered: doAutoScroll();
    }

    Timer {
        id: autoResizeTimer;

        interval: 50;
        running: true;
        repeat: true;
        onTriggered: doAutoResize();
    }

    function doAutoResize() {
        var cX = root.contentX, cY = root.contentY;
        var cW = root.contentWidth, cH = root.contentHeight;
        var rightMost = null, bottomMost = null;
        for (var ii = 0; ii < root.contentItem.children.length; ii++) {
            var qmlOutput = root.contentItem.children[ii];
            if (qmlOutput.output.conntected == false) {
                continue;
            }

            if ((rightMost == null) || (qmlOutput.x + qmlOutput.width > rightMost.x + rightMost.width)) {
                rightMost = qmlOutput;
            }

            if ((bottomMost == null) || (qmlOutput.y + qmlOutput.height > bottomMost.y + bottomMost.height)) {
                bottomMost = qmlOutput;
            }
        }

        if ((rightMost != null) && (bottomMost != null)) {
            var ncW = rightMost.x + rightMost.width;
            var ncH = bottomMost.y + bottomMost.height;

            root.resizeContent(ncW, ncH, Qt.point(0, 0));
        }
    }

    function doAutoScroll() {
        var verticalPos = root.contentY;
        var horizontalPos = root.contentX;

        var mouseX = _cursor.x;
        var mouseY = _cursor.y

        if (root._autoScrollStep < Math.max(root.width, root.height)) {
            root._autoScrollStep++;
        }

        if (activeOutput.isDragged) {
            if (mouseX < 50) {
                /* Apparently content{X,Y} can be set outside the boundaries of the contentItem,
                * so limit it to <0, root.contentWidth> and <0, root.contentHeight> */
                root.contentX = Math.max(0, horizontalPos - root._autoScrollStep);
                activeOutput.x = root.contentX + Math.max(0, mouseX) - activeOutput.width;
            } else if (mouseX > root.width - 50) {
                root.contentX = Math.min(root.contentWidth, horizontalPos + root._autoScrollStep);
                activeOutput.x = root.contentX + Math.min(root.width, mouseX) - activeOutput.width;
            }

            if (mouseY < 50) {
                root.contentY = Math.max(0, verticalPos - root._autoScrollStep);
                activeOutput.y = root.contentY + Math.max(0, mouseY) - activeOutput.height;
            } else if (mouseY > root.height - 50) {
                root.contentY = Math.min(root.contentHeight, verticalPos + root._autoScrollStep);
                activeOutput.y = root.contentY + Math.min(root.height, mouseY) - activeOutput.height;
            }
        }

        if (!activeOutput.isDragged ||
            (verticalPos == root.contentY) && (horizontalPos == root.contentX)) {

            autoScrollTimer.stop();
            root._autoScrollStep = 0;
        }

        doAutoResize();
    }

    function addOutput(output) {
        var component = Qt.createComponent("Output.qml");
        if (component.status == Component.Error) {
            console.log("Error creating output '" + output.name + "': " + component.errorString());
            return;
        }
        var qmlOutput = component.createObject(root.contentItem, { "output": output, "outputView": root });
        qmlOutput.z = root.children.length;

        qmlOutput.moved.connect(outputMoved);
        qmlOutput.clicked.connect(outputClicked);
        qmlOutput.changed.connect(outputChanged);
        qmlOutput.primaryTriggered.connect(primaryTriggered);

        if (!output.connected) {
            return;
        }

        root.outputsChanged();
    }

    function reorderOutputs(initialPlacement) {
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

            if (initialPlacement && !qmlOutput.output.enabled) {
                disabledOffset -= qmlOutput.width;
                qmlOutput.x = disabledOffset;
                qmlOutput.y = 0;
            }

            qmlOutput.x = qmlOutput.output.pos.x * qmlOutput.displayScale;
            qmlOutput.y = qmlOutput.output.pos.y * qmlOutput.displayScale;

            if (qmlOutput.x < rectX) {
                rectX = qmlOutput.x;
            }

            if (qmlOutput.x + qmlOutput.width > rectWidth) {
                rectWidth = qmlOutput.x + qmlOutput.width;
            }

            if (qmlOutput.y < rectY) {
                rectY = qmlOutput.y;
            }

            if (qmlOutput.y + qmlOutput.height > rectHeight) {
                rectHeight = qmlOutput.y + qmlOutput.height;
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

    function getPrimaryOutput() {
        for (var i = 0; i < root.contentItem.children.length; i++) {
            var qmlOutput = root.contentItem.children[i];
            if (qmlOutput.output.primary) {
                return qmlOutput;
            }
        }

        return null;
    }

    function outputClicked(outputName) {
        var output = findOutputByName(outputName);
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

    function outputMoved(outputName, snap) {
        var output = findOutputByName(outputName);
        var x = output.x;
        var y = output.y;
        var width = output.width;
        var height = output.height;

        /* FIXME: The size of the active snapping area should be the output */
        if (snap) {
            for (var ii = 0; ii < root.contentItem.children.length; ii++) {
                var otherOutput = root.contentItem.children[ii];

                if (otherOutput == output) {
                    continue;
                }

                if (!otherOutput.output.connected) {
                    continue;
                }

                var x2 = otherOutput.x;
                var y2 = otherOutput.y;
                var height2 = otherOutput.height;
                var width2 = otherOutput.width;
                var centerX = x + (width / 2);
                var centerY = y + (width / 2);
                var centerX2 = x2 + (width2 / 2);
                var centerY2 = y2 + (height2 / 2);

                /* @output is left of @otherOutput */
                if ((x + width > x2 - 30) && (x + width < x2 + 30) &&
                    (y + height > y2) && (y < y2 + height2)) {

                    output.x = x2 - width;
                    x = output.x;
                    centerX = x + (width / 2);
                    output.cloneOf = null;

                    /* @output is snapped to @otherOutput on left and their
                    * upper sides are aligned */
                    if ((x + width == x2) && (y < y2 + 5) && (y > y2 - 5)) {
                        output.y = y2;
                        break;
                    }

                    /* @output is snapped to @otherOutput on left and they
                    * are centered */
                    if ((x + width == x2) && (centerY < centerY2 + 5) && (centerY > centerY2 - 5)) {
                        output.y = centerY2 - (height / 2);
                        break;
                    }

                    /* @output is snapped to @otherOutput on left and their
                    * bottom sides are aligned */
                    if ((x + width == x2) && (y + height < y2 + height2 + 5) && (y + height > y2 + height2 - 5)) {
                        output.y = y2 + height2 - height;
                        break;
                    }
                }


                /* @output is right of @otherOutput */
                if ((x > x2 + width2 - 30) && (x < x2 + width2 + 30) &&
                    (y + height > y2) && (y < y2 + height2)) {

                    output.x = x2 + width2;
                    x = output.x;
                    centerX = x + (width / 2);
                    output.cloneOf = null;

                    /* @output is snapped to @otherOutput on right and their
                    * upper sides are aligned */
                    if ((x == x2 + width2) && (y < y2 + 5) && (y > y2 - 5)) {
                        output.y = y2;
                        break;
                    }

                    /* @output is snapped to @otherOutput on right and they
                    * are centered */
                    if ((x == x2 + width2) && (centerY < centerY2 + 5) && (centerY > centerY2 - 5)) {
                        output.y = centerY2 - (height / 2);
                        break;
                    }

                    /* @output is snapped to @otherOutput on right and their
                    * bottom sides are aligned */
                    if ((x == x2 + width2) && (y + height < y2 + height2 + 5) && (y + height > y2 + height2 - 5)) {
                        output.y = y2 + height2 - height;
                        break;
                    }
                }


                /* @output is above @otherOutput */
                if ((y + height > y2 - 30) && (y + height < y2 + 30) &&
                    (x + width > x2) && (x < x2 + width2)) {
                    output.y = y2 - height;
                    y = output.y;
                    centerY = y + (height / 2);
                    output.cloneOf = null;

                    /* @output is snapped to @otherOutput on top and their
                    * left sides are aligned */
                    if ((y + height == y2) && (x < x2 + 5) && (x > x2 - 5)) {
                        output.x = x2;
                        break;
                    }

                    /* @output is snapped to @otherOutput on top and they
                    * are centered */
                    if ((y + height == y2) && (centerX < centerX2 + 5) && (centerX > centerX2 - 5)) {
                        output.x = centerX2 - (width / 2);
                        break;
                    }

                    /* @output is snapped to @otherOutput on top and their
                    * right sides are aligned */
                    if ((y + height == y2) && (x + width < x2 + width2 + 5) && (x + width > x2 + width2 - 5)) {
                        output.x = x2 + width2 - width;
                        break;
                    }
                }


                /* @output is below @otherOutput */
                if ((y > y2 + height2 - 30) && (y < y2 + height2 + 30) &&
                    (x + width > x2) && (x < x2 + width2)) {
                    output.y = y2 + height2;
                    y = output.y;
                    centerY = y + (height / 2);
                    output.cloneOf = null;

                    /* @output is snapped to @otherOutput on bottom and their
                    * left sides are aligned */
                    if ((y == y2 + height2) && (x < x2 + 5) && (x > x2 - 5)) {
                        output.x = x2;
                        break;
                    }

                    /* @output is snapped to @otherOutput on bottom and they
                    * are centered */
                    if ((y == y2 + height2) && (centerX < centerX2 + 5) && (centerX > centerX2 - 5)) {
                        output.x = centerX2 - (width / 2);
                        break;
                    }

                    /* @output is snapped to @otherOutput on bottom and their
                    * right sides are aligned */
                    if ((y == y2 + height2) && (x + width < x2 + width2 + 5) && (x + width > x2 + width2 - 5)) {
                        output.x = x2 + width2 - width;
                        break;
                    }
                }


                /* @output is to be clone of @otherOutput (left top corners
                * are aligned */
                if ((x > x2) && (x < x2 + 10) &&
                    (y > y2) && (y < y2 + 10)) {

                    output.y = y2;
                    output.x = x2;

                    /* Find the most common cloned output and set this
                    * monitor to be clone of it as well */
                    var cloned = otherOutput.cloneOf;
                    if (cloned != null) {
                        output.cloneOf = otherOutput;
                    } else {
                        while (cloned) {
                            if (cloned.cloneOf == null && (cloned != output)) {
                                output.cloneOf = cloned;
                                break;
                            }

                            cloned = cloned.cloneOf;
                        }
                    }

                    break;
                }

                /* If the item did not match any of the conditions
                * above then it's not a clone either :) */
                output.cloneOf = null;
            }
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
        *relatively to these */
        var topMostOutput = null;
        var leftMostOutput = null;
        var rightMostOutput = null;
        var bottomMostOutput = null;

        for (var ii = 0; ii < root.contentItem.children.length; ii++) {
            var otherOutput = root.contentItem.children[ii];

            if (!otherOutput.output.connected || !otherOutput.output.enabled) {
                continue;
            }

            if ((leftMostOutput == null) || (otherOutput.x < leftMostOutput.x)) {
                leftMostOutput = otherOutput;
            }

            if ((topMostOutput == null) || (otherOutput.y < topMostOutput.y)) {
                topMostOutput = otherOutput;
            }

            if ((rightMostOutput == null) ||
                (otherOutput.x + otherOutput.width > rightMostOutput.x + rightMostOutput.width)) {
                rightMostOutput = otherOutput;
            }

            if ((bottomMostOutput == null) ||
                (otherOutput.y + otherOutput.height > bottomMostOutput.y + bottomMostOutput.height)) {
                bottomMostOutput = otherOutput;
            }
        }

        if (leftMostOutput != null) {
            leftMostOutput.outputX = 0
        }

        if (topMostOutput != null) {
            topMostOutput.outputY = 0;
        }

        if ((output.x < root.contentX + 50) || /* left */
            (output.x > root.contentX + root.width - 50) || /* right */
            (output.y < root.contentY + 50) || /* top */
            (output.y > root.contentY + root.height - 50)) { /* bottom */

            if (!autoScrollTimer.running) {
                root._autoScrollStep = 0;
                autoScrollTimer.start();
            }
        }

        /* If the leftmost output is currently being moved, then reposition
         * all output relatively to it, otherwise reposition the current output
         * relatively to the leftmost output */
        if (output == leftMostOutput) {
            for (var ii = 0; ii < root.contentItem.children.length; ii++) {
                var otherOutput = root.contentItem.children[ii];

                if (otherOutput == leftMostOutput) {
                    continue;
                }

                if (!otherOutput.output.connected ||
                    !otherOutput.output.enabled) {
                    continue;
                }

                otherOutput.outputX = (otherOutput.x - leftMostOutput.x) / otherOutput.displayScale;
            }
        } else if (leftMostOutput != null) {
            output.outputX = (output.x - leftMostOutput.x) / output.displayScale;
        }

        /* If the topmost output is currently being moved, then reposition
        * all outputs relatively to it, otherwise reposition the current output
        * relatively to the topmost output */
        if (output == topMostOutput) {
            for (var ii = 0; ii < root.contentItem.children.length; ii++) {
                var otherOutput = root.contentItem.children[ii];

                if (otherOutput == topMostOutput) {
                    continue;
                }

                if (!otherOutput.output.connected ||
                    !otherOutput.output.enabled) {
                    continue;
                }

                otherOutput.outputY = (otherOutput.y - topMostOutput.y) / otherOutput.displayScale;
            }
        } else if (topMostOutput != null) {
            output.outputY = (output.y - topMostOutput.y) / output.displayScale;
        }

        var debug = false;
        if (debug) {
            console.log("Leftmost:" + leftMostOutput.output.name);
            console.log("Topmost:" + topMostOutput.output.name);
            for (var ii = 0; ii < root.contentItem.children.length; ii++) {
                var otherOutput = root.contentItem.children[ii];
                if (!otherOutput.output.connected || !otherOutput.output.enabled) {
                    continue;
                }

                console.log(otherOutput.output.name + ": " + otherOutput.outputX + "," + otherOutput.outputY);
            }
            console.log("");
        }
    }

    function primaryTriggered(outputName) {
            /* Unset primary flag on all other outputs */
        var output = findOutputByName(outputName);
        for (var i = 0; i < root.contentItem.children.length; i++) {
            var otherOutput = root.contentItem.children[i];

            if (otherOutput != output) {
                otherOutput.output.primary = false;
            }
        }
    }

    function findOutputByName(outputName) {
        for (var i = 0; i < root.contentItem.children.length; i++) {
            var output = root.contentItem.children[i];

            if (output.output.name == outputName) {
                return output;
            }
        }

        return null;
    }
}
