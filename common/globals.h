/*
SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
**/
#ifndef COMMON_GLOBALS_H
#define COMMON_GLOBALS_H

#include <QString>

namespace Globals
{
void setDirPath(const QString &path);
QString dirPath();
}

#endif
