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
#include "FermentableSortFilterProxyModel.h"
#include "FermentableTableModel.h"
#include <iostream>

FermentableSortFilterProxyModel::FermentableSortFilterProxyModel(QObject *parent) 
: QSortFilterProxyModel(parent)
{
}

bool FermentableSortFilterProxyModel::lessThan(const QModelIndex &left, 
                                         const QModelIndex &right) const
{
    QVariant leftFermentable = sourceModel()->data(left);
    QVariant rightFermentable = sourceModel()->data(right);

   switch( left.column() )
   {
      case FERMAMOUNTCOL:
        return Unit::qstringToSI(leftFermentable.toString()) < Unit::qstringToSI(rightFermentable.toString());
      case FERMYIELDCOL:
        return leftFermentable.toDouble() < rightFermentable.toDouble();
      case FERMCOLORCOL:
        return leftFermentable.toDouble() < rightFermentable.toDouble();
    }

    return leftFermentable.toString() < rightFermentable.toString();
}

