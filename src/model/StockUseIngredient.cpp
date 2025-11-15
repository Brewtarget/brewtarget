/*======================================================================================================================
 * model/StockUseIngredient.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "model/StockUseIngredient.h"

#include "model/StockPurchaseFermentable.h"
#include "model/StockPurchaseHop.h"
#include "model/StockPurchaseMisc.h"
#include "model/StockPurchaseSalt.h"
#include "model/StockPurchaseYeast.h"

STOCK_USE_COMMON_CODE(Fermentable)
STOCK_USE_COMMON_CODE(Hop        )
STOCK_USE_COMMON_CODE(Misc       )
STOCK_USE_COMMON_CODE(Salt       )
STOCK_USE_COMMON_CODE(Yeast      )

ENUMERATED_COMMON_CODE(StockUseFermentable)
ENUMERATED_COMMON_CODE(StockUseHop        )
ENUMERATED_COMMON_CODE(StockUseMisc       )
ENUMERATED_COMMON_CODE(StockUseSalt       )
ENUMERATED_COMMON_CODE(StockUseYeast      )
