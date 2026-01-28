/*======================================================================================================================
 * trees/RecipeTreeView.h is part of Brewtarget, and is copyright the following authors 2021-2026:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#ifndef TREES_RECIPETREEVIEW_H
#define TREES_RECIPETREEVIEW_H
#pragma once

#include <QAction>
#include <QMenu>

#include "model/BrewNote.h"
#include "model/Recipe.h"
#include "trees/NamedEntityTreeSortFilterProxyModel.h"
#include "trees/RecipeTreeModel.h"
#include "trees/TreeView.h"
#include "trees/TreeViewBase.h"

class AncestorDialog;
class OptionDialog;

class RecipeTreeView : public TreeView,
                       public TreeViewBase<RecipeTreeView,
                                           RecipeTreeModel,
                                           RecipeTreeSortFilterProxyModel,
                                           MainWindow,  // This is in place of RecipeEditor
                                           Recipe,
                                           BrewNote> {
   Q_OBJECT
   TREE_VIEW_COMMON_DECL(Recipe, BrewNote)

public:
   using TreeViewBase::init;
   void init(AncestorDialog & ancestorDialog, OptionDialog & optionDialog);

   //! \brief returns true if the recipe at ndx is showing its ancestors
   bool ancestorsAreShowing(QModelIndex ndx);

public slots:
   void versionedRecipe(Recipe * descendant);

   void showAncestors();
   void hideAncestors();
   void revertRecipeToPreviousVersion();
   void orphanRecipe();
   void spawnRecipe();

   void brewItHelper();
   void brewAgainHelper();
   void changeBrewDate();
   void fixBrewNote();

signals:
   void recipeSpawn(Recipe * descendant);

};

#endif
