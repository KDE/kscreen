/*
    Copyright 2017 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.private.kscreendoctor 1.0

FocusScope {
    focus: true

    PlasmaExtras.Heading {
        id: screensheading
        level: 3
        text: i18n("Controls go here. :-)")

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }

    KScreenDoctor {
        id: doctor

        onOutputNamesChanged: outputCombo.updateCurrent()
        onCurrentOutputChanged: outputCombo.updateCurrent()
    }

    ComboBox {
        id: outputCombo
        model: doctor.outputNames
        anchors {
            top: screensheading.bottom
            left: parent.left
            right: parent.right

        }
        function updateCurrent() {
            for (var i = 0; i < doctor.outputNames.length; i++) {
                if (doctor.outputNames[i] == doctor.currentOutput) {
                    outputCombo.currentIndex = i;
                }
            }
        }
        onActivated: {
            print("Selected: " + doctor.outputNames[index])
            doctor.currentOutput = doctor.outputNames[index]
        }
    }

    ColumnLayout {
        anchors {
            top: outputCombo.bottom
            left: parent.left
            right: parent.right
            //bottom: parent.bottom

        }
        Button {
            text: i18n("Normal")
            onClicked: doctor.setRotation(0);
        }
        Button {
            text: i18n("90°")
            onClicked: doctor.setRotation(90);
        }
        Button {
            text: i18n("Upside down")
            onClicked: doctor.setRotation(180);
        }
        Button {
            text: i18n("270°")
            onClicked: doctor.setRotation(270);
        }

    }
}
