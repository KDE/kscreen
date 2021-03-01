/*
SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
**/
#include "globals.h"

#include <QStandardPaths>

namespace Globals
{
QString s_dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % QStringLiteral("/kscreen/");

QString dirPath()
{
    return s_dirPath;
}

void setDirPath(const QString &path)
{
    s_dirPath = path;
    if (!s_dirPath.endsWith(QLatin1Char('/'))) {
        s_dirPath += QLatin1Char('/');
    }
}

}
