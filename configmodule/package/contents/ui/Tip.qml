import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

Item {

    property alias icon: tipIcon.source;
    property alias text: tipText.text;

    width: parent.width;
    height: units.iconSizes.toolbar;

    opacity: 0.0;

    Behavior on opacity {
        PropertyAnimation {
            duration: 100;
            easing.type: Easing.InOutQuad;
        }
    }

    PlasmaCore.IconItem {

        id: tipIcon;

        width: units.iconSizes.toolbar;
        height: units.iconSizes.toolbar;

        source: "dialog-information";
    }

    Text {

        id: tipText;

        anchors {
            left: tipIcon.right;
            leftMargin: 5;
            verticalCenter: parent.verticalCenter;
        }

        color: palette.text;
    }
}
