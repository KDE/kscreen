import QtQuick 1.1
import org.kde.kscreen 1.0

Item {
    id: root;

    property alias output: slider.output;
    property alias modes: slider.modes;

    SystemPalette {
        id: palette;
    }


    QMLSlider {
        id: slider;

        anchors {
            bottom: parent.bottom;
            leftMargin: 10;
            rightMargin: 10;
            horizontalCenter: parent.horizontalCenter;
        }
    }

    Text {
        id: leftText;

        color: "white";

        text: slider.modes.length > 0 ? slider.modes[0].name : "";

        anchors {
            left: parent.left;
            bottom: slider.top;
            bottomMargin: 5;
        }
    }

    Text {
        id: rightText;

        text: slider.modes.length > 1 ? slider.modes[slider.modes.length - 1].name : "";

        color: "white";

        anchors {
            right: parent.right;
            bottom: slider.top;
            bottomMargin: 5;
        }
    }
}
