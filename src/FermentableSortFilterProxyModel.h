/*
 * FermentableSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright Mik
 * Firestone (mikfire@gmail.com), 2010-2011.
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

#ifndef _FermentableSORTFILTERPROXYMODEL_H
#define _FermentableSORTFILTERPROXYMODEL_H

class FermentableSortFilterProxyModel;

#include <QSortFilterProxyModel>

class FermentableSortFilterProxyModel : public QSortFilterProxyModel
{
   Q_OBJECT

public:
   FermentableSortFilterProxyModel(QObject *parent = 0);

protected:
   bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
   QString getName( const QModelIndex &index ) const;
};

#endif
