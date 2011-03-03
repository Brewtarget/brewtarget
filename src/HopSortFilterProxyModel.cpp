/*
 * YeastSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright Mik
 * Firestone (mikfire@gmail.com), 2010.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "unit.h"
#include "HopSortFilterProxyModel.h"
#include "HopTableModel.h"
#include <iostream>
HopSortFilterProxyModel::HopSortFilterProxyModel(QObject *parent) 
: QSortFilterProxyModel(parent)
{
}

bool HopSortFilterProxyModel::lessThan(const QModelIndex &left, 
                                         const QModelIndex &right) const
{
    QVariant leftHop = sourceModel()->data(left);
    QVariant rightHop = sourceModel()->data(right);

   switch( left.column() )
   {
      case HOPALPHACOL:
        return leftHop.toDouble() < rightHop.toDouble();
      case HOPAMOUNTCOL:
      case HOPTIMECOL:
        return Unit::qstringToSI(leftHop.toString()) < Unit::qstringToSI(rightHop.toString());
    }

    return leftHop.toString() < rightHop.toString();
}


