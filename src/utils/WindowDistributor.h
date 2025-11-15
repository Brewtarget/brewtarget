/*======================================================================================================================
 * utils/WindowDistributor.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef UTILS_WINDOWDISTRIBUTOR_H
#define UTILS_WINDOWDISTRIBUTOR_H
#pragma once

#include <type_traits>

class Ingredient;

/**
 * \brief Having a central place to obtain/invoke various windows helps us avoid circular dependencies.
 *
 *        Previously some of this was done in \c MainWindow, but that class is already big enough!
 */
namespace WindowDistributor {
   /**
    * \brief For each different dialog in the application, the corresponding specialisation of this function (in
    *        \c utils/WindowDistributor.cpp knows how to obtain a (or the) instance thereof.
    */
   template<class WindowClass> WindowClass & get();

   /**
    * \brief For a given instance of an \c Ingredient subclass, this invokes the relevant \c StockPurchase editor (eg
    *        \c StockPurchaseFermentableEditor, \c StockPurchaseHopEditor, etc) ready to record a new purchase of the
    *        \c Ingredient.
    *
    * \param ingredient - if \c null, user will have to select ingredient in the editor
    */
   template<class IngredientClass>
   void editorForNewStockPurchase(IngredientClass const * ingredient) requires(std::is_base_of_v<Ingredient, IngredientClass>);
}


#endif
