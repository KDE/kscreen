/*
 *  Copyright 2014 (c) Martin Klapetek <mklapetek@kde.org>
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KSCREEN_OSD_H
#define KSCREEN_OSD_H

#include <QObject>
#include <QRect>
#include <QString>

#include <KScreen/Output>

namespace KDeclarative {
    class QmlObject;
}

class QTimer;

namespace KScreen {

class Osd : public QObject {

    Q_OBJECT

public:
    Osd(QObject *parent = nullptr);
    ~Osd() override;

    void showOutputIdentifier(const KScreen::OutputPtr output);

private Q_SLOTS:
    void hideOsd();

private:
    void showOsd();
    void updatePosition();

    QString m_osdPath;
    QRect m_outputGeometry;
    KDeclarative::QmlObject *m_osdObject = nullptr;
    QTimer *m_osdTimer = nullptr;
    int m_timeout = 0;

};

} // ns

#endif // KSCREEN_OSD_H
