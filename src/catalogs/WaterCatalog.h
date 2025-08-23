/*======================================================================================================================
 * catalogs/WaterCatalog.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef CATALOGS_WATERCATALOG_H
#define CATALOGS_WATERCATALOG_H
#pragma once

#include <QDialog>
#include <QEvent>

#include "editors/WaterEditor.h"
#include "model/Water.h"
#include "model/RecipeUseOfWater.h"
#include "qtModels/sortFilterProxyModels/WaterSortFilterProxyModel.h"
#include "qtModels/tableModels/WaterTableModel.h"

// This needs to be the last include.  (I know, I know...)
#include "catalogs/CatalogBase.h"

#define WaterCatalogOptions CatalogBaseOptions{ }

/*!
 * \class WaterCatalog
 *
 * \brief View/controller class for showing/editing the list of salts in the database.
 */
class WaterCatalog : public QDialog, public CatalogBase<WaterCatalog,
                                                        Water,
                                                        WaterTableModel,
                                                        WaterSortFilterProxyModel,
                                                        WaterEditor,
                                                        WaterCatalogOptions> {
   Q_OBJECT

   CATALOG_COMMON_DECL(Water)

};

#endif
