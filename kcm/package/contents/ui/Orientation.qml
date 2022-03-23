/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.9
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.4 as Kirigami

ColumnLayout {
    Kirigami.FormData.label: i18n("Orientation:")
    Kirigami.FormData.buddyFor: autoRotateRadio
    spacing: Kirigami.Units.smallSpacing

    ColumnLayout {
        id: autoRotateColumn

        // TODO: Make this dependend on tablet mode being available
        enabled: element.internal
        visible: kcm.autoRotationSupported && kcm.orientationSensorAvailable

        ColumnLayout {
            Controls.RadioButton {
                id: autoRotateRadio
                text: i18n("Automatic")
                checked: autoRotateColumn.enabled && element.autoRotate
                onToggled: element.autoRotate = true
            }

            Controls.CheckBox {
                id: autoRotateOnlyInTabletMode
                Layout.leftMargin: Kirigami.Units.gridUnit

                text: i18n("Only when in tablet mode")
                enabled: autoRotateRadio.checked
                checked: enabled && element.autoRotateOnlyInTabletMode
                onToggled: element.autoRotateOnlyInTabletMode = checked
            }
        }

        Controls.RadioButton {
            id: manualRotateRadio
            text: i18n("Manual")
            checked: !element.autoRotate || !autoRotateColumn.enabled
            onToggled: element.autoRotate = false
        }
    }

    RowLayout {
       id: orientation
       enabled: !element.autoRotate || !autoRotateColumn.enabled || !autoRotateColumn.visible

       Controls.ButtonGroup {
           buttons: orientation.children
       }

       RotationButton {
           value: 0
       }
       RotationButton {
           value: 90
       }
       RotationButton {
           value: 180
       }
       RotationButton {
           value: 270
       }
    }
}
