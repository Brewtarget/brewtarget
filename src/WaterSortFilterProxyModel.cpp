/*
 * WaterSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#include "WaterSortFilterProxyModel.h"
#include "WaterTableModel.h"
#include "yeast.h"
#include "brewtarget.h"
#include <iostream>

WaterSortFilterProxyModel::WaterSortFilterProxyModel(QObject *parent, bool filt)
: QSortFilterProxyModel(parent),
  filter(filt)
{
}

bool WaterSortFilterProxyModel::lessThan(const QModelIndex &left,
                                         const QModelIndex &right) const
{
    QVariant leftWater = sourceModel()->data(left);
    QVariant rightWater = sourceModel()->data(right);

    return leftWater.toString() < rightWater.toString();
}

/*
bool WaterSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent) const
{
   return !filter;
}
*/
