/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.ScrollView {
    id: screenView

    required property var outputs
    required property bool interactive

    property size totalSize

    function resetTotalSize() {
        totalSize = kcm.normalizeScreen();
    }

    onWidthChanged: resetTotalSize()
    onHeightChanged: resetTotalSize()

    readonly property real relativeFactor: {
        const relativeSize = Qt.size(
            totalSize.width / (0.6 * width),
            totalSize.height / (0.65 * height),
        );
        if (relativeSize.width > relativeSize.height) {
            // Available width smaller than height, optimize for width (we have
            // '>' because the available width, height is in the denominator).
            return relativeSize.width;
        } else {
            return relativeSize.height;
        }
    }

    readonly property int xOffset: (width - totalSize.width / relativeFactor) / 2;
    readonly property int yOffset: (height - totalSize.height / relativeFactor) / 2;

    Kirigami.Heading {
        z: 90
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Kirigami.Units.smallSpacing
        }
        level: 4
        horizontalAlignment: Text.AlignHCenter
        text: i18n("Drag screens to re-arrange them")

        opacity: interactive ? 0.6 : 0
        visible: opacity > 0
        Behavior on opacity {
            PropertyAnimation {
                easing.type: Easing.InOutQuad
                duration: Kirigami.Units.shortDuration
            }
        }
    }

    Repeater {
        model: kcm.outputModel
        delegate: Output {
            interactive: screenView.interactive
        }

        onCountChanged: resetTotalSize()
    }
}
