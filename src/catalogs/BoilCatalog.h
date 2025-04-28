/*======================================================================================================================
 * catalogs/BoilCatalog.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef CATALOGS_BOILCATALOG_H
#define CATALOGS_BOILCATALOG_H
#pragma once

#include <QDialog>
#include <QEvent>

#include "editors/BoilEditor.h"
#include "model/Boil.h"
#include "qtModels/sortFilterProxyModels/BoilSortFilterProxyModel.h"
#include "qtModels/tableModels/BoilTableModel.h"

// This needs to be the last include.  (I know, I know...)
#include "catalogs/CatalogBase.h"

#define BoilCatalogOptions CatalogBaseOptions{ .onePerRecipe = true }

/*!
 * \class BoilCatalog
 *
 * \brief View/controller class for showing/editing the list of salts in the database.
 */
class BoilCatalog : public QDialog, public CatalogBase<BoilCatalog,
                                                       Boil,
                                                       BoilTableModel,
                                                       BoilSortFilterProxyModel,
                                                       BoilEditor,
                                                       BoilCatalogOptions> {
   Q_OBJECT

   CATALOG_COMMON_DECL(Boil)

};

#endif
