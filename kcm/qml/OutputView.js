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

/**
 * Step by which to auto scroll. This is 0 at the beginning of drag and grows
 * by 1 with every tick of the autoScrollTimer
 */
var _autoScrollStep = 0;

/**
 * Automatically resizes outputView to match bounding rectangle of all outputs
 * @param OutputView outputView An OutputView to resize
 */
function doAutoResize(outputView) {
    var cX = outputView.contentX, cY = outputView.contentY;
    var cW = outputView.contentWidth, cH = outputView.contentHeight;
    var rightMost = null, bottomMost = null;
    for (var ii = 0; ii < outputView.contentItem.children.length; ii++) {
        var qmlOutput = outputView.contentItem.children[ii];
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

        if (ncW != cW) {
            outputView.contentWidth = ncW;
            outputView.contentX = cX;
        }

        if (ncH != cH) {
            outputView.contentHeight = ncH;
            outputView.contentY = cY;
        }
    }
}

/**
 * Automatically scrolls outputView so that currently dragged output is still visible
 * 
 * @param OutputView outputView An OutputView to scroll
 */
function doAutoScroll(outputView) {
    var verticalPos = outputView.contentY;
    var horizontalPos = outputView.contentX;

    var mouseX = _cursor.x;
    var mouseY = _cursor.y

    if (_autoScrollStep < Math.max(outputView.width, outputView.height)) {
        _autoScrollStep++;
    }

    if (activeOutput.isDragged) {
        if (mouseX < 50) {
            /* Apparently content{X,Y} can be set outside the boundaries of the contentItem,
            * so limit it to <0, outputView.contentWidth> and <0, outputView.contentHeight> */
            outputView.contentX = Math.max(0, horizontalPos - _autoScrollStep);
            activeOutput.x = outputView.contentX + Math.max(0, mouseX) - activeOutput.width;
        } else if (mouseX > outputView.width - 50) {
            outputView.contentX = Math.min(outputView.contentWidth, horizontalPos + _autoScrollStep);
            activeOutput.x = outputView.contentX + Math.min(outputView.width, mouseX) - activeOutput.width;
        }

        if (mouseY < 50) {
            outputView.contentY = Math.max(0, verticalPos - _autoScrollStep);
            activeOutput.y = outputView.contentY + Math.max(0, mouseY) - activeOutput.height;
        } else if (mouseY > outputView.height - 50) {
            outputView.contentY = Math.min(outputView.contentHeight, verticalPos + _autoScrollStep);
            activeOutput.y = outputView.contentY + Math.min(outputView.height, mouseY) - activeOutput.height;
        }
    }

    if (!activeOutput.isDragged ||
        (verticalPos == outputView.contentY) && (horizontalPos == outputView.contentX)) {

        autoScrollTimer.stop();
        _autoScrollStep = 0;
    }
}

/**
 * @param OutputView outputView Parent OutputView in which to search
 * @param string     outputName Name of output to look for
 * @returns: Returns QMLOutput with name @outputName or NULL if no such output
 *           exists.
 */
function findOutputByName(outuptView, outputName) {
    for (var ii = 0; ii < outputView.contentItem.children.length; ii++) {
        var output = outputView.contentItem.children[ii];

        if (output.output.name == outputName) {
            return output;
        }
    }

    return null;
}

/**
 * @param OutputView outputView Parent OutputView in which to search
 * @returns: Returns an associative array of leftmost, topmost, rightmost
 *           and bottommost outputs on @outputView
 */
function findCornerOutputs(outputView) {
    var cornerOutputs = new Array();
    cornerOutputs["left"] = null;
    cornerOutputs["top"] = null;
    cornerOutputs["right"] = null;
    cornerOutputs["bottom"] = null;

    for (var ii = 0; ii < root.contentItem.children.length; ii++) {
        var output = root.contentItem.children[ii];
        var otherOutput;

        if (!output.output.connected || !output.output.enabled) {
            continue;
        }

        otherOutput = cornerOutputs["left"];
        if ((otherOutput == null) || (output.x < otherOutput.x)) {
            cornerOutputs["left"] = output;
        }

        otherOutput = cornerOutputs["top"];
        if ((otherOutput == null) || (output.y < otherOutput.y)) {
            cornerOutputs["top"] = output;
        }

        otherOutput = cornerOutputs["right"];
        if ((otherOutput == null) || (output.x + output.width > otherOutput.x + otherOutput.width)) {
            cornerOutputs["right"] = output;
        }

        otherOutput = cornerOutputs["bottom"];
        if ((otherOutput == null) || (output.y + output.height > otherOutput.y + otherOutput.height)) {
            cornerOutputs["bottom"] = output;
        }
    }

    return cornerOutputs;
}

/**
 * @param OutputView outputView    A parent OutputView
 * @param QMLOutput  output        A QMLOutput to reposition (or relatively to
 *         which reposition other outputs in @outputView)
 * @param Array      cornerOutputs An associative array of leftmost, topmost, rightmost and
 *         bottomost outputs on the @outputView
 */
function updateVirtualPosition(outputView, output, cornerOutputs) {

    /* If the leftmost output is currently being moved, then reposition
     * all output relatively to it, otherwise reposition the current output
     * relatively to the leftmost output */
    if (output == cornerOutputs["left"]) {
        for (var ii = 0; ii < outputView.contentItem.children.length; ii++) {
            var otherOutput = outputView.contentItem.children[ii];

            if (otherOutput == cornerOutputs["left"]) {
                continue;
            }

            if (!otherOutput.output.connected ||
                !otherOutput.output.enabled) {
                continue;
            }

            otherOutput.outputX = (otherOutput.x - cornerOutputs["left"].x) / otherOutput.displayScale;
        }
    } else if (cornerOutputs["left"] != null) {
        output.outputX = (output.x - cornerOutputs["left"].x) / output.displayScale;
    }

    /* If the topmost output is currently being moved, then reposition
     * all outputs relatively to it, otherwise reposition the current output
     * relatively to the topmost output */
    if (output == cornerOutputs["top"]) {
        for (var ii = 0; ii < outputView.contentItem.children.length; ii++) {
            var otherOutput = outputView.contentItem.children[ii];

            if (otherOutput == cornerOutputs["top"]) {
                continue;
            }

            if (!otherOutput.output.connected ||
                !otherOutput.output.enabled) {
                continue;
            }

            otherOutput.outputY = (otherOutput.y - cornerOutputs["top"].y) / otherOutput.displayScale;
        }
    } else if (cornerOutputs["top"] != null) {
        output.outputY = (output.y - cornerOutputs["top"].y) / output.displayScale;
    }
}

/**
 * @param OutputView outputView Parent OutputView
 * @param QMLOutput  output     A QMLOutput to snap to other output in @outputView
 */
function snapOutput(outputView, output) {
    var x = output.x;
    var y = output.y;
    var height = output.height;
    var width = output.width;
    var centerX = x + (width / 2);
    var centerY = y + (height / 2);

    for (var ii = 0; ii < outputView.contentItem.children.length; ii++) {
        var otherOutput = outputView.contentItem.children[ii];

        if (otherOutput == output) {
            continue;
        }

        if (!otherOutput.output.connected || !otherOutput.output.enabled) {
            continue;
        }

        var x2 = otherOutput.x;
        var y2 = otherOutput.y;
        var height2 = otherOutput.height;
        var width2 = otherOutput.width;
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
