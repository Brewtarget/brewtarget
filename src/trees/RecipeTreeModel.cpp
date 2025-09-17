/*======================================================================================================================
 * trees/RecipeTreeModel.cpp is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#include "trees/RecipeTreeModel.h"

#include "model/BrewNote.h"
#include "AncestorDialog.h"
#include "OptionDialog.h"
#include "PersistentSettings.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeTreeModel.cpp"
#endif

TREE_MODEL_COMMON_CODE(Recipe, BrewNote)


void RecipeTreeModel::init(AncestorDialog & ancestorDialog, OptionDialog & optionDialog) {
   // This is how we get notified of recipe versioning changes
   connect(&ObjectStoreTyped<Recipe>::getInstance(),
           &ObjectStoreTyped<Recipe>::signalPropertyChanged,
           this,
           &RecipeTreeModel::recipePropertyChanged);

   connect(&ancestorDialog, &AncestorDialog::ancestoryChanged, this, &RecipeTreeModel::versionedRecipe);
   connect(&optionDialog  ,   &OptionDialog::showAllAncestors, this, &RecipeTreeModel::showOrHideAllAncestors );

   return;
}

void RecipeTreeModel::addBrewNoteSubTree(TreeNode & recipeNodeRaw,
                                         int recipeChildNumber,
                                         Recipe const & recipe,
                                         bool recurse) {
   auto const brewNotes = recurse ? RecipeHelper::brewNotesForRecipeAndAncestors(recipe) : recipe.brewNotes();

   // It's a coding error to call this for a non-Recipe node...
   Q_ASSERT(recipeNodeRaw.classifier() == TreeNodeClassifier::PrimaryItem);
   // ...so this cast should be safe.
   auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(recipeNodeRaw);

   // Normally leave the next line commented out as it generates quite a bit of logging
//   qDebug() << Q_FUNC_INFO << "Adding" << brewNotes.size() << "BrewNotes for" << recipe;
   int childNum = recipeNode.childCount();
   for (auto brewNote : brewNotes) {
      //
      // You might think we should just set recipeNodeIndex outside the loop or even pass it into this function as a
      // parameter (because the caller probably has it already).  This would probably work, at least in current versions
      // of Qt, BUT, strictly speaking, every time we add or remove something to or from the tree, we have to assume
      // indexes from before the insertion/deletion are no longer valid.  (See
      // https://doc.qt.io/qt-6/qmodelindex.html#details where it says "Model indexes should be used immediately and
      // then discarded. You should not rely on indexes to remain valid after calling model functions that change the
      // structure of the model or delete items.")
      //
      QModelIndex recipeNodeIndex = this->createIndex(recipeChildNumber, 0, &recipeNodeRaw);
      // In previous insert loops, we ignore the error and soldier on. So we
      // will do that here too
      if (!this->insertChild(recipeNode, recipeNodeIndex, childNum, brewNote)) {
         qWarning() << Q_FUNC_INFO << "BrewNote insert failed";
         continue;
      }
      this->observeElement(brewNote);
      ++childNum;
   }
   return;
}

void RecipeTreeModel::addAncestoralTree(TreeNode & recipeNodeRaw,
                                        int recipeChildNumber,
                                        Recipe const & recipe) {
   auto ancestors = recipe.ancestors();

   // It's a coding error to call this for a non-Recipe node...
   Q_ASSERT(recipeNodeRaw.classifier() == TreeNodeClassifier::PrimaryItem);
   // ...so this cast should be safe.
   auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(recipeNodeRaw);

   // Now loop through the ancestors. The nature of the beast is nearest ancestors are first
   qDebug() << Q_FUNC_INFO << "Adding" << ancestors.size() << "ancestors for" << recipe;
   int childNum = recipeNode.childCount();
   for (auto ancestor : ancestors) {
      // Comment in addBrewNoteSubTree() about indexes also applies here
      QModelIndex recipeNodeIndex = this->createIndex(recipeChildNumber, 0, &recipeNode);
      if (!this->insertChild(static_cast<TreeItemNode<Recipe> &>(recipeNode), recipeNodeIndex, childNum, ancestor)) {
         qWarning() << Q_FUNC_INFO << "Ancestor insert failed";
         continue;
      }
      // We need the index of what we just inserted, which is a "child" in the tree and an "ancestor" of the recipe
      QModelIndex ancestorIndex = this->findElement(ancestor.get(), &recipeNode);
      this->setShowChild(ancestorIndex, true);

      TreeNode * ancestorNode = this->treeNode(ancestorIndex);

      // finally, add this ancestor's brewnotes but do not recurse
      this->addBrewNoteSubTree(*ancestorNode, childNum, *ancestor, false);
      this->observeElement(ancestor);
      ++childNum;
   }
   return;
}

void RecipeTreeModel::addSubTree(Recipe const & recipe,
                                 TreeItemNode<Recipe> & recipeNode,
                                 bool const recurse) {

   bool const showSnapshots = PersistentSettings::value(PersistentSettings::Names::showsnapshots, false).toBool();

   QModelIndex recipeNodeIndex = this->indexOfNode(&recipeNode);
   TreeNode * recipeParent = recipeNode.rawParent();
   qDebug() <<
      Q_FUNC_INFO << "recipe:" << recipe << ", recipeNode:" << recipeNode << ", recipeParent:" << recipeParent;
   QModelIndex recipeParentIndex = this->derived().parent(recipeNodeIndex);

   int const childNumber = recipeNode.childNumber();
   if (showSnapshots && recipe.hasAncestors()) {
      this->setShowChild(recipeParentIndex, true);
      this->addAncestoralTree (recipeNode, childNumber, recipe);
      this->addBrewNoteSubTree(recipeNode, childNumber, recipe, false);
   } else {
      this->addBrewNoteSubTree(recipeNode, childNumber, recipe, true);
   }

   return;
}

bool RecipeTreeModel::showChild(QModelIndex child) const {
   TreeNode * node = this->treeNode(child);
   return node->showMe();
}

void RecipeTreeModel::setShowChild(QModelIndex child, bool val) {
   TreeNode * node = this->treeNode(child);
   return node->setShowMe(val);
}

void RecipeTreeModel::showAncestors(QModelIndex recipeNodeIndex) {

   if (!recipeNodeIndex.isValid()) {
      return;
   }

   TreeNode * node = this->treeNode(recipeNodeIndex);
   if (node->classifier() != TreeNodeClassifier::PrimaryItem) {
      // This is probably a coding error
      qWarning() << Q_FUNC_INFO << "Unexpected node type" << static_cast<int>(node->classifier());
      return;
   }

   auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(*node);

   Recipe & recipe = *(recipeNode.underlyingItem());

   this->removeChildren(0, node->childCount(), recipeNodeIndex);

   int const childNumber = recipeNode.childNumber(); // == recipeNodeIndex.row()

   // add the brewnotes for this version back
   this->addBrewNoteSubTree(*node, childNumber, recipe, false);

   // Although it would probably still work, strictly speaking, recipeNodeIndex is no longer valid because we made a
   // modification to the tree structure, so we should reset it.
   recipeNodeIndex = this->findElement(&recipe, recipeNode.rawParent());

   // set showChild on the leaf node. I use this for drawing menus
   this->setShowChild(recipeNodeIndex, true);

   this->addAncestoralTree(recipeNode, childNumber, recipe);

   return;
}

void RecipeTreeModel::hideAncestors(QModelIndex index) {
   // This has no potential to be clever. None.
   if (!index.isValid()) {
      return;
   }

   TreeNode * node = this->treeNode(index);

   // It's a coding error to call this for a non-Recipe node...
   Q_ASSERT(node->classifier() == TreeNodeClassifier::PrimaryItem);
   // ...so this cast should be safe.
   auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(*node);

   // remove all the currently shown children
   this->removeChildren(0, recipeNode.childCount(), index);
   auto descendant = recipeNode.underlyingItem();

   // put the brewnotes back, including those from the ancestors.
   this->addBrewNoteSubTree(*node, index.row(), *descendant);

   // This is for menus
   this->setShowChild(index, false);

   // Now we just need to mark each ancestor invisible again
   for (auto ancestor : descendant->ancestors()) {
      QModelIndex aIndex = this->findElement(ancestor.get(), node);
      this->setShowChild(aIndex, false);
      emit dataChanged(aIndex, aIndex);
   }
   return;
}

// more cleverness must happen. Wonder if I can figure it out.
void RecipeTreeModel::showOrHideAllAncestors(bool show) {
   for (auto recipe : ObjectStoreWrapper::getAllDisplayable<Recipe>()) {
      if (recipe->hasAncestors()) {
         QModelIndex recipeIndex = this->findElement(recipe.get());
         if (show) {
            this->showAncestors(recipeIndex);
         } else {
            this->hideAncestors(recipeIndex);
         }
      }
   }
   return;
}

void RecipeTreeModel::recipePropertyChanged(int recipeId, BtStringConst const & propertyName) {
   // If a Recipe's ancestor ID has changed then it might be because a new ancestor has been created
   // .:TBD:. We could probably get away with propertyName == PropertyNames::Recipe::ancestorId here because
   // we always use the same constants for property names.
   if (propertyName != PropertyNames::Recipe::ancestorId) {
      qDebug() << Q_FUNC_INFO << "Ignoring change to" << propertyName << "on Recipe" << recipeId;
      return;
   }

   Recipe * descendant = ObjectStoreWrapper::getByIdRaw<Recipe>(recipeId);
   if (!descendant) {
      qCritical() << Q_FUNC_INFO << "Unable to find Recipe" << recipeId << "to check its change to" << propertyName;
      return;
   }

   int ancestorId = descendant->getAncestorId();

   if (ancestorId <= 0 || ancestorId == descendant->key()) {
      qDebug() << Q_FUNC_INFO << "No ancestor (" << ancestorId << ") on Recipe" << recipeId;
      return;
   }

   Recipe * ancestor = ObjectStoreWrapper::getByIdRaw<Recipe>(ancestorId);
   if (!ancestor) {
      qCritical() << Q_FUNC_INFO << "Unable to find Recipe" << ancestorId << "set as ancestor to Recipe" << recipeId;
      return;
   }

   this->versionedRecipe(ancestor, descendant);

   return;
}

void RecipeTreeModel::revertRecipeToPreviousVersion(QModelIndex index) {
   TreeNode * node = this->treeNode(index);
   QModelIndex pIndex = this->parent(index);

   // It's a coding error if the index supplied doesn't point to a Recipe node...
   Q_ASSERT(node->classifier() == TreeNodeClassifier::PrimaryItem);
   // ...so this cast should be safe.
   auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(*node);

   // The recipe referred to by the index is the one that's about to be deleted
   auto recipeToRevert = recipeNode.underlyingItem();

   auto ancestor = recipeToRevert->revertToPreviousVersion();

   // don't do anything if there is nothing to do
   if (!ancestor) {
      return;
   }

   // Remove all the rows associated with the about-to-be-deleted Recipe
   this->removeChildren(0, node->childCount(), index);

   // Put the ancestor into the tree
   auto ancestorPointer = ObjectStoreWrapper::getShared(*ancestor);
   if (!this->insertChild(pIndex, pIndex.row(), ancestorPointer)) {
      qWarning() << Q_FUNC_INFO << "Could not add ancestor to tree";
   }

   // Find the ancestor in the tree
   QModelIndex ancestorIndex = this->findElement(ancestorPointer.get());
   if (!ancestorIndex.isValid()) {
      qWarning() << Q_FUNC_INFO << "Couldn't find the ancestor";
   }

   TreeNode * ancestorNode = this->treeNode(ancestorIndex);

   // Add the ancestor's brewnotes to the descendant
   this->addBrewNoteSubTree(*ancestorNode, ancestorIndex.row(), *ancestor);

   return;
}

// This is detaching a Recipe from its previous versions
void RecipeTreeModel::orphanRecipe(QModelIndex index) {
   TreeNode* node = this->treeNode(index);
   //TreeNode* parentNode = node->rawParent();
   QModelIndex pIndex = this->parent(index);

   // It's a coding error if the index supplied doesn't point to a Recipe node...
   Q_ASSERT(node->classifier() == TreeNodeClassifier::PrimaryItem);
   // ...so this cast should be safe.
   auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(*node);

   // I need the recipe referred to by the index
   auto orphan = recipeNode.underlyingItem();

   // don't do anything if there is nothing to do
   if (!orphan->hasAncestors()) {
      return;
   }

   // And I need its immediate ancestor
   auto ancestor = orphan->ancestors().at(0);

   // Deal with the soon-to-be orphan first
   // Remove all the rows associated with the orphan
   this->removeChildren(0, node->childCount(), index);

   // This looks weird, but I think it will do what I need -- set
   // the ancestor_id to itself and reload the ancestors array. setAncestor
   // handles the locked and display flags
   orphan->setAncestor(*orphan);
   // Display all of its brewnotes
   this->addBrewNoteSubTree(*node, index.row(), *orphan, false);

   ancestor->setLocked(false);

   // Put the ancestor into the tree
   if (!this->insertChild(pIndex, pIndex.row(), ancestor)) {
      qWarning() << Q_FUNC_INFO << "Could not add ancestor to tree";
   }

   // Find the ancestor in the tree
   QModelIndex ancestorIndex = findElement(ancestor.get());
   if (!ancestorIndex.isValid()) {
      qWarning() << Q_FUNC_INFO << "Couldn't find the ancestor";
   }

   TreeNode * ancestorNode = this->treeNode(ancestorIndex);

   // Add the ancestor's brewnotes to the descendant
   this->addBrewNoteSubTree(*ancestorNode, ancestorIndex.row(), *ancestor);

   return;
}

void RecipeTreeModel::spawnRecipe(QModelIndex index) {
   Q_ASSERT(index.isValid());

   TreeNode * node = this->treeNode(index);
   // It's a coding error if the index supplied doesn't point to a Recipe node...
   Q_ASSERT(node->classifier() == TreeNodeClassifier::PrimaryItem);
   // ...so this cast should be safe.
   auto & ancestorNode = static_cast<TreeItemNode<Recipe> &>(*node);

   Recipe * ancestor = ancestorNode.underlyingItem().get();
   Q_ASSERT(ancestor);
   std::shared_ptr<Recipe> descendant = std::make_shared<Recipe>(*ancestor);
   //
   // We want to store the new Recipe first so that it gets an ID.
   //
   ObjectStoreWrapper::insert<Recipe>(descendant);
   //
   // The call above to ObjectStoreWrapper::insert will trigger the signal
   // ObjectStoreTyped<Recipe>::signalObjectInserted, which invokes the RecipeTreeModel::elementAdded slot, which calls
   // TreeModelBase::doElementAdded and adds the new Recipe (descendant) to the tree -- but without any ancestors
   // (because we haven't yet set the ancestor).
   //
   qDebug() <<
      Q_FUNC_INFO << "Created descendant Recipe" << descendant->key() << "of Recipe" << ancestor->key();

   //
   // Now we stored the new recipe, we can connect it to the one it was copied from
   //
   descendant->setAncestor(*ancestor);
   //
   // The call above to setAncestor will trigger a ObjectStoreTyped<Recipe>::signalPropertyChanged signal, which is
   // invokes the RecipeTreeModel::recipePropertyChanged slot, which will have called RecipeTreeModel::versionedRecipe.
   // In turn, that will have removed the ancestor from the tree, and added the ancestor(s) as one of the descendant's
   // children.
   //
   return;
}

void RecipeTreeModel::versionedRecipe(Recipe * ancestor, Recipe * descendant) {
   qDebug() <<
      Q_FUNC_INFO << "Updating tree now that Recipe" << descendant->key() << "has ancestor Recipe" << ancestor->key();

   //
   // We have a recipe `ancestor` in the tree as a TreeNodeClassifier::PrimaryItem, and we've created `descendant` as a
   // snapshot of it.  We need to remove `ancestor` from the tree and add show it as a TreeNodeClassifier::SecondaryItem
   // child of `descendant` (which should already be in the tree by virtue of the
   // ObjectStoreTyped<Recipe>::signalObjectInserted per comment in RecipeTreeModel::spawnRecipe).
   //
   this->removeElement(*ancestor);
   auto descendantPointer = ObjectStoreWrapper::getShared(*descendant);

   QModelIndex descendantIndex = this->findElement(descendantPointer.get());
   this->showAncestors(descendantIndex);

   // We have to signal the data is in the tree first (dataChanged), then tell MainWindow something happened
   // (recipeSpawn).
   emit dataChanged(descendantIndex, descendantIndex);
   emit recipeSpawn(descendant);
   return;
}
