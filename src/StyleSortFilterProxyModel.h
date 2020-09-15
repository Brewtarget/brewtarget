/*
 * StyleSortFilterProxyModel.h is part of Brewtarget, and is Copyright the following
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

#ifndef _STYLESORTFILTERPROXYMODEL_H
#define _STYLESORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/*!
 * \class StyleSortFilterProxyModel
 * \author Philip G. Lee
 *
 * \brief Proxy model for sorting/filtering Styles.
 * This should really be a base filter for all ingredient models that filters
 * based on the Ingredient::display() field.
 */
class StyleSortFilterProxyModel : public QSortFilterProxyModel
{
   Q_OBJECT
   
   public:
      StyleSortFilterProxyModel(QObject* parent = 0);
      
   protected:
      bool filterAcceptsRow( int source_row, const QModelIndex &source_parent) const;
};

#endif
