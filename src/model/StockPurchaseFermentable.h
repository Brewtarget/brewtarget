/*======================================================================================================================
 * model/StockPurchaseFermentable.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef MODEL_STOCKPURCHASEFERMENTABLE_H
#define MODEL_STOCKPURCHASEFERMENTABLE_H
#pragma once

#include <QObject>
#include <QString>

#include "model/Fermentable.h"
#include "model/StockPurchase.h"
#include "model/IngredientAmount.h"
#include "model/StockPurchaseBase.h"
#include "model/StockUseIngredient.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StockPurchaseFermentable { inline BtStringConst const property{#property}; }
AddPropertyName(color_lovibond)
AddPropertyName(color_srm     )
AddPropertyName(fermentable   )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

class StockUseFermentable;
class StockPurchaseFermentableEditor;

/**
 * \brief StockPurchase of \c Fermentable
 */
class StockPurchaseFermentable : public StockPurchase,
                             public IngredientAmount<StockPurchaseFermentable, Fermentable>,
                             public StockPurchaseBase   <StockPurchaseFermentable, Fermentable, StockUseFermentable> {
   Q_OBJECT

   INGREDIENT_AMOUNT_DECL(StockPurchaseFermentable, Fermentable)
   STOCK_PURCHASE_DECL(Fermentable, fermentable)

public:
    /**
     * \brief See comment in model/NamedEntity.h
     *
     *        (NB: localisedName() is declared in \c StockPurchaseBase)
     */
    static QString localisedName_color_lovibond();
    static QString localisedName_color_srm     ();

   //=================================================== PROPERTIES ====================================================
   //! See \c StockPurchaseBase for getters and setters
   Q_PROPERTY(Fermentable *                        fermentable         READ fermentable         WRITE setFermentable   )
   Q_PROPERTY(std::optional<Measurement::Amount>   amountOrdered       READ amountOrdered       WRITE setAmountOrdered )
   Q_PROPERTY(Measurement::Amount                  amountReceived      READ amountReceived      WRITE setAmountReceived)
   Q_PROPERTY(Measurement::Amount                  amountRemaining     READ amountRemaining     STORED false)
   Q_PROPERTY(double                               quantityRemaining   READ quantityRemaining   STORED false)

   // See model/IngredientAmount.h for info, getters and setters for these properties
   Q_PROPERTY(int                           ingredientId READ ingredientId WRITE setIngredientId)
   Q_PROPERTY(Measurement::Amount           amount       READ amount       WRITE setAmount      )
   Q_PROPERTY(double                        quantity     READ quantity     WRITE setQuantity    )
   Q_PROPERTY(Measurement::Unit const *     unit         READ unit         WRITE setUnit        )
   Q_PROPERTY(Measurement::PhysicalQuantity measure      READ measure      WRITE setMeasure     )
   Q_PROPERTY(bool                          isWeight     READ isWeight     WRITE setIsWeight    )

   /**
    * \brief See comments in model/StockPurchaseHop.h for why we optionally allow the user to set the color of the
    *        purchased fermentable.
    *
    *        See comments in model/Fermentable.h for why we store in Lovibond, not SRM.
    */
   Q_PROPERTY(std::optional<double> color_lovibond   READ color_lovibond   WRITE setColor_lovibond)
   Q_PROPERTY(std::optional<double> color_srm        READ color_srm        WRITE setColor_srm     )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   std::optional<double> color_lovibond() const;
   std::optional<double> color_srm     () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setColor_lovibond(std::optional<double> const val);
   void setColor_srm     (std::optional<double> const val);

private:
    std::optional<double> m_color_lovibond;

};

#endif
