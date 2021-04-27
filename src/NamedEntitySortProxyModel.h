/*
 * NamedEntitySortProxyModel.h is part of Brewtarget, and is Copyright the following
 * authors 2020-2025
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#ifndef NAMEDENTITYSORTPROXYMODEL_H
#define NAMEDENTITYSORTPROXYMODEL_H

#include <QSortFilterProxyModel>
class QAbstractItemModel;

/*!
 * \brief Small wrapper on QSortFilterProxyModel for sorting NamedEntity lists.
 * \author Philip G. Lee
 *
 * Sorts models dynamically based on their properties' default sort behavior.
 */
class NamedEntitySortProxyModel : public QSortFilterProxyModel
{
   Q_OBJECT

public:
   NamedEntitySortProxyModel(QAbstractItemModel* sourceModel = 0);

protected:
   // Can reimplement if we need something fancy in the future.
   //bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif
