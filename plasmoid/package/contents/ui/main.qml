/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.8
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.private.kscreen 1.0

Item {
    id: root

    // Only show if there's screen layouts available or the user enabled presentation mode
    Plasmoid.status: presentationModeEnabled || plasmoid.nativeInterface.connectedOutputCount > 1 ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.toolTipSubText: presentationModeEnabled ? i18n("Presentation mode is enabled") : ""

    readonly property string kcmName: "kcm_kscreen"
    readonly property bool kcmAllowed: KCMShell.authorize(kcmName + ".desktop").length > 0

    readonly property bool presentationModeEnabled: presentationModeCookie > 0
    property int presentationModeCookie: -1

    readonly property var screenLayouts: {
        var layouts = OsdAction.actionOrder().filter(function (layout) {
            // We don't want the "No action" item in the plasmoid
            return layout !== OsdAction.NoAction;
        });

        layouts.map(function (layout) {
            return {
                iconName: OsdAction.actionIconName(layout),
                label: OsdAction.actionLabel(layout),
                action: layout
            }
        });
    }

    function action_configure() {
        KCMShell.openSystemSettings(kcmName);
    }

    PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["PowerDevil", "Inhibitions"]

        onSourceAdded: {
            disconnectSource(source);
            connectSource(source);
        }
        onSourceRemoved: {
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

    Component.onCompleted: {
        if (kcmAllowed) {
            plasmoid.removeAction("configure");
            plasmoid.setAction("configure", i18n("Configure Display Settings…"), "preferences-desktop-display")
        }
    }

    Plasmoid.fullRepresentation: ColumnLayout {
        spacing: 0
        Layout.preferredWidth: units.gridUnit * 15

        ScreenLayoutSelection {
            Layout.leftMargin: units.smallSpacing
            Layout.fillWidth: true
        }

        PresentationModeItem {
            Layout.fillWidth: true
            Layout.topMargin: units.largeSpacing
            Layout.leftMargin: units.smallSpacing
        }

        // compact the layout
        Item {
            Layout.fillHeight: true
        }


    }
}
