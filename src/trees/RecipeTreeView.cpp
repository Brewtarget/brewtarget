/*======================================================================================================================
 * trees/RecipeTreeView.cpp is part of Brewtarget, and is copyright the following authors 2021-2026:
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

   return;
}

void RecipeTreeView::doSetSelected(QModelIndex const & index) {
   this->TreeViewBase::doSetSelected(index);
   auto recipe = this->getItem<Recipe>(index);
   MainWindow::instance().setRecipe(recipe.get());
   return;
}

bool RecipeTreeView::ancestorsAreShowing(QModelIndex ndx) {
   QModelIndex translated = m_treeSortFilterProxy.mapToSource(ndx);
   return this->m_model.showChild(translated);
}

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

void RecipeTreeView::brewItHelper   () { MainWindow::instance().brewItHelper   (); return; }
void RecipeTreeView::brewAgainHelper() { MainWindow::instance().brewAgainHelper(); return; }
void RecipeTreeView::changeBrewDate () { MainWindow::instance().changeBrewDate (); return; }
void RecipeTreeView::fixBrewNote    () { MainWindow::instance().fixBrewNote    (); return; }

TREE_VIEW_COMMON_CODE(Recipe, BrewNote)
