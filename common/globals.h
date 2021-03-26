/********************************************************************
Copyright 2018 Roman Gilg <subdiff@gmail.com>
Copyright 2021 David Redondo <kde@david-redondo.de>

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
#ifndef COMMON_GLOBALS_H
#define COMMON_GLOBALS_H

#include <QString>

namespace Globals
{
QString dirPath();
/**
 * Tries to find the specified file realtive to dirPath(). Also considers presets if there is no
 * existing file under dirPath() yet.
 * @returns The abosolute path to a matching file if on exists or an empty string
 */
QString findFile(const QString &filePath);
}

#endif
