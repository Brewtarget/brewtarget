/*======================================================================================================================
 * trees/NamedEntityTreeSortFilterProxyModel.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "trees/NamedEntityTreeSortFilterProxyModel.h"

#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Mash.h"
#include "model/Misc.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "trees/TreeModel.h"
#include "trees/TreeModelBase.h"

TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Equipment  )
TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Fermentable)
TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Hop        )
TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Mash       )
TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Misc       )
TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Yeast      )
TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Style      )
TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Water      )
TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(Recipe     , BrewNote)
