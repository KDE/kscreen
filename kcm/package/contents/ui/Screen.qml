/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 2.9
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.5 as Kirigami

Controls.ScrollView {
    property var outputs
    property size totalSize

    function resetTotalSize() {
        totalSize = kcm.normalizeScreen();
    }

    onWidthChanged: resetTotalSize()
    onHeightChanged: resetTotalSize()

    property real relativeFactor: {
        var relativeSize = Qt.size(totalSize.width / (0.6 * width),
                                   totalSize.height / (0.6 * height));
        if (relativeSize.width > relativeSize.height) {
            // Available width smaller than height, optimize for width (we have
            // '>' because the available width, height is in the denominator).
            return relativeSize.width;
        } else {
            return relativeSize.height;
        }
    }

    property int xOffset: (width - totalSize.width / relativeFactor) / 2;
    property int yOffset: (height - totalSize.height / relativeFactor) / 2;

    implicitHeight: Math.max(root.height * 0.4, Kirigami.Units.gridUnit * 13)

    Component.onCompleted: background.visible = true;

    Kirigami.Heading {
        z: 90
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Kirigami.Units.smallSpacing
        }
        level: 4
        opacity: 0.6
        horizontalAlignment: Text.AlignHCenter
        text: i18n("Drag screens to re-arrange them")
        visible: kcm.outputModel && kcm.outputModel.rowCount() > 1
    }

    Controls.Button {
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            margins: Kirigami.Units.smallSpacing
        }
        z: 90

        onClicked: kcm.identifyOutputs()
        text: i18n("Identify")
        icon.name: "documentinfo"
        focusPolicy: Qt.NoFocus
        visible: kcm.outputModel && kcm.outputModel.rowCount() > 1
    }

    Repeater {
        model: kcm.outputModel
        delegate: Output {}

        onCountChanged: resetTotalSize()
    }
}
