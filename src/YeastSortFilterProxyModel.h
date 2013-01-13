/*
 * YeastSortFilterProxyModel.cpp is part of Brewtarget, and is Copyright Mik
 * Firestone (mikfire@gmail.com) and Philip G. Lee (rocketman768@gmail.com),
 * 2010-2013.
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

#ifndef _YEASTSORTFILTERPROXYMODEL_H
#define _YEASTSORTFILTERPROXYMODEL_H

class YeastSortFilterProxyModel;

#include <QSortFilterProxyModel>

/*!
 * \class YeastSortFilterProxyModel
 * \author Mik Firestone
 *
 * \brief Proxy model for sorting yeasts.
 */
class YeastSortFilterProxyModel : public QSortFilterProxyModel
{
   Q_OBJECT

public:
   YeastSortFilterProxyModel(QObject *parent = 0, bool filt = true);

protected:
   bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
   bool filterAcceptsRow( int source_row, const QModelIndex &source_parent) const;

private:
   bool filter;
};

#endif
