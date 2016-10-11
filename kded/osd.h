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

#ifndef OSD_H
#define OSD_H

#include <QObject>
#include <QString>

#include <KScreen/Output>
class QmlObject;

namespace KDeclarative {
    class QmlObject;
}
namespace Plasma {
}

class QTimer;

namespace KScreen {

class Osd : public QObject {
    Q_OBJECT

public:
    Osd(QObject *parent = nullptr);
    ~Osd() override;

    bool setRootProperty(const char *name, const QVariant &value);
    void showOutputIdentifier(const KScreen::OutputPtr output);

public Q_SLOTS:
    void showText(const QString &icon, const QString &text);

Q_SIGNALS:
    void osdProgress(const QString &icon, const int percent, const QString &additionalText);
    void osdText(const QString &icon, const QString &text);

private Q_SLOTS:
    void hideOsd();

private:
    bool init();

    void showProgress(const QString &icon, const int percent, const QString &additionalText = QString());
    void showOsd();

    void updatePosition();
    QString m_osdPath;
    KDeclarative::QmlObject *m_osdObject = nullptr;
    QTimer *m_osdTimer = nullptr;
    int m_timeout = 0;

    KScreen::OutputPtr m_output;
};

} // ns

#endif // OSD_H
