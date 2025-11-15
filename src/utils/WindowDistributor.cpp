/*======================================================================================================================
 * utils/WindowDistributor.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "utils/WindowDistributor.h"

#include "StockWindow.h"
#include "MainWindow.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Salt.h"
#include "model/Yeast.h"

namespace WindowDistributor {
   //
   // There is no general case for WindowDistributor::getInstance(), only specialisations.
   //
   template<>   EquipmentEditor & get<  EquipmentEditor>() { return MainWindow::instance().getEditor<  Equipment>(); }
   template<> FermentableEditor & get<FermentableEditor>() { return MainWindow::instance().getEditor<Fermentable>(); }
   template<>         HopEditor & get<        HopEditor>() { return MainWindow::instance().getEditor<        Hop>(); }
   template<>        MiscEditor & get<       MiscEditor>() { return MainWindow::instance().getEditor<       Misc>(); }
   template<>        SaltEditor & get<       SaltEditor>() { return MainWindow::instance().getEditor<       Salt>(); }
   template<>       StyleEditor & get<      StyleEditor>() { return MainWindow::instance().getEditor<      Style>(); }
   template<>       YeastEditor & get<      YeastEditor>() { return MainWindow::instance().getEditor<      Yeast>(); }

   template<>             MashEditor & get<            MashEditor>() { return MainWindow::instance().getEditor<            Mash>(); }
   template<>             BoilEditor & get<            BoilEditor>() { return MainWindow::instance().getEditor<            Boil>(); }
   template<>     FermentationEditor & get<    FermentationEditor>() { return MainWindow::instance().getEditor<    Fermentation>(); }
   template<>         MashStepEditor & get<        MashStepEditor>() { return MainWindow::instance().getEditor<        MashStep>(); }
   template<>         BoilStepEditor & get<        BoilStepEditor>() { return MainWindow::instance().getEditor<        BoilStep>(); }
   template<> FermentationStepEditor & get<FermentationStepEditor>() { return MainWindow::instance().getEditor<FermentationStep>(); }

   template<>   EquipmentCatalog & get<  EquipmentCatalog>() { return MainWindow::instance().getCatalog<  Equipment>(); }
   template<> FermentableCatalog & get<FermentableCatalog>() { return MainWindow::instance().getCatalog<Fermentable>(); }
   template<>         HopCatalog & get<        HopCatalog>() { return MainWindow::instance().getCatalog<        Hop>(); }
   template<>        MiscCatalog & get<       MiscCatalog>() { return MainWindow::instance().getCatalog<       Misc>(); }
   template<>        SaltCatalog & get<       SaltCatalog>() { return MainWindow::instance().getCatalog<       Salt>(); }
   template<>       StyleCatalog & get<      StyleCatalog>() { return MainWindow::instance().getCatalog<      Style>(); }
   template<>       WaterCatalog & get<      WaterCatalog>() { return MainWindow::instance().getCatalog<      Water>(); }
   template<>       YeastCatalog & get<      YeastCatalog>() { return MainWindow::instance().getCatalog<      Yeast>(); }

   template<> StockWindow & get<StockWindow>() { return MainWindow::instance().getWindow<StockWindow>(); }

   template<> StockPurchaseFermentableEditor & get<StockPurchaseFermentableEditor>() { return get<StockWindow>().getPurchaseEditor<Fermentable>(); }
   template<> StockPurchaseHopEditor         & get<StockPurchaseHopEditor        >() { return get<StockWindow>().getPurchaseEditor<Hop        >(); }
   template<> StockPurchaseMiscEditor        & get<StockPurchaseMiscEditor       >() { return get<StockWindow>().getPurchaseEditor<Misc       >(); }
   template<> StockPurchaseSaltEditor        & get<StockPurchaseSaltEditor       >() { return get<StockWindow>().getPurchaseEditor<Salt       >(); }
   template<> StockPurchaseYeastEditor       & get<StockPurchaseYeastEditor      >() { return get<StockWindow>().getPurchaseEditor<Yeast      >(); }

   template<> StockUseFermentableEditor & get<StockUseFermentableEditor>() { return get<StockWindow>().getUseEditor<Fermentable>(); }
   template<> StockUseHopEditor         & get<StockUseHopEditor        >() { return get<StockWindow>().getUseEditor<Hop        >(); }
   template<> StockUseMiscEditor        & get<StockUseMiscEditor       >() { return get<StockWindow>().getUseEditor<Misc       >(); }
   template<> StockUseSaltEditor        & get<StockUseSaltEditor       >() { return get<StockWindow>().getUseEditor<Salt       >(); }
   template<> StockUseYeastEditor       & get<StockUseYeastEditor      >() { return get<StockWindow>().getUseEditor<Yeast      >(); }

   template<class IngredientClass>
   void editorForNewStockPurchase(IngredientClass const * ingredient) requires(std::is_base_of_v<Ingredient, IngredientClass>) {
      auto stockPurchase = std::make_shared<typename IngredientClass::StockPurchaseClass>();
      stockPurchase->setIngredientRaw(ingredient);
      auto & stockPurchaseEditor = WindowDistributor::get<typename IngredientClass::StockPurchaseClass::EditorClass>();
      stockPurchaseEditor.setEditItem(stockPurchase);
      stockPurchaseEditor.show();
      return;
   }

   //
   // Instantiate the above template functions for the types that are going to use them
   // (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
   //
   template void editorForNewStockPurchase<Fermentable>(Fermentable const * ingredient);
   template void editorForNewStockPurchase<Hop        >(Hop         const * ingredient);
   template void editorForNewStockPurchase<Misc       >(Misc        const * ingredient);
   template void editorForNewStockPurchase<Salt       >(Salt        const * ingredient);
   template void editorForNewStockPurchase<Yeast      >(Yeast       const * ingredient);

}
