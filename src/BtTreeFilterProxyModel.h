/*
 * BtTreeFilterProxyModel.h is part of Brewtarget, and is Copyright the following
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

#ifndef _BTTREEFILTERPROXYMODEL_H
#define _BTTREEFILTERPROXYMODEL_H

class BtTreeFilterProxyModel;

#include <QSortFilterProxyModel>

#include "BtFolder.h"
#include "BtTreeModel.h"
#include "BtTreeView.h"
#include "BtTreeItem.h"
#include "recipe.h"
#include "equipment.h"
#include "fermentable.h"
#include "misc.h"
#include "hop.h"
#include "yeast.h"
#include "style.h"
#include "water.h"

/*!
 * \class BtTreeFilterProxyModel
 * \author Mik Firestone
 * \author Philip G. Lee
 *
 * \brief Proxy model for sorting brewtarget trees.
 */
class BtTreeFilterProxyModel : public QSortFilterProxyModel
{
   Q_OBJECT

public:
   BtTreeFilterProxyModel(QObject *parent, BtTreeModel::TypeMasks mask);

protected:
   bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
   bool filterAcceptsRow( int source_row, const QModelIndex &source_parent) const;

private:
   BtTreeModel::TypeMasks treeMask;

   bool lessThanRecipe(BtTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanEquip(BtTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanFerment(BtTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanMisc(BtTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanHop(BtTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanYeast(BtTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanStyle(BtTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanWater(BtTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
};

#endif
