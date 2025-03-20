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

void RecipeTreeModel::addBrewNoteSubTree(Recipe & recipe, int childNumber, TreeNode const & parentNode, bool recurse) {
   auto const brewNotes = recurse ? RecipeHelper::brewNotesForRecipeAndAncestors(recipe) : recipe.brewNotes();
   TreeNode * recipeNodeRaw = parentNode.rawChild(childNumber);
   // It's a coding error to call this for a non-Recipe node...
   Q_ASSERT(recipeNodeRaw->classifier() == TreeNodeClassifier::PrimaryItem);
   // ...so this cast should be safe.
   auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(*recipeNodeRaw);

   QModelIndex recipeNodeIndex = this->createIndex(childNumber, 0, recipeNodeRaw);

   int jj = 0;

   for (auto brewNote : brewNotes) {
      // In previous insert loops, we ignore the error and soldier on. So we
      // will do that here too
      if (!this->insertChild(recipeNode, recipeNodeIndex, jj, brewNote)) {
         qWarning() << Q_FUNC_INFO << "BrewNote insert failed";
         continue;
      }
      this->observeElement(brewNote);
      ++jj;
   }
   return;
}

void RecipeTreeModel::loadTreeModel() {
   this->TreeModelBase::loadTreeModel();
   // Set up the BrewNotes in the tree
   auto recipes = ObjectStoreWrapper::getAllDisplayable<Recipe>();
   for (auto recipe : recipes) {
      QModelIndex recipeIndex = this->findElement(recipe);
      TreeNode * recipeNode = this->doTreeNode(recipeIndex);
      TreeNode * recipeParent = recipeNode->rawParent();
      qDebug() <<
         Q_FUNC_INFO << "recipe:" << *recipe << ", recipeNode:" << recipeNode << ", recipeParent:" << recipeParent;
      QModelIndex recipeParentIndex = this->derived().parent(recipeIndex);

      int childNumber = recipeNode->childNumber();
      if (PersistentSettings::value(PersistentSettings::Names::showsnapshots, false).toBool() &&
          recipe->hasAncestors()) {
         this->setShowChild(recipeParentIndex, true);
         this->addAncestoralTree(*recipe, childNumber, recipeParent);
         this->addBrewNoteSubTree(*recipe, childNumber, *recipeParent, false);
      } else {
         this->addBrewNoteSubTree(*recipe, childNumber, *recipeParent);
      }
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

void RecipeTreeModel::showAncestors(QModelIndex index) {

   if (!index.isValid()) {
      return;
   }

   TreeNode * node = this->treeNode(index);
   if (node->classifier() != TreeNodeClassifier::PrimaryItem) {
      // This is probably a coding error
      qWarning() << Q_FUNC_INFO << "Unexpected node type" << static_cast<int>(node->classifier());
      return;
   }

   auto & itemNode = static_cast<TreeItemNode<Recipe> &>(*node);

   auto descendant = itemNode.underlyingItem();
   auto ancestors = descendant->ancestors();

   this->removeChildren(0, node->childCount(), index);

   // add the brewnotes for this version back
   this->addBrewNoteSubTree(*descendant, index.row(), *node->rawParent(), false);

   // set showChild on the leaf node. I use this for drawing menus
   this->setShowChild(index, true);

   // Now loop through the ancestors. The nature of the beast is nearest
   // ancestors are first
   for (auto ancestor : ancestors) {
      int jj = node->childCount();
      if (!this->insertChild(index, jj, ancestor)) {
         qWarning() << "Could not add ancestoral brewnotes";
      }
      QModelIndex cIndex = this->findElement(ancestor, node);
      this->setShowChild(cIndex, true);
      // ew, but apparently this has to happen here.
      emit dataChanged(cIndex, cIndex);

      // add the brewnotes to the ancestors, but make sure we don't recurse
      this->addBrewNoteSubTree(*ancestor, jj, *node, false);
   }
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
   this->addBrewNoteSubTree(*descendant, index.row(), *node->rawParent());

   // This is for menus
   this->setShowChild(index, false);

   // Now we just need to mark each ancestor invisible again
   for (auto ancestor : descendant->ancestors()) {
      QModelIndex aIndex = this->findElement(ancestor, node);
      this->setShowChild(aIndex, false);
      emit dataChanged(aIndex, aIndex);
   }
   return;
}

// more cleverness must happen. Wonder if I can figure it out.
void RecipeTreeModel::showOrHideAllAncestors(bool show) {
   for (auto recipe : ObjectStoreWrapper::getAllDisplayable<Recipe>()) {
      if (recipe->hasAncestors()) {
         QModelIndex recipeIndex = this->findElement(recipe);
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
   QModelIndex ancestorIndex = this->findElement(ancestorPointer);
   if (!ancestorIndex.isValid()) {
      qWarning() << Q_FUNC_INFO << "Couldn't find the ancestor";
   }

   // Add the ancestor's brewnotes to the descendant
   this->addBrewNoteSubTree(*ancestor, ancestorIndex.row(), *node->rawParent());

   return;
}

// This is detaching a Recipe from its previous versions
void RecipeTreeModel::orphanRecipe(QModelIndex index) {
   TreeNode* node = this->treeNode(index);
   TreeNode* parentNode = node->rawParent();
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
   this->addBrewNoteSubTree(*orphan, index.row(), *parentNode, false);

   // set the ancestor to visible. Not sure this is required?
   ancestor->setDisplay(true);
   ancestor->setLocked(false);

   // Put the ancestor into the tree
   if (!this->insertChild(pIndex, pIndex.row(), ancestor)) {
      qWarning() << Q_FUNC_INFO << "Could not add ancestor to tree";
   }

   // Find the ancestor in the tree
   QModelIndex ancestorIndex = findElement(ancestor);
   if (!ancestorIndex.isValid()) {
      qWarning() << Q_FUNC_INFO << "Couldn't find the ancestor";
   }

   // Add the ancestor's brewnotes to the descendant
   this->addBrewNoteSubTree(*ancestor, ancestorIndex.row(), *parentNode);

   return;
}

void RecipeTreeModel::spawnRecipe(QModelIndex index) {
   Q_ASSERT(index.isValid());

   TreeNode * node = this->treeNode(index);
   // It's a coding error if the index supplied doesn't point to a Recipe node...
   Q_ASSERT(node->classifier() == TreeNodeClassifier::PrimaryItem);
   // ...so this cast should be safe.
   auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(*node);

   auto ancestor = recipeNode.underlyingItem();
   Q_ASSERT(ancestor);
   std::shared_ptr<Recipe> descendant = std::make_shared<Recipe>(*ancestor);
   // We want to store the new Recipe first so that it gets an ID...
   ObjectStoreWrapper::insert<Recipe>(descendant);
   // ...then we can connect it to the one it was copied from
   descendant->setAncestor(*ancestor);
   qDebug() <<
      Q_FUNC_INFO << "Created descendant Recipe" << descendant->key() << "of Recipe" << ancestor->key() <<
      "(at position" << index.row() << ")";

   //
   // First, we remove the ancestor from the tree
   //
   // NB: Inserting the descendant in the database will have generated a signal resulting in a call to
   // RecipeTreeModel::elementAddedRecipe(), so we can't assume that index is still valid, hence the reassignement here.
   //
   index = this->findElement(ancestor);
   if (!this->removeChildren(index.row(), 1, this->parent(index))) {
      qCritical() << Q_FUNC_INFO << "Could not find Recipe" << ancestor->key() << "in display tree";
   }

   // Now we need to find the descendant in the tree. This has to be done
   // after we removed the ancestor row, otherwise the index will be wrong.
   QModelIndex descendantIndex = this->findElement(descendant);
   this->showAncestors(descendantIndex);

   emit dataChanged(descendantIndex, descendantIndex);
   emit recipeSpawn(descendant.get());

   return;
}

void RecipeTreeModel::versionedRecipe(Recipe * ancestor, Recipe * descendant) {
   qDebug() <<
      Q_FUNC_INFO << "Updating tree now that Recipe" << descendant->key() << "has ancestor Recipe" << ancestor->key();

   auto ancestorPointer = ObjectStoreWrapper::getShared(*ancestor);

   // like before, remove the ancestor
   QModelIndex index = this->findElement(ancestorPointer);
   if (!this->removeChildren(index.row(), 1, this->parent(index))) {
      qCritical() << Q_FUNC_INFO << "Could not find Recipe" << ancestor->key() << "in display tree";
   }

   // add the descendant in, but get the index only after we removed the
   // ancestor
   auto descendantPointer = ObjectStoreWrapper::getShared(*descendant);
   QModelIndex descendantIndex = this->findElement(descendantPointer);
   this->showAncestors(descendantIndex);

   // do not mess with this order. We have to signal the data is in the tree
   // first (dataChanged), then put it in the right folder (doFolderChanged) and
   // finally tell MainWindow something happened (recipeSpawn).
   // Any other order doesn't work, or dumps core
   emit dataChanged(descendantIndex, descendantIndex);
   this->doFolderChanged(descendant);
   emit recipeSpawn(descendant);
   return;
}

void RecipeTreeModel::addAncestoralTree(Recipe const & recipe, int ii, TreeNode * parent) {
   TreeNode * temp = parent->rawChild(ii);
   int jj = 0;

   for (auto ancestor : recipe.ancestors()) {
      // insert the ancestor. This is most of magic. One day, I understood it.
      // Now I simply copy/paste it
      if (!this->insertChild(createIndex(ii, 0, temp), jj, ancestor)) {
         qWarning() << Q_FUNC_INFO << "Ancestor insert failed";
         continue;
      }
      // we need to find the index of what we just inserted
      QModelIndex cIndex = this->findElement(ancestor, temp);
      // and set showChild on it
      this->setShowChild(cIndex, true);

      // finally, add this ancestors brewnotes but do not recurse
      this->addBrewNoteSubTree(*ancestor, jj, *temp, false);
      this->observeElement(ancestor);
      ++jj;
   }
   return;
}

void RecipeTreeModel::addSubTree(std::shared_ptr<Recipe> const element,
                                 TreeItemNode<Recipe> * elementNode,
                                 [[maybe_unused]] int offset,
                                 bool const recurse) {
   QList<std::shared_ptr<BrewNote>> const notes{
      recurse ? RecipeHelper::brewNotesForRecipeAndAncestors(*element) : element->brewNotes()
   };
   TreeNode * temp = elementNode->rawChild(offset);

   int jj = 0;

   for (auto note : notes) {
      // In previous insert loops, we ignore the error and soldier on. So we
      // will do that here too
      auto const index = this->createIndex(elementNode->childNumber(), 0, temp);
      bool const insertOk = this->insertChild(*elementNode, index, jj, note);
      if (!insertOk) {
         qWarning() << Q_FUNC_INFO << "BrewNote insert failed";
         continue;
      }
      this->observeElement(note);
      ++jj;
   }

   return;
}
