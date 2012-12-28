/*
 * BtTreeFilterProxyModel.cpp is part of Brewtarget, and is Copyright Mik
 * Firestone (mikfire@gmail.com) and Philip G. Lee (rocketman768@gmail.com),
 * 2010-2012.
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

#ifndef _BTTREEFILTERPROXYMODEL_H
#define _BTTREEFILTERPROXYMODEL_H

class BtTreeFilterProxyModel;

#include <QSortFilterProxyModel>

#include "BrewTargetTreeModel.h"
#include "BrewTargetTreeView.h"
#include "BrewTargetTreeItem.h"
#include "recipe.h"
#include "equipment.h"
#include "fermentable.h"
#include "misc.h"
#include "hop.h"
#include "yeast.h"
#include "style.h"
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
   BtTreeFilterProxyModel(QObject *parent, BrewTargetTreeModel::TypeMasks mask);

protected:
   bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
   bool filterAcceptsRow( int source_row, const QModelIndex &source_parent) const;

private:
   BrewTargetTreeModel::TypeMasks treeMask;

   bool lessThanRecipe(BrewTargetTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanEquip(BrewTargetTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanFerment(BrewTargetTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanMisc(BrewTargetTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanHop(BrewTargetTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
   bool lessThanYeast(BrewTargetTreeModel* model,const QModelIndex &left, const QModelIndex &right) const;
};

#endif
