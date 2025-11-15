/*======================================================================================================================
 * model/StockUseIngredient.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef MODEL_STOCKUSEINGREDIENT_H
#define MODEL_STOCKUSEINGREDIENT_H
#pragma once

#include "model/EnumeratedBase.h"
#include "model/StockUse.h"
#include "model/StockUseBase.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Salt.h"
#include "model/Yeast.h"

class StockPurchaseFermentable;
class StockPurchaseHop;
class StockPurchaseMisc;
class StockPurchaseSalt;
class StockPurchaseYeast;

class StockUseFermentableItemDelegate;
class StockUseHopItemDelegate;
class StockUseMiscItemDelegate;
class StockUseSaltItemDelegate;
class StockUseYeastItemDelegate;

class StockUseFermentableEditor;
class StockUseHopEditor;
class StockUseMiscEditor;
class StockUseSaltEditor;
class StockUseYeastEditor;

class StockUseFermentableTableModel;
class StockUseHopTableModel;
class StockUseMiscTableModel;
class StockUseSaltTableModel;
class StockUseYeastTableModel;

class StockUseFermentable : public StockUse,
                                   public StockUseBase<StockUseFermentable, StockPurchaseFermentable>,
                                   public EnumeratedBase     <StockUseFermentable, StockPurchaseFermentable> {
   Q_OBJECT

   ENUMERATED_COMMON_DECL(StockUseFermentable, StockPurchaseFermentable)
   STOCK_USE_COMMON_DECL(Fermentable)

   //=================================================== PROPERTIES ====================================================
   //! See \c StockUseBase for getters and setters
   Q_PROPERTY(Measurement::Amount amountUsed         READ amountUsed          WRITE setAmountUsed)
   Q_PROPERTY(double              quantityRemaining  READ quantityRemaining   STORED false)
   Q_PROPERTY(Measurement::Amount amountRemaining    READ amountRemaining     STORED false)
   // See model/EnumeratedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId          READ ownerId          WRITE setOwnerId       )
   Q_PROPERTY(int sequenceNumber   READ sequenceNumber   WRITE setSequenceNumber)

};

class StockUseHop : public StockUse,
                           public StockUseBase<StockUseHop, StockPurchaseHop>,
                           public EnumeratedBase     <StockUseHop, StockPurchaseHop> {
   Q_OBJECT

   ENUMERATED_COMMON_DECL(StockUseHop, StockPurchaseHop)
   STOCK_USE_COMMON_DECL(Hop)

   //=================================================== PROPERTIES ====================================================
   //! See \c StockUseBase for getters and setters
   Q_PROPERTY(Measurement::Amount amountUsed         READ amountUsed          WRITE setAmountUsed)
   Q_PROPERTY(double              quantityRemaining  READ quantityRemaining   STORED false)
   Q_PROPERTY(Measurement::Amount amountRemaining    READ amountRemaining     STORED false)
   // See model/EnumeratedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId          READ ownerId          WRITE setOwnerId       )
   Q_PROPERTY(int sequenceNumber   READ sequenceNumber   WRITE setSequenceNumber)

};

class StockUseMisc : public StockUse,
                            public StockUseBase<StockUseMisc, StockPurchaseMisc>,
                            public EnumeratedBase     <StockUseMisc, StockPurchaseMisc> {
   Q_OBJECT

   ENUMERATED_COMMON_DECL(StockUseMisc, StockPurchaseMisc)
   STOCK_USE_COMMON_DECL(Misc)

   //=================================================== PROPERTIES ====================================================
   //! See \c StockUseBase for getters and setters
   Q_PROPERTY(Measurement::Amount amountUsed         READ amountUsed          WRITE setAmountUsed)
   Q_PROPERTY(double              quantityRemaining  READ quantityRemaining   STORED false)
   Q_PROPERTY(Measurement::Amount amountRemaining    READ amountRemaining     STORED false)
   // See model/EnumeratedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId          READ ownerId          WRITE setOwnerId       )
   Q_PROPERTY(int sequenceNumber   READ sequenceNumber   WRITE setSequenceNumber)

};

class StockUseSalt : public StockUse,
                            public StockUseBase<StockUseSalt, StockPurchaseSalt>,
                            public EnumeratedBase     <StockUseSalt, StockPurchaseSalt> {
   Q_OBJECT

   ENUMERATED_COMMON_DECL(StockUseSalt, StockPurchaseSalt)
   STOCK_USE_COMMON_DECL(Salt)

   //=================================================== PROPERTIES ====================================================
   //! See \c StockUseBase for getters and setters
   Q_PROPERTY(Measurement::Amount amountUsed         READ amountUsed          WRITE setAmountUsed)
   Q_PROPERTY(double              quantityRemaining  READ quantityRemaining   STORED false)
   Q_PROPERTY(Measurement::Amount amountRemaining    READ amountRemaining     STORED false)
   // See model/EnumeratedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId          READ ownerId          WRITE setOwnerId       )
   Q_PROPERTY(int sequenceNumber   READ sequenceNumber   WRITE setSequenceNumber)

};

class StockUseYeast : public StockUse,
                             public StockUseBase<StockUseYeast, StockPurchaseYeast>,
                             public EnumeratedBase     <StockUseYeast, StockPurchaseYeast> {
   Q_OBJECT

   ENUMERATED_COMMON_DECL(StockUseYeast, StockPurchaseYeast)
   STOCK_USE_COMMON_DECL(Yeast)

   //=================================================== PROPERTIES ====================================================
   //! See \c StockUseBase for getters and setters
   Q_PROPERTY(Measurement::Amount amountUsed         READ amountUsed          WRITE setAmountUsed)
   Q_PROPERTY(double              quantityRemaining  READ quantityRemaining   STORED false)
   Q_PROPERTY(Measurement::Amount amountRemaining    READ amountRemaining     STORED false)
   // See model/EnumeratedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId          READ ownerId          WRITE setOwnerId       )
   Q_PROPERTY(int sequenceNumber   READ sequenceNumber   WRITE setSequenceNumber)

};

STOCK_USE_COMMON_TMPL(Fermentable)
STOCK_USE_COMMON_TMPL(Hop)
STOCK_USE_COMMON_TMPL(Misc)
STOCK_USE_COMMON_TMPL(Salt)
STOCK_USE_COMMON_TMPL(Yeast)

#endif
