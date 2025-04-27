/*======================================================================================================================
 * catalogs/MashCatalog.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef CATALOGS_MASHCATALOG_H
#define CATALOGS_MASHCATALOG_H
#pragma once

#include <QDialog>
#include <QEvent>

#include "editors/MashEditor.h"
#include "model/Mash.h"
#include "qtModels/sortFilterProxyModels/MashSortFilterProxyModel.h"
#include "qtModels/tableModels/MashTableModel.h"

// This needs to be the last include.  (I know, I know...)
#include "catalogs/CatalogBase.h"

#define MashCatalogOptions CatalogBaseOptions{ .onePerRecipe = true }

/*!
 * \class MashCatalog
 *
 * \brief View/controller class for showing/editing the list of salts in the database.
 */
class MashCatalog : public QDialog, public CatalogBase<MashCatalog,
                                                       Mash,
                                                       MashTableModel,
                                                       MashSortFilterProxyModel,
                                                       MashEditor,
                                                       MashCatalogOptions> {
   Q_OBJECT

   CATALOG_COMMON_DECL(Mash)

};

#endif
