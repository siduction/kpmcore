/*************************************************************************
 *  Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                 *
 *  Copyright (C) 2016 by Andrius Štikonas <andrius@stikonas.eu>         *
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

#include "fs/ntfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"
#include "util/globallog.h"

#include <KLocalizedString>

#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QUuid>

#include <algorithm>
#include <ctime>

namespace FS
{
FileSystem::CommandSupportType ntfs::m_GetUsed = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_GetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_Create = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_Grow = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_Shrink = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_Move = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_Check = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_Copy = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_Backup = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_SetLabel = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_UpdateUUID = FileSystem::cmdSupportNone;
FileSystem::CommandSupportType ntfs::m_GetUUID = FileSystem::cmdSupportNone;

ntfs::ntfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
    FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Ntfs)
{
}

void ntfs::init()
{
    m_Shrink = m_Grow = m_Check = m_GetUsed = findExternal(QStringLiteral("ntfsresize")) ? cmdSupportFileSystem : cmdSupportNone;
    m_GetLabel = cmdSupportCore;
    m_SetLabel = findExternal(QStringLiteral("ntfslabel")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Create = findExternal(QStringLiteral("mkfs.ntfs")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Copy = findExternal(QStringLiteral("ntfsclone")) ? cmdSupportFileSystem : cmdSupportNone;
    m_Backup = cmdSupportCore;
    m_UpdateUUID = cmdSupportCore;
    m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
    m_GetUUID = cmdSupportCore;
}

bool ntfs::supportToolFound() const
{
    return
        m_GetUsed != cmdSupportNone &&
        m_GetLabel != cmdSupportNone &&
        m_SetLabel != cmdSupportNone &&
        m_Create != cmdSupportNone &&
        m_Check != cmdSupportNone &&
        m_UpdateUUID != cmdSupportNone &&
        m_Grow != cmdSupportNone &&
        m_Shrink != cmdSupportNone &&
        m_Copy != cmdSupportNone &&
        m_Move != cmdSupportNone &&
        m_Backup != cmdSupportNone &&
        m_GetUUID != cmdSupportNone;
}

FileSystem::SupportTool ntfs::supportToolName() const
{
    return SupportTool(QStringLiteral("ntfs-3g"), QUrl(QStringLiteral("http://www.tuxera.com/community/ntfs-3g-download/")));
}

qint64 ntfs::minCapacity() const
{
    return 2 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
}

qint64 ntfs::maxCapacity() const
{
    return 256 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
}

int ntfs::maxLabelLength() const
{
    return 128;
}

qint64 ntfs::readUsedCapacity(const QString& deviceNode) const
{
    ExternalCommand cmd(QStringLiteral("ntfsresize"), { QStringLiteral("--info"), QStringLiteral("--force"), QStringLiteral("--no-progress-bar"), deviceNode });

    if (cmd.run(-1) && cmd.exitCode() == 0) {
        qint64 usedBytes = -1;
        QRegularExpression re(QStringLiteral("resize at (\\d+) bytes"));
        QRegularExpressionMatch reUsedBytes = re.match(cmd.output());

        if (reUsedBytes.hasMatch())
            usedBytes = reUsedBytes.captured(1).toLongLong();

        if (usedBytes > -1)
            return usedBytes;
    }

    return -1;
}

bool ntfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
    ExternalCommand writeCmd(report, QStringLiteral("ntfslabel"), { QStringLiteral("--force"), deviceNode, newLabel });
    writeCmd.setProcessChannelMode(QProcess::SeparateChannels);

    if (!writeCmd.run(-1))
        return false;

    return true;
}

bool ntfs::check(Report& report, const QString& deviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("ntfsresize"), { QStringLiteral("--no-progress-bar"), QStringLiteral("--info"), QStringLiteral("--force"), QStringLiteral("--verbose"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool ntfs::create(Report& report, const QString& deviceNode)
{
    ExternalCommand cmd(report, QStringLiteral("mkfs.ntfs"), { QStringLiteral("--quick"), QStringLiteral("--verbose"), deviceNode });
    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool ntfs::copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const
{
    ExternalCommand cmd(report, QStringLiteral("ntfsclone"), { QStringLiteral("--force"), QStringLiteral("--overwrite"), targetDeviceNode, sourceDeviceNode });

    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool ntfs::resize(Report& report, const QString& deviceNode, qint64 length) const
{
    QStringList args = { QStringLiteral("--no-progress-bar"), QStringLiteral("--force"), deviceNode, QStringLiteral("--size"), QString::number(length) };

    QStringList dryRunArgs = args;
    dryRunArgs << QStringLiteral("--no-action");
    ExternalCommand cmdDryRun(QStringLiteral("ntfsresize"), dryRunArgs);

    if (cmdDryRun.run(-1) && cmdDryRun.exitCode() == 0) {
        ExternalCommand cmd(report, QStringLiteral("ntfsresize"), args);
        return cmd.run(-1) && cmd.exitCode() == 0;
    }

    return false;
}

bool ntfs::updateUUID(Report& report, const QString& deviceNode) const
{
    Q_UNUSED(report);
    ExternalCommand cmd(QStringLiteral("ntfslabel"), { QStringLiteral("--new-serial"), deviceNode });

    return cmd.run(-1) && cmd.exitCode() == 0;
}

bool ntfs::updateBootSector(Report& report, const QString& deviceNode) const
{
    report.line() << xi18nc("@info:progress", "Updating boot sector for NTFS file system on partition <filename>%1</filename>.", deviceNode);

    qint64 n = firstSector();
    char* s = reinterpret_cast<char*>(&n);

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    std::swap(s[0], s[3]);
    std::swap(s[1], s[2]);
#endif

    ExternalCommand cmd(report, QStringLiteral("dd"), { QStringLiteral("of=") + deviceNode , QStringLiteral("bs=1"), QStringLiteral("count=4"), QStringLiteral("seek=28") });

    if (!cmd.start()) {
        Log() << xi18nc("@info:progress", "Could not write new start sector to partition <filename>%1</filename> when trying to update the NTFS boot sector.", deviceNode);
        return false;
    }
    cmd.write(QByteArray(s, sizeof(s)));
    cmd.waitFor(-1);

    // Also update backup NTFS boot sector located at the end of the partition
    // NOTE: this should fail if filesystem does not span the whole partition
    qint64 pos = (lastSector() - firstSector()) * sectorSize() + 28;
    ExternalCommand cmd2(report, QStringLiteral("dd"), { QStringLiteral("of=") + deviceNode , QStringLiteral("bs=1"), QStringLiteral("count=4"), QStringLiteral("seek=") + QString::number(pos) });

    if (!cmd2.start()) {
        Log() << xi18nc("@info:progress", "Could not write new start sector to partition <filename>%1</filename> when trying to update the NTFS boot sector.", deviceNode);
        return false;
    }
    cmd2.write(QByteArray(s, sizeof(s)));
    cmd2.waitFor(-1);

    Log() << xi18nc("@info:progress", "Updated NTFS boot sector for partition <filename>%1</filename> successfully.", deviceNode);

    return true;
}
}