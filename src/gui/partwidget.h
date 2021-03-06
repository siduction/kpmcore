/*************************************************************************
 *  Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                *
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

#if !defined(KPMCORE_PARTWIDGET_H)

#define KPMCORE_PARTWIDGET_H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"
#include "partwidgetbase.h"

#include <QColor>

class Partition;

class QPaintEvent;
class QResizeEvent;

/** Widget that represents a Partition.

    Represents a single Partition (possibly with its children, in case of an extended Partition) in the GUI.

    @author Volker Lanz <vl@fidra.de>
*/
class LIBKPMCORE_EXPORT PartWidget : public PartWidgetBase
{
    Q_OBJECT

public:
    explicit PartWidget(QWidget* parent, Partition* p = nullptr);

    void init(Partition* p);
    void setActive(bool b) {
        m_Active = b;
    }
    bool isActive() const {
        return m_Active;    /**< @return true if this is the currently active widget */
    }
    void updateChildren();

    Partition* partition() {
        return m_Partition;    /**< @return the widget's Partition */
    }

    const Partition* partition() const {
        return m_Partition;    /**< @return the widget's Partition */
    }

    void setFileSystemColorCode( const std::vector<QColor>& colorCode );

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

    QColor activeColor(const QColor& col) const;

    void drawGradient(QPainter* painter, const QColor& color, const QRect& rect, bool active = false) const;

private:
    Partition* m_Partition;
    bool m_Active;
    std::vector<QColor> m_fileSystemColorCode;
};

#endif

