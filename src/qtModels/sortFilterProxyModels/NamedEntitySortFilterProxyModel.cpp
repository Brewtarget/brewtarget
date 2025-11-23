/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/sortFilterProxyModels/NamedEntitySortFilterProxyModel.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2025:
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Jamie Daws <jdelectronics1@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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

#include "model/StockPurchaseFermentable.h"
#include "model/StockPurchaseHop.h"
#include "model/StockPurchaseMisc.h"
#include "model/StockPurchaseSalt.h"
#include "model/StockPurchaseYeast.h"
#include "qtModels/sortFilterProxyModels/BoilSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/EquipmentSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/FermentableSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/FermentationSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/HopSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/MashSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/MiscSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionFermentableSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionHopSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionMiscSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionYeastSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdjustmentSaltSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/SaltSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/StyleSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/WaterSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/YeastSortFilterProxyModel.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing these includes reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BoilSortFilterProxyModel.cpp"
   #include "moc_EquipmentSortFilterProxyModel.cpp"
   #include "moc_FermentableSortFilterProxyModel.cpp"
   #include "moc_FermentationSortFilterProxyModel.cpp"
   #include "moc_HopSortFilterProxyModel.cpp"
   #include "moc_MashSortFilterProxyModel.cpp"
   #include "moc_MiscSortFilterProxyModel.cpp"
   #include "moc_RecipeAdditionFermentableSortFilterProxyModel.cpp"
   #include "moc_RecipeAdditionHopSortFilterProxyModel.cpp"
   #include "moc_RecipeAdditionMiscSortFilterProxyModel.cpp"
   #include "moc_RecipeAdditionYeastSortFilterProxyModel.cpp"
   #include "moc_RecipeAdjustmentSaltSortFilterProxyModel.cpp"
   #include "moc_RecipeSortFilterProxyModel.cpp"
   #include "moc_SaltSortFilterProxyModel.cpp"
   #include "moc_StyleSortFilterProxyModel.cpp"
   #include "moc_WaterSortFilterProxyModel.cpp"
   #include "moc_YeastSortFilterProxyModel.cpp"
#endif

// Insert the boiler-plate stuff that we cannot do in SortFilterProxyModelBase
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Boil)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Equipment)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Fermentable)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Fermentation)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Hop)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Mash)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Misc)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(RecipeAdditionFermentable)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(RecipeAdditionHop)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(RecipeAdditionMisc)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(RecipeAdditionYeast)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(RecipeAdjustmentSalt)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Recipe)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Salt)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Style)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Water)
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Yeast)