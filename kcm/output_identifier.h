/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#pragma once

#include <kscreen/config.h>

#include <QVector>

namespace PlasmaQuick
{
class Dialog;
}

class OutputIdentifier : public QObject
{
    Q_OBJECT

public:
    explicit OutputIdentifier(KScreen::ConfigPtr config, QObject *parent = nullptr);
    ~OutputIdentifier() override;

Q_SIGNALS:
    void identifiersFinished();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QVector<PlasmaQuick::Dialog *> m_views;
};
