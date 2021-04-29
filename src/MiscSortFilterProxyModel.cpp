/*
 * MiscSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QAbstractItemModel>
#include "MiscSortFilterProxyModel.h"
#include "MiscTableModel.h"
#include "model/Misc.h"
#include "brewtarget.h"
#include "unit.h"

MiscSortFilterProxyModel::MiscSortFilterProxyModel(QObject *parent, bool filt)
: QSortFilterProxyModel(parent)
{
   filter = filt;
}

bool MiscSortFilterProxyModel::lessThan(const QModelIndex &left,
                                        const QModelIndex &right) const
{
   QAbstractItemModel* source = sourceModel();
   QVariant leftMisc, rightMisc;
   if( source )
   {
      leftMisc = source->data(left);
      rightMisc = source->data(right);
   }

   switch( left.column() )
   {
   case MISCINVENTORYCOL:
         if (Brewtarget::qStringToSI(leftMisc.toString(), Units::kilograms) == 0.0 && this->sortOrder() == Qt::AscendingOrder)
            return false;
         else
            return Brewtarget::qStringToSI(leftMisc.toString(), Units::kilograms) < Brewtarget::qStringToSI(rightMisc.toString(), Units::kilograms);
   case MISCAMOUNTCOL:
         return Brewtarget::qStringToSI(leftMisc.toString(), Units::kilograms) < Brewtarget::qStringToSI(rightMisc.toString(), Units::kilograms);
   case MISCTIMECOL:
      return Brewtarget::qStringToSI(leftMisc.toString(), Units::minutes) < Brewtarget::qStringToSI(rightMisc.toString(), Units::minutes);
    default:
      return leftMisc.toString() < rightMisc.toString();
   }
}


bool MiscSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent) const
{
   MiscTableModel* model = qobject_cast<MiscTableModel*>(sourceModel());
   QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

   return !filter
          ||
          (  sourceModel()->data(index).toString().contains(filterRegExp())
             && model->getMisc(source_row)->display()
          );
}
