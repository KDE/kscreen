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
    }

    ComboBox {
        model: doctor.screenNames
        anchors {
            top: screensheading.bottom
            left: parent.left
            right: parent.right

        }
    }

    ColumnLayout {

    }

}
