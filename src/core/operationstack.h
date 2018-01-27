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

#if !defined(KPMCORE_OPERATIONSTACK_H)

#define KPMCORE_OPERATIONSTACK_H

#include "util/libpartitionmanagerexport.h"

#include <QObject>
#include <QList>
#include <QReadWriteLock>

#include <QtGlobal>

class Device;
class Partition;
class Operation;
class DeviceScanner;

/** The list of Operations the user wants to have performed.

    OperationStack also handles the Devices that were found on this computer and the merging of
    Operations, e.g., when the user first creates a Partition, then deletes it.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT OperationStack : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationStack)

    friend class DeviceScanner;

public:
    typedef QList<Device*> Devices;
    typedef QList<Operation*> Operations;

public:
    OperationStack(QObject* parent = nullptr);
    ~OperationStack();

Q_SIGNALS:
    void operationsChanged();
    void devicesChanged();

public:
    void push(Operation* o);
    void pop();
    bool contains(const Partition* p) const;
    void clearOperations();
    int size() const {
        return operations().size();    /**< @return number of operations */
    }

    Devices& previewDevices() {
        return m_PreviewDevices;    /**< @return the list of Devices */
    }
    const Devices& previewDevices() const {
        return m_PreviewDevices;    /**< @return the list of Devices */
    }

    Operations& operations() {
        return m_Operations;    /**< @return the list of operations */
    }
    const Operations& operations() const {
        return m_Operations;    /**< @return the list of operations */
    }

    Device* findDeviceForPartition(const Partition* p);

    QReadWriteLock& lock() {
        return m_Lock;
    }

protected:
    void clearDevices();
    void addDevice(Device* d);
    void sortDevices();

    bool mergeNewOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergeCopyOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergeRestoreOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergePartFlagsOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergePartLabelOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergeCreatePartitionTableOperation(Operation*& currentOp, Operation*& pushedOp);
    bool mergeResizeVolumeGroupResizeOperation(Operation*& pushedOp);

private:
    Operations m_Operations;
    mutable Devices m_PreviewDevices;
    QReadWriteLock m_Lock;
};

#endif
