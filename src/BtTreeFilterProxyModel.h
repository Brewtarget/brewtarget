/*
 * BtTreeFilterProxyModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef BTTREEFILTERPROXYMODEL_H
#define BTTREEFILTERPROXYMODEL_H
#pragma once

#include <QModelIndex>
#include <QSortFilterProxyModel>

#include "BtTreeModel.h"

/*!
 * \class BtTreeFilterProxyModel
 *
 * \brief Proxy model for sorting brewtarget trees.
 */
class BtTreeFilterProxyModel : public QSortFilterProxyModel {
   Q_OBJECT

public:
   BtTreeFilterProxyModel(QObject *parent, BtTreeModel::TypeMasks mask);

protected:
   bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
   bool filterAcceptsRow( int source_row, const QModelIndex &source_parent) const;

private:
   BtTreeModel::TypeMasks treeMask;
};

#endif
