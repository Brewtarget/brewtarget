/*======================================================================================================================
 * model/StockPurchaseSalt.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#ifndef MODEL_STOCKPURCHASESALT_H
#define MODEL_STOCKPURCHASESALT_H
#pragma once

#include <QObject>
#include <QString>

#include "model/Salt.h"
#include "model/StockPurchase.h"
#include "model/IngredientAmount.h"
#include "model/StockPurchaseBase.h"
#include "model/StockUseIngredient.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StockPurchaseSalt { inline BtStringConst const property{#property}; }
AddPropertyName(salt)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

class StockUseSalt;
class StockPurchaseSaltEditor;

/**
 * \brief StockPurchase of \c Salt
 */
class StockPurchaseSalt : public StockPurchase,
                      public IngredientAmount<StockPurchaseSalt, Salt>,
                      public StockPurchaseBase   <StockPurchaseSalt, Salt, StockUseSalt> {
   Q_OBJECT

   INGREDIENT_AMOUNT_DECL(StockPurchaseSalt, Salt)
   STOCK_PURCHASE_DECL(Salt, salt)

   //=================================================== PROPERTIES ====================================================
   //! See \c StockPurchaseBase for getters and setters
   Q_PROPERTY(Salt *                               salt                READ salt                WRITE setSalt)
   Q_PROPERTY(std::optional<Measurement::Amount>   amountOrdered       READ amountOrdered       WRITE setAmountOrdered )
   Q_PROPERTY(Measurement::Amount                  amountReceived      READ amountReceived      WRITE setAmountReceived)
   Q_PROPERTY(Measurement::Amount                  amountRemaining     READ amountRemaining     STORED false)
   Q_PROPERTY(double                               quantityRemaining   READ quantityRemaining   STORED false)

   // See model/IngredientAmount.h for info, getters and setters for these properties
   Q_PROPERTY(int                           ingredientId READ ingredientId WRITE setIngredientId)
   Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
   Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
   Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
   Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
   Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)
};

#endif
