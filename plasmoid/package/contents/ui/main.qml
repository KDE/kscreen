/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.kquickcontrolsaddons 2.0
import org.kde.config as KConfig  // KAuthorized.authorizeControlModule
import org.kde.kcmutils // KCMLauncher
import org.kde.private.kscreen 1.0

PlasmoidItem {
    id: root

    // Only show if the user enabled presentation mode
    Plasmoid.status: presentationModeEnabled ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    toolTipSubText: presentationModeEnabled ? i18n("Presentation mode is enabled") : ""

    readonly property string kcmName: "kcm_kscreen"
    readonly property bool kcmAllowed: KConfig.KAuthorized.authorizeControlModule("kcm_kscreen")

    readonly property bool presentationModeEnabled: presentationModeCookie > 0
    property int presentationModeCookie: -1

    readonly property bool isLaptop: (pmSource.data["PowerDevil"] && pmSource.data["PowerDevil"]["Is Lid Present"]) ? true : false

    P5Support.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["PowerDevil", "Inhibitions"]

        onSourceAdded: source => {
            disconnectSource(source);
            connectSource(source);
        }
        onSourceRemoved: source => {
            disconnectSource(source);
        }

        readonly property var inhibitions: {
            var inhibitions = [];

            var data = pmSource.data.Inhibitions;
            if (data) {
                for (var key in data) {
                    if (key === "plasmashell" || key === "plasmoidviewer") { // ignore our own inhibition
                        continue;
                    }

                    inhibitions.push(data[key]);
                }
            }

            return inhibitions;
        }
    }

    PlasmaCore.Action {
        id: configureAction
        text: i18n("Configure Display Settings…")
        icon.name: "preferences-desktop-display"
        visible: kcmAllowed
        onTriggered: KCMLauncher.openSystemSettings(kcmName)
    }
    Component.onCompleted: {
        Plasmoid.setInternalAction("configure", configureAction);
    }

    fullRepresentation: ColumnLayout {
        spacing: 0
        Layout.preferredWidth: Kirigami.Units.gridUnit * 15

        ScreenLayoutSelection {
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.fillWidth: true
            screenLayouts: Plasmoid.availableActions().filter(action => action.action !== OsdAction.NoAction)
        }

        PresentationModeItem {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing * 2
            Layout.leftMargin: Kirigami.Units.smallSpacing
        }

        // compact the layout
        Item {
            Layout.fillHeight: true
        }
    }
}
