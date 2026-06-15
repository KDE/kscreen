// SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import org.kde.kirigami as Kirigami

Kirigami.Badge {
    id: root

    required property int number

    visible: number >= 0
    text: number
    customColor: badgeColor(number)

    function badgeColor(number) {
        const colors = [
            Kirigami.Theme.highlightColor,
            Kirigami.Theme.visitedLinkColor,
            Kirigami.Theme.neutralTextColor,
            Kirigami.Theme.positiveTextColor,
            Kirigami.Theme.negativeTextColor,
            Kirigami.Theme.linkColor,
        ];
        return colors[(number - 1) % colors.length];
    }
}
