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

#ifndef KSCREENDOCIMPORT_H
#define KSCREENDOCIMPORT_H

#include <QObject>
#include <QStringList>

class KScreenDoctor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList outputNames READ outputNames NOTIFY outputNamesChanged)
    Q_PROPERTY(QString currentOutput READ currentOutput WRITE setCurrentOutput NOTIFY currentOutputChanged)
    Q_PROPERTY(int currentOutputRotation READ currentOutputRotation NOTIFY currentOutputRotationChanged)

public:
    explicit KScreenDoctor(QObject *parent = Q_NULLPTR);

    QStringList outputNames() const;
    QString currentOutput() const;
    int currentOutputRotation() const;

public Q_SLOTS:
    void setCurrentOutput(const QString &outputName);
    Q_INVOKABLE void setRotation(const QString &outputName, int rotation);

Q_SIGNALS:
    void outputNamesChanged() const;
    void currentOutputChanged() const;
    void currentOutputRotationChanged() const;

private:
    QString m_currentOutput;
};

#endif // KSCREENDOCIMPORT_H
