/*
 * StyleSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include "StyleSortFilterProxyModel.h"
#include "StyleListModel.h"
#include "model/Style.h"

StyleSortFilterProxyModel::StyleSortFilterProxyModel(QObject* parent)
   : QSortFilterProxyModel(parent)
{
}

bool StyleSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent) const
{
   StyleListModel* m = qobject_cast<StyleListModel*>(sourceModel());
   if( !m )
      return true;
   Style* s = m->at(source_row);
   if( !s )
      return true;

   return s->display() && !s->deleted();
}
