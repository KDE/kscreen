/********************************************************************
Copyright 2019 Roman Gilg <subdiff@gmail.com>

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
#ifndef COMMON_CONTROL_H
#define COMMON_CONTROL_H

#include <kscreen/types.h>

#include <QVariantMap>

class Control
{
public:
    enum class OutputRetention {
        Undefined = -1,
        Global = 0,
        Individual = 1,
    };

    static QMap<QString, OutputRetention> readInOutputRetentionValues(const QString &configId);
    static OutputRetention getOutputRetention(const QString &outputId, const QMap<QString, OutputRetention> &retentions);

    static QString configFilePath(const QString &hash);
    static QString outputFilePath(const QString &hash);

private:
    static QString dirPath();
    static OutputRetention convertVariantToOutputRetention(QVariant variant);

    static QString s_dirName;
};

#endif
