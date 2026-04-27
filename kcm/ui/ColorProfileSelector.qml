/*
    SPDX-FileCopyrightText: 2026 Xaver Hugl <xaver.hugl@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

import org.kde.private.kcm.kscreen as KScreen

RowLayout {
    id: selector

    required property var colorProfileSource
    signal sourceChanged();

    required property bool supportsIccProfile
    required property bool supportsBuiltInProfile
    required property var comboboxWidth

    // Set the same limit as the device ComboBox
    Layout.maximumWidth: Kirigami.Units.gridUnit * 14
    Kirigami.FormData.label: i18nc("@label:listbox", "Color profile:")
    Kirigami.FormData.buddyFor: colorProfileCombobox
    spacing: Kirigami.Units.smallSpacing

    QQC2.ComboBox {
        id: colorProfileCombobox
        Layout.minimumWidth: comboboxWidth
        model: [
            {
                text: i18nc("@item:inlistbox color profile", "None"),
                value: KScreen.Output.ColorProfileSource.sRGB,
                available: true
            },
            {
                text: i18nc("@item:inlistbox color profile", "ICC profile"),
                value: KScreen.Output.ColorProfileSource.ICC,
                available: supportsIccProfile
            },
            {
                text: i18nc("@item:inlistbox color profile", "Built-in"),
                value: KScreen.Output.ColorProfileSource.EDID,
                available: supportsBuiltInProfile
            }
        ]
        textRole: "text"
        valueRole: "value"

        onActivated: {
            colorProfileSource = currentValue;
            selector.sourceChanged();
        }
        Component.onCompleted: currentIndex = indexOfValue(colorProfileSource);

        delegate: QQC2.MenuItem {
            text: modelData.text
            enabled: modelData.available
            highlighted: colorProfileCombobox.highlightedIndex == index
        }
    }
    Kirigami.ContextualHelpButton {
        toolTipText: i18nc("@info:tooltip", "Use the color profile built into the screen itself, if present. Note that built-in color profiles are sometimes wrong, and often inaccurate. For optimal color fidelity, calibration using a colorimeter is recommended.")
        visible: colorProfileSource == KScreen.Output.ColorProfileSource.EDID
    }
}
