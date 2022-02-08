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
#include "MiscSortFilterProxyModel.h"

#include <QAbstractItemModel>

#include "measurement/Measurement.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"
#include "model/Misc.h"
#include "tableModels/MiscTableModel.h"

MiscSortFilterProxyModel::MiscSortFilterProxyModel(QObject *parent, bool filt)
: QSortFilterProxyModel(parent)
{
   filter = filt;
}

bool MiscSortFilterProxyModel::lessThan(const QModelIndex &left,
                                        const QModelIndex &right) const {
   QAbstractItemModel* source = sourceModel();
   QVariant leftMisc, rightMisc;
   if (source) {
      leftMisc = source->data(left);
      rightMisc = source->data(right);
   }

   switch (left.column()) {
       case MISCINVENTORYCOL:
         if (Measurement::qStringToSI(leftMisc.toString(), Measurement::PhysicalQuantity::Mass).quantity == 0.0 &&
             this->sortOrder() == Qt::AscendingOrder) {
            return false;
         }
         return (Measurement::qStringToSI(leftMisc.toString(), Measurement::PhysicalQuantity::Mass) <
                 Measurement::qStringToSI(rightMisc.toString(), Measurement::PhysicalQuantity::Mass));

      case MISCAMOUNTCOL:
         return (Measurement::qStringToSI(leftMisc.toString(), Measurement::PhysicalQuantity::Mass) <
                 Measurement::qStringToSI(rightMisc.toString(), Measurement::PhysicalQuantity::Mass));

      case MISCTIMECOL:
         return (Measurement::qStringToSI(leftMisc.toString(), Measurement::PhysicalQuantity::Time) <
                 Measurement::qStringToSI(rightMisc.toString(), Measurement::PhysicalQuantity::Time));

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
