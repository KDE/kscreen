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

#ifndef XTOUCHSCREEN_H
#define XTOUCHSCREEN_H

// #include "../../src/kscreen_export.h"
// #include "../../src/types.h"

#include <KScreen/Output>

#include <QObject>
#include <QLoggingCategory>

namespace KScreen {

class XTouchscreen : public QObject
{
    Q_OBJECT

    public:
        Q_ENUMS(Rotation)
        Q_PROPERTY(int id READ id CONSTANT)
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
        Q_PROPERTY(Output::Rotation rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
        //Q_PROPERTY(QString transformationMatrix READ transformationMatrix WRITE setTransformationMatrix NOTIFY transformationMatrixChanged)

        explicit XTouchscreen(QObject* parent = nullptr);
        virtual ~XTouchscreen();

        //XTouchscreenPtr clone() const;

        int id() const;
        void setId(int id);

        QString name() const;
        void setName(const QString& name);

        Output::Rotation rotation() const;
        void setRotation(Output::Rotation rotation);

        static QString transformationMatrix(KScreen::Output::Rotation rot);
        void rotate(KScreen::Output::Rotation rot);

Q_SIGNALS:
        void nameChanged() const;
        void rotationChanged() const;
        void transformationMatrixChanged() const;

    private:

        Q_DISABLE_COPY(XTouchscreen)

        class Private;
        Private * const d = nullptr;

        XTouchscreen(Private *dd);
};

} //KScreen namespace

//KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::XTouchscreenPtr &output);

//Q_DECLARE_METATYPE(KScreen::XTouchscreenList)
// Q_DECLARE_METATYPE(KScreen::XTouchscreen::Rotation)

#endif // XTOUCHSCREEN_H
