/*======================================================================================================================
 * trees/NamedEntityTreeSortFilterProxyModel.h is part of Brewtarget, and is copyright the following authors 2025:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#ifndef TREES_NAMEDENTITYTREESORTFILTERPROXYMODEL_H
#define TREES_NAMEDENTITYTREESORTFILTERPROXYMODEL_H
#pragma once

#include "trees/NamedEntityTreeModel.h"
#include "trees/RecipeTreeModel.h"
#include "trees/TreeSortFilterProxyModelBase.h"

// See comment in trees/NamedEntityTreeView.h for why we can't remove the repetition below with a macro

class EquipmentTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<EquipmentTreeSortFilterProxyModel, EquipmentTreeModel, Equipment> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Equipment)
};

class FermentableTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<FermentableTreeSortFilterProxyModel, FermentableTreeModel, Fermentable> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Fermentable)
};

class HopTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<HopTreeSortFilterProxyModel, HopTreeModel, Hop> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Hop)
};

class MashTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<MashTreeSortFilterProxyModel, MashTreeModel, Mash> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Mash)
};

class MiscTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<MiscTreeSortFilterProxyModel, MiscTreeModel, Misc> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Misc)
};

class StyleTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<StyleTreeSortFilterProxyModel, StyleTreeModel, Style> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Style)
};

class WaterTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<WaterTreeSortFilterProxyModel, WaterTreeModel, Water> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Water)
};

class YeastTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<YeastTreeSortFilterProxyModel, YeastTreeModel, Yeast> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Yeast)
};

class RecipeTreeSortFilterProxyModel :
   public QSortFilterProxyModel,
   public TreeSortFilterProxyModelBase<RecipeTreeSortFilterProxyModel, RecipeTreeModel, Recipe, BrewNote> {
   Q_OBJECT
   TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(Recipe, BrewNote)
};

#endif
