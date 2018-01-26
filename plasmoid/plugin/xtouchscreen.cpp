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
// #include "../../src/debug_p.h"

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

QString XTouchscreen::transformationMatrix() const
{
    return d->transformationMatrix;
}

void XTouchscreen::setTransformationMatrix(const QString& matrix)
{
    if (d->transformationMatrix == matrix) {
        return;
    }

    d->transformationMatrix = matrix;

    Q_EMIT transformationMatrixChanged();
}

Output::Rotation XTouchscreen::rotation() const
{
    return d->rotation;
}

void XTouchscreen::setRotation(Output::Rotation rotation)
{
    if (d->rotation == rotation) {
        return;
    }

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
