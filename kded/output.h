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
#ifndef KDED_OUTPUT_H
#define KDED_OUTPUT_H

#include <kscreen/types.h>

#include <QVariantMap>

class Output
{
public:
    static void readInOutputs(KScreen::OutputList outputs, const QVariantList &outputsInfo);

    static void writeGlobal(const KScreen::OutputPtr &output);
    static bool writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info);

    static QString dirPath();

private:
    static QString globalFileName(const QString &hash);
    static QVariantMap getGlobalData(KScreen::OutputPtr output);

    static void readIn(KScreen::OutputPtr output, const QVariantMap &info);
    static bool readInGlobal(KScreen::OutputPtr output);
    static void readInGlobalPartFromInfo(KScreen::OutputPtr output, const QVariantMap &info);

    static QString s_dirName;
};

#endif
