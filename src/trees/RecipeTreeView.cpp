/*======================================================================================================================
 * trees/RecipeTreeView.cpp is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#include "trees/RecipeTreeView.h"

#include "MainWindow.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeTreeView.cpp"
#endif


void RecipeTreeView::init(AncestorDialog & ancestorDialog, OptionDialog & optionDialog) {
   this->init(MainWindow::instance());
   this->m_model.init(ancestorDialog, optionDialog);

   this->m_versionMenu.setTitle(tr("Snapshots"));
   this->m_showAncestorAction = this->m_versionMenu.addAction(tr("Show Snapshots" ), this, &RecipeTreeView::showAncestors);
   this->m_hideAncestorAction = this->m_versionMenu.addAction(tr("Hide Snapshots" ), this, &RecipeTreeView::hideAncestors);
   this->m_orphanAction       = this->m_versionMenu.addAction(tr("Detach Recipe"  ), this, &RecipeTreeView::orphanRecipe );
   this->m_spawnAction        = this->m_versionMenu.addAction(tr("Snapshot Recipe"), this, &RecipeTreeView::spawnRecipe  );
   this->m_contextMenu.addMenu(&this->m_versionMenu);

   // NB below: for TreeViewBase<Recipe>, this->m_editor holds a pointer to MainWindow

   this->m_contextMenu.addSeparator();
   this->m_brewItAction = this->m_contextMenu.addAction(tr("Brew It!"), this->m_editor, &MainWindow::brewItHelper);
   this->m_contextMenu.addSeparator();

   this->m_secondaryContextMenu.addAction(tr("Brew Again"     ), this->m_editor, &MainWindow::brewAgainHelper);
   this->m_secondaryContextMenu.addAction(tr("Change date"    ), this->m_editor, &MainWindow::changeBrewDate );
   this->m_secondaryContextMenu.addAction(tr("Recalculate eff"), this->m_editor, &MainWindow::fixBrewNote    );
   this->m_secondaryContextMenu.addAction(tr("Delete"         ), this->m_editor, &MainWindow::deleteSelected );

   return;
}

void RecipeTreeView::doSetSelected(QModelIndex const & index) {
   this->TreeViewBase::doSetSelected(index);
   auto recipe = this->getItem<Recipe>(index);
   MainWindow::instance().setRecipe(recipe.get());
   return;
}

QMenu * RecipeTreeView::doGetContextMenu(QModelIndex const & selectedViewIndex) {
   auto [menu, selectedNode] = this->getContextMenuPair(selectedViewIndex);

   if (selectedNode->classifier() == TreeNodeClassifier::PrimaryItem) {
      auto & recipeNode = static_cast<TreeItemNode<Recipe> &>(*selectedNode);

      // TBD: Right at the top of the tree, it's possible to click on something that is neither a folder nor a recipe, so
      // we might have to check for that here.
      auto recipe = recipeNode.underlyingItem();
      QModelIndex translated = this->m_treeSortFilterProxy.mapToSource(selectedViewIndex);

      // you can not delete a locked recipe
      this->enableDelete(!recipe->locked());

      // if we have ancestors and are showing them but are not an actual
      // ancestor, then enable hide
      this->enableHideAncestor(recipe->hasAncestors() && this->m_model.showChild(translated) && recipe->display());

      // if we have ancestors and are not showing them, enable showAncestors
      this->enableShowAncestor(recipe->hasAncestors() && !this->m_model.showChild(translated));

      // if we have ancestors and are not locked, then we are a leaf node and
      // allow orphaning
      this->enableOrphan(recipe->hasAncestors() && ! recipe->locked());

      // if display is true, we can spawn it. This should mean we cannot spawn
      // ancestors directly, which is what I want.
      this->enableSpawn(recipe->display());

      // If user has clicked the Top-level Item 'Recipes' Once this menu Item will be forever disabled if we don't enable it.
      this->m_exportMenu.setEnabled(true);
      this->m_copyAction->setEnabled(true);
      this->m_brewItAction->setEnabled(true);
   } else if (selectedNode->classifier() == TreeNodeClassifier::Folder) {
      // For the root node, we want to disable delete etc
      bool const isRootNode = !selectedNode->rawParent();
      this->enableDelete(!isRootNode);
      this->enableHideAncestor(false);
      this->enableShowAncestor(false);
      this->enableOrphan      (false);
      this->enableSpawn       (false);
      this->m_exportMenu.setEnabled(false);
      this->m_copyAction->setEnabled(false);
      this->m_brewItAction->setEnabled(false);
   }

   return menu;
}

bool RecipeTreeView::ancestorsAreShowing(QModelIndex ndx) {
   QModelIndex translated = m_treeSortFilterProxy.mapToSource(ndx);
   return this->m_model.showChild(translated);
}

void RecipeTreeView::enableDelete      (bool enable) { this->m_deleteAction      ->setEnabled(enable); return; }
void RecipeTreeView::enableShowAncestor(bool enable) { this->m_showAncestorAction->setEnabled(enable); return; }
void RecipeTreeView::enableHideAncestor(bool enable) { this->m_hideAncestorAction->setEnabled(enable); return; }
void RecipeTreeView::enableOrphan      (bool enable) { this->m_orphanAction      ->setEnabled(enable); return; }
void RecipeTreeView::enableSpawn       (bool enable) { this->m_spawnAction       ->setEnabled(enable); return; }

void RecipeTreeView::versionedRecipe(Recipe * descendant) {
   emit recipeSpawn(descendant);
   return;
}

void RecipeTreeView::showAncestors() {
   for (QModelIndex selected : this->selectionModel()->selectedRows()) {
      this->m_model.showAncestors(this->m_treeSortFilterProxy.mapToSource(selected));
   }
   return;
}

void RecipeTreeView::hideAncestors() {
   for (QModelIndex selected : this->selectionModel()->selectedRows()) {
      this->m_model.hideAncestors(this->m_treeSortFilterProxy.mapToSource(selected));
   }
   return;
}

void RecipeTreeView::revertRecipeToPreviousVersion() {
   for (QModelIndex selected : this->selectionModel()->selectedRows()) {
      this->m_model.revertRecipeToPreviousVersion(this->m_treeSortFilterProxy.mapToSource(selected));
   }
   return;
}

void RecipeTreeView::orphanRecipe() {
   for (QModelIndex selected : this->selectionModel()->selectedRows()) {
      this->m_model.orphanRecipe(this->m_treeSortFilterProxy.mapToSource(selected));
   }
   return;
}

void RecipeTreeView::spawnRecipe() {
   for (QModelIndex selected : this->selectionModel()->selectedRows()) {
      this->m_model.spawnRecipe(this->m_treeSortFilterProxy.mapToSource(selected));
   }
   return;
}

TREE_VIEW_COMMON_CODE(Recipe, BrewNote)
