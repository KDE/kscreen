/*************************************************************************************
 *  Copyright 2017 by Sebastian Kügler <sebas@kde.org>                               *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#include "xtouchscreen.h"

#include <QStandardPaths>
#include <QProcess>

#include <QStringList>

using namespace KScreen;

class XTouchscreen::Private
{
  public:
    Private():
        id(0),
        rotation(Output::None)
    {}

    Private(const Private &other):
        id(other.id),
        name(other.name),
        transformationMatrix(other.transformationMatrix),
        rotation(other.rotation)
    {}


    int id;
    QString name;
    QString transformationMatrix;
    QHash<Output::Rotation, QString> m_transformationMatrices;
    Output::Rotation rotation;
};

XTouchscreen::XTouchscreen(QObject* parent)
 : QObject(parent)
 , d(new Private())
{

}

XTouchscreen::XTouchscreen(XTouchscreen::Private *dd)
 : QObject()
 , d(dd)
{
}

XTouchscreen::~XTouchscreen()
{
    delete d;
}

// XTouchscreenPtr XTouchscreen::clone() const
// {
//     return XTouchscreenPtr(new XTouchscreen(new Private(*d)));
// }

int XTouchscreen::id() const
{
    return d->id;
}

void XTouchscreen::setId(int id)
{
    if (d->id == id) {
        return;
    }

    d->id = id;
}

QString XTouchscreen::name() const
{
    return d->name;
}

void XTouchscreen::setName(const QString& name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;

    Q_EMIT nameChanged();
}

QStringList XTouchscreen::transformationMatrix(KScreen::Output::Rotation rot)
{
    QString matrix;
    switch (rot) {
        case KScreen::Output::None:
            matrix = QStringLiteral("1 0 0 0 1 0 0 0 1");
            break;
        case KScreen::Output::Left:
            matrix = QStringLiteral("0 -1 1 1 0 0 0 0 1");
            break;
        case KScreen::Output::Right:
            matrix = QStringLiteral("0 1 0 -1 0 1 0 0 1");
            break;
        case KScreen::Output::Inverted:
            matrix = QStringLiteral("-1 0 1 0 -1 1 0 0 1");
            break;
        default:
            Q_UNREACHABLE();
            qDebug() << "Weird Rotation, unhandled case.";
    }
    return matrix.split(" ");
}

void XTouchscreen::rotate(KScreen::Output::Rotation rot)
{
    const QString xinput = QStandardPaths::findExecutable("xinput");
//     const QString xinput = QStandardPaths::findExecutable("uname");

    QStringList arguments;
    arguments << QStringLiteral("set-prop")
              << name()
              << QStringLiteral("Coordinate Transformation Matrix")
//               << name()
//               << QStringLiteral("Coordinate Transformation Matrix")
              << transformationMatrix(rot);
//               << QStringLiteral("-1")
//               << QStringLiteral("0")
//               << QStringLiteral("1")
//               << QStringLiteral("0")
//               << QStringLiteral("-1")
//               << QStringLiteral("1")
//               << QStringLiteral("0")
//               << QStringLiteral("0")
//               << QStringLiteral("1");

//               ("-1 0 1 0 -1 1 0 0 1")
//     arguments << QStringLiteral("$DISPLAY");
//     arguments << QStringLiteral("-a");

    qDebug() << "EXECUTE:" << xinput << arguments;

    QProcess *myProcess = new QProcess(this);
    auto env = QProcessEnvironment::systemEnvironment();
//     env.insert(QStringLiteral("DISPLAY"), QStringLiteral(":0"));
    myProcess->setProcessEnvironment(env);
    myProcess->start(xinput, arguments);
    myProcess->waitForFinished();
    auto errors = myProcess->readAllStandardError();
    auto out = myProcess->readAllStandardOutput();
    qDebug() << QString::fromLocal8Bit(errors);
    qDebug() << QString::fromLocal8Bit(out);
}

Output::Rotation XTouchscreen::rotation() const
{
    return d->rotation;
}

void XTouchscreen::setRotation(Output::Rotation rotation)
{
    qDebug() << "setRotation" << rotation;
    if (d->rotation == rotation) {
        return;
    }

    rotate(rotation);
    d->rotation = rotation;

    Q_EMIT rotationChanged();
}
/*
QDebug operator<<(QDebug dbg, const KScreen::XTouchscreenPtr &touchscreen)
{
    if(touchscreen) {
        dbg << "KScreen::XTouchscreen(" << touchscreen->id() << " "
                                  << touchscreen->name()
                                  << touchscreen->transformationMatrix()
                                  << ")";
    } else {
        dbg << "KScreen::XTouchscreen(NULL)";
    }
    return dbg;
}*/
