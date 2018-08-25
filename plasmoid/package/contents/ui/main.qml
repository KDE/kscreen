/*
 * Copyright (c) 2018 Kai Uwe Broulik <kde@broulik.de>
 *                    Work sponsored by the LiMux project of
 *                    the city of Munich.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    // does this need an ellipsis?
    readonly property string kcmLabel: i18nc("Open the full display settings module", "Advanced Display Settings")
    readonly property string kcmIconName: "preferences-desktop-display-randr"
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

    function action_openKcm() {
        KCMShell.open(kcmName);
    }

    Component.onCompleted: {
        if (kcmAllowed) {
            plasmoid.setAction("openKcm", root.kcmLabel, root.kcmIconName)
        }
    }

    Plasmoid.fullRepresentation: ColumnLayout {
        spacing: 0
        Layout.preferredWidth: units.gridUnit * 15

        ScreenLayoutSelection {
            Layout.fillWidth: true
        }

        PresentationModeItem {
            Layout.fillWidth: true
            Layout.topMargin: units.largeSpacing
        }

        // compact the layout, push settings button to the bottom
        Item {
            Layout.fillHeight: true
        }

        PlasmaComponents.Button {
            Layout.alignment: Qt.AlignRight
            Layout.topMargin: units.smallSpacing
            text: root.kcmLabel
            iconName: root.kcmIconName
            onClicked: action_openKcm()
        }
    }
}
