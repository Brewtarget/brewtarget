/*======================================================================================================================
 * widgets/StockManagerIngredient.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef WIDGETS_STOCKMANAGERINGREDIENT_H
#define WIDGETS_STOCKMANAGERINGREDIENT_H
#pragma once

#include "editors/StockUseIngredientEditor.h"
#include "editors/StockPurchaseFermentableEditor.h"
#include "editors/StockPurchaseHopEditor.h"
#include "editors/StockPurchaseMiscEditor.h"
#include "editors/StockPurchaseSaltEditor.h"
#include "editors/StockPurchaseYeastEditor.h"
#include "model/StockPurchaseFermentable.h"
#include "model/StockPurchaseHop.h"
#include "model/StockPurchaseMisc.h"
#include "model/StockPurchaseSalt.h"
#include "model/StockPurchaseYeast.h"
#include "trees/NamedEntityTreeView.h"
#include "trees/NamedEntityTreeSortFilterProxyModel.h"
#include "widgets/StockManager.h"
#include "widgets/StockManagerBase.h"

class StockManagerFermentable : public StockManager,
                                public StockManagerBase<StockManagerFermentable,
                                                        StockPurchaseFermentable,
                                                        StockPurchaseFermentableTreeView> {
   Q_OBJECT
   STOCK_MGR_COMMON_DECL(Fermentable)
};

class StockManagerHop : public StockManager,
                        public StockManagerBase<StockManagerHop,
                                                StockPurchaseHop,
                                                StockPurchaseHopTreeView> {
   Q_OBJECT
   STOCK_MGR_COMMON_DECL(Hop)
};

class StockManagerMisc : public StockManager,
                         public StockManagerBase<StockManagerMisc,
                                                 StockPurchaseMisc,
                                                 StockPurchaseMiscTreeView> {
   Q_OBJECT
   STOCK_MGR_COMMON_DECL(Misc)
};

class StockManagerSalt : public StockManager,
                         public StockManagerBase<StockManagerSalt,
                                                 StockPurchaseSalt,
                                                 StockPurchaseSaltTreeView> {
   Q_OBJECT
   STOCK_MGR_COMMON_DECL(Salt)
};

class StockManagerYeast : public StockManager,
                          public StockManagerBase<StockManagerYeast,
                                                  StockPurchaseYeast,
                                                  StockPurchaseYeastTreeView> {
   Q_OBJECT
   STOCK_MGR_COMMON_DECL(Yeast)
};

#endif
