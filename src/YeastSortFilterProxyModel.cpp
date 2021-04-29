/*
 * YeastSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright the following
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

#include "YeastSortFilterProxyModel.h"
#include "YeastTableModel.h"
#include "model/Yeast.h"
#include "brewtarget.h"
#include <iostream>

YeastSortFilterProxyModel::YeastSortFilterProxyModel(QObject *parent, bool filt)
: QSortFilterProxyModel(parent)
{
   filter = filt;
}

bool YeastSortFilterProxyModel::lessThan(const QModelIndex &left,
                                         const QModelIndex &right) const
{
    QVariant leftYeast = sourceModel()->data(left);
    QVariant rightYeast = sourceModel()->data(right);
    Unit* unit = Units::liters;
    double lAmt, rAmt;

    switch( left.column() )
    {
    case YEASTINVENTORYCOL:
      if (Brewtarget::qStringToSI(leftYeast.toString(),unit) == 0.0 && this->sortOrder() == Qt::AscendingOrder)
         return false;
      else
         return Brewtarget::qStringToSI(leftYeast.toString(),unit) < Brewtarget::qStringToSI(rightYeast.toString(),unit);
       // This is a lie. I need to figure out if they are weights or volumes.
       // and then figure some reasonable way to compare weights to volumes.
       // Maybe lying isn't such a bad idea
    case YEASTAMOUNTCOL:
      return Brewtarget::qStringToSI(leftYeast.toString(),unit) < Brewtarget::qStringToSI(rightYeast.toString(),unit);
    case YEASTPRODIDCOL:
      lAmt = Brewtarget::toDouble( leftYeast.toString(), "YeastSortFilterProxyModel::lessThan");
      rAmt = Brewtarget::toDouble( rightYeast.toString(), "YeastSortFilterProxyModel::lessThan");
      return lAmt < rAmt;
    default:
      return leftYeast.toString() < rightYeast.toString();
    }
}

bool YeastSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent) const
{
   YeastTableModel* model = qobject_cast<YeastTableModel*>(sourceModel());
   QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

   return !filter
          ||
           ( sourceModel()->data(index).toString().contains(filterRegExp())
             && model->getYeast(source_row)->display()
           );
}
