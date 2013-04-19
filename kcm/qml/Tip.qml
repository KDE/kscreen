import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {

    property alias icon: tipIcon.icon;
    property alias text: tipText.text;

    width: parent.width;
    height: theme.iconSizes.toolbar;

    opacity: 0.0;

    Behavior on opacity {
        PropertyAnimation {
            duration: 100;
            easing.type: Easing.InOutQuad;
        }
    }

    PlasmaCore.Theme {

        id: theme;

    }

    QIconItem {

        id: tipIcon;

        width: theme.iconSizes.toolbar;
        height: theme.iconSizes.toolbar;

        icon: "dialog-information";
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
