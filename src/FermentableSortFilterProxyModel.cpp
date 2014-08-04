/*
 * FermentableSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright the following
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

#include "unit.h"
#include "FermentableSortFilterProxyModel.h"
#include "FermentableTableModel.h"
#include "fermentable.h"
#include "brewtarget.h"
#include <iostream>
#include <QDebug>

FermentableSortFilterProxyModel::FermentableSortFilterProxyModel(QObject *parent, bool filt) 
: QSortFilterProxyModel(parent)
{
   filter = filt;
}

bool FermentableSortFilterProxyModel::lessThan(const QModelIndex &left, 
                                         const QModelIndex &right) const
{
    QVariant leftFermentable = sourceModel()->data(left);
    QVariant rightFermentable = sourceModel()->data(right);

   switch( left.column() )
   {
      case FERMAMOUNTCOL:
         // This is a bit twisted. If the numbers are equal, reset the left
         // and right to the names and let it hit the default
         if (Brewtarget::weightQStringToSI(leftFermentable.toString()) == Brewtarget::weightQStringToSI(rightFermentable.toString()))
            return getName(right) < getName(left);
         else
            return Brewtarget::weightQStringToSI(leftFermentable.toString()) < Brewtarget::weightQStringToSI(rightFermentable.toString());
      case FERMYIELDCOL:
         if (leftFermentable.toDouble() == rightFermentable.toDouble() )
            return getName(right) < getName(left);
         else
            return leftFermentable.toDouble() < rightFermentable.toDouble();
      case FERMCOLORCOL:
         if (leftFermentable.toDouble() == rightFermentable.toDouble())
            return getName(right) < getName(left);
         else
            return leftFermentable.toDouble() < rightFermentable.toDouble();
   }

   return leftFermentable.toString() < rightFermentable.toString();
}

QString FermentableSortFilterProxyModel::getName( const QModelIndex &index ) const
{
   QVariant info = sourceModel()->data(QAbstractItemModel::createIndex(index.row(),FERMNAMECOL));
   return info.toString();
}

bool FermentableSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent) const
{
   FermentableTableModel* model = qobject_cast<FermentableTableModel*>(sourceModel());
   return ! filter || model->getFermentable(source_row)->display();
}
