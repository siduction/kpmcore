/*************************************************************************
 *  Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                      *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#include "fs/unknown.h"

namespace FS
{
unknown::unknown(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Type::Unknown)
{
}

bool unknown::canMount(const QString & deviceNode, const QString & mountPoint) const
{
    Q_UNUSED(deviceNode)
    Q_UNUSED(mountPoint)
    return false;
}
}
