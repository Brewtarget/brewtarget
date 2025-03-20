/*======================================================================================================================
 * qtModels/sortFilterProxyModels/RecipeSortFilterProxyModel.h is part of Brewtarget, and is copyright the following authors
 * 2025:
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
#ifndef SORTFILTERPROXYMODELS_RECIPESORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODELS_RECIPESORTFILTERPROXYMODEL_H
#pragma once

#include <QSortFilterProxyModel>

#include "qtModels/sortFilterProxyModels/SortFilterProxyModelBase.h"
#include "qtModels/tableModels/RecipeTableModel.h"
#include "qtModels/listModels/RecipeListModel.h"

/*!
 * \class RecipeSortFilterProxyModel
 *
 * \brief Proxy model for sorting/filtering Recipes.
 */
class RecipeSortFilterProxyModel : public QSortFilterProxyModel,
                                   public SortFilterProxyModelBase<RecipeSortFilterProxyModel,
                                                                   RecipeTableModel,
                                                                   RecipeListModel> {
   Q_OBJECT
   SORT_FILTER_PROXY_MODEL_COMMON_DECL(Recipe)
};

#endif
