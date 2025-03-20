/*======================================================================================================================
 * qtModels/sortFilterProxyModels/SaltSortFilterProxyModel.h is part of Brewtarget, and is copyright the following authors
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
#ifndef SORTFILTERPROXYMODELS_SALTSORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODELS_SALTSORTFILTERPROXYMODEL_H
#pragma once

#include <QSortFilterProxyModel>

#include "qtModels/sortFilterProxyModels/SortFilterProxyModelBase.h"
#include "qtModels/tableModels/SaltTableModel.h"
#include "qtModels/listModels/SaltListModel.h"

/*!
 * \class SaltSortFilterProxyModel
 *
 * \brief Proxy model for sorting/filtering salts.
 */
class SaltSortFilterProxyModel : public QSortFilterProxyModel,
                                 public SortFilterProxyModelBase<SaltSortFilterProxyModel,
                                                                 SaltTableModel,
                                                                 SaltListModel> {
   Q_OBJECT

   SORT_FILTER_PROXY_MODEL_COMMON_DECL(Salt)
};

#endif
