/*======================================================================================================================
 * widgets/CommonContextMenus.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "widgets/CommonContextMenus.h"

#include "MainWindow.h"

//
// We have to define this function here rather than in the header to avoid circular dependencies with MainWindow.h
//
template<class NE>
void CommonContextMenuHelper::doAddToOrSetForRecipe(std::shared_ptr<NE> selected) {
   if (selected) {
      //
      // In both cases, MainWindow does the heavy lifting here, including ensuring that the action is undoable
      //
      if constexpr (std::is_base_of_v<Ingredient, NE>) {
         // Version for FermentableCatalog, HopCatalog, MiscCatalog, YeastCatalog, etc
         MainWindow::instance().addIngredientToRecipe(*selected);
      } else {
         // Version for EquipmentCatalog, StyleCatalog, MashCatalog, BoilCatalog, FermentationCatalog
         MainWindow::instance().setForRecipe(selected);
      }
   }
   return;
}

template void CommonContextMenuHelper::doAddToOrSetForRecipe<Boil        >(std::shared_ptr<Boil        > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Equipment   >(std::shared_ptr<Equipment   > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Fermentable >(std::shared_ptr<Fermentable > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Fermentation>(std::shared_ptr<Fermentation> selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Hop         >(std::shared_ptr<Hop         > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Mash        >(std::shared_ptr<Mash        > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Misc        >(std::shared_ptr<Misc        > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Salt        >(std::shared_ptr<Salt        > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Style       >(std::shared_ptr<Style       > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Water       >(std::shared_ptr<Water       > selected);
template void CommonContextMenuHelper::doAddToOrSetForRecipe<Yeast       >(std::shared_ptr<Yeast       > selected);
