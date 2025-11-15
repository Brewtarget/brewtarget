/*======================================================================================================================
 * model/StockPurchaseHop.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef MODEL_STOCKPURCHASEHOP_H
#define MODEL_STOCKPURCHASEHOP_H
#pragma once

#include <QObject>
#include <QString>

#include "model/Hop.h"
#include "model/StockPurchase.h"
#include "model/IngredientAmount.h"
#include "model/StockPurchaseBase.h"
#include "model/StockUseIngredient.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StockPurchaseHop { inline BtStringConst const property{#property}; }
AddPropertyName(alpha_pct)
AddPropertyName(form     )
AddPropertyName(year     )
AddPropertyName(hop      )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

class StockUseHop;
class StockPurchaseHopEditor;

/**
 * \brief StockPurchase of \c Hop
 */
class StockPurchaseHop : public StockPurchase,
                     public IngredientAmount<StockPurchaseHop, Hop>,
                     public StockPurchaseBase   <StockPurchaseHop, Hop, StockUseHop> {
   Q_OBJECT

   INGREDIENT_AMOUNT_DECL(StockPurchaseHop, Hop)
   STOCK_PURCHASE_DECL(Hop, hop)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    *
    *        (NB: localisedName() is declared in \c StockPurchaseBase)
    */
   static QString localisedName_alpha_pct();
   static QString localisedName_form     ();
   static QString localisedName_year     ();

   //=================================================== PROPERTIES ====================================================
   //! See \c StockPurchaseBase for getters and setters
   Q_PROPERTY(Hop *                                hop                 READ hop                 WRITE setHop)
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

   /**
    * \brief Harvest year of this batch of hops.  (If not set, look to the \c Hop record.)
    *
    *        As with other fields here, you can set year on \c Hop itself, in which case you'd probably either want to
    *        leave it blank here in \c StockPurchaseHop (or set it to the same).  Alternatively, you can leave it blank in
    *        \c Hop and set it per-batch here in in \c StockPurchaseHop.
    *
    *        You might think this should be an integer, but we don't actually want it formatted as an integer -- eg it
    *        is idiomatic to write 2025 not 2,025 for the year.  In theory, an integer would be better for sorting, but
    *        in the Gregorian calendar that's not a problem for another 7975 years!
    *
    *        An alternative would be to store this as \c QDate, but that seems overkill when we only want the year part.
    */
   Q_PROPERTY(QString               year        READ year        WRITE setYear     )
   /**
    * \brief Actual percent alpha acid for this batch of hops.  (If not set, look to the \c Hop record.)
    *
    *        NOTE that, unlike the other fields here, this is a \b required field on \c Hop.  However, we make it
    *        optional here to allow users the choice about whether to use \c Hop records as per-year, per-type etc or
    *        as more generic with the per-year, per-type etc info in \c StockPurchaseHop.
    */
   Q_PROPERTY(std::optional<double> alpha_pct   READ alpha_pct   WRITE setAlpha_pct)
   /**
    * \brief The \c Hop::Form.  (If not set, look to the \c Hop record.)
    *
    *        NOTE that \c Hop also has an optional \c form property.  Depending on how you want to use the software, you
    *        either:
    *           - Set \c form on each \c Hop and leave it unset here in \c StockPurchaseHop, thus eg treating pellet and
    *             leaf forms of the same hop as completely different.
    *        or:
    *           - Set \c form on each \c StockPurchaseHop and leave it unset on \c Hop, thus eg treating pellet and leaf
    *             forms of the same hop as just differences between batches.
    *
    *        See comment in \c model/Fermentable.h for \c grainGroup property for why this has to be
    *        \c std::optional<int>, not \c std::optional<Use>
    */
   Q_PROPERTY(std::optional<int>    form        READ formAsInt   WRITE setFormAsInt)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   std::optional<double>    alpha_pct         () const;
   std::optional<Hop::Form> form              () const;
   std::optional<int      > formAsInt         () const;
   QString                  year              () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setAlpha_pct(std::optional<double>    const   val);
   void setForm     (std::optional<Hop::Form> const   val);
   void setFormAsInt(std::optional<int      > const   val);
   void setYear     (QString                  const & val);

private:
   std::optional<double>    m_alpha_pct;
   std::optional<Hop::Form> m_form     ;
   QString                  m_year     ;

};

#endif
