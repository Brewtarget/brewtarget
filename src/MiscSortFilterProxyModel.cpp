/*
 * MiscSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright
 * Philip Greggory Lee <rocketman768@gmail.com, 2011.
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

#include <QAbstractItemModel>
#include "unit.h"
#include "MiscSortFilterProxyModel.h"
#include "MiscTableModel.h"

MiscSortFilterProxyModel::MiscSortFilterProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
}

bool MiscSortFilterProxyModel::lessThan(const QModelIndex &left,
                                        const QModelIndex &right) const
{
   QAbstractItemModel* source = sourceModel();
   QVariant leftMisc, rightMisc;
   if( source )
   {
      //if( left.isValid() )
      leftMisc = source->data(left);
      //else
      //   return true;
      
      //if( right.isValid() )
      rightMisc = source->data(right);
      //else
      //   return true;
   }

   switch( left.column() )
   {
   case MISCAMOUNTCOL:
   case MISCTIMECOL:
      return Unit::qstringToSI(leftMisc.toString()) < Unit::qstringToSI(rightMisc.toString());
    default:
      return leftMisc.toString() < rightMisc.toString();
   }
}


