/*======================================================================================================================
 * trees/RecipeTreeModel.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#ifndef TREES_RECIPETREEMODEL_H
#define TREES_RECIPETREEMODEL_H
#pragma once

#include <QMenu>

#include "model/BrewNote.h"
#include "model/Recipe.h"
#include "trees/TreeModel.h"
#include "trees/TreeModelBase.h"

class AncestorDialog;
class OptionDialog;

class RecipeTreeModel : public TreeModel,
                        public TreeModelBase<RecipeTreeModel, Recipe, BrewNote> {
   Q_OBJECT
   TREE_MODEL_COMMON_DECL(Recipe, BrewNote)

public:

   void init(AncestorDialog & ancestorDialog, OptionDialog & optionDialog);

   //! \b show the versions of the recipe at index
   void showAncestors(QModelIndex ndx);
   //! \b show the child of the recipe at index
   bool showChild(QModelIndex ndx) const;
   //! \b hide the ancestors
   void hideAncestors(QModelIndex ndx);
   void revertRecipeToPreviousVersion(QModelIndex ndx);
   //! \b orphan a recipe
   void orphanRecipe(QModelIndex ndx);
   //! \b spawns a recipe
   void spawnRecipe(QModelIndex ndx);

public slots:
   void versionedRecipe(Recipe * ancestor, Recipe * descendant);
   void showOrHideAllAncestors(bool show);

protected slots:
   void recipePropertyChanged(int recipeId, BtStringConst const & propertyName);

signals:
   void recipeSpawn(Recipe * descendant);

protected:
private:
   //! \b flip the switch to show descendants
   void setShowChild(QModelIndex child, bool val);

   //! \brief convenience function to add brewnotes to a recipe as a subtree
   void addBrewNoteSubTree(TreeNode & recipeNodeRaw,
                           int recipeChildNumber,
                           Recipe const & recipe,
                           bool recurse = true);

   void addAncestoralTree(TreeNode & recipeNode,
                          int recipeChildNumber,
                          Recipe const & recipe);

   /**
    * \brief Add BrewNotes and ancestors to a recipe as a sub-tree
    * \param recurse
    */
   void addSubTree(Recipe const & recipe,
                   TreeItemNode<Recipe> & recipeNode,
                   bool const recurse = true);

};

#endif
