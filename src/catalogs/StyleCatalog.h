/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * catalogs/StyleCatalog.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef CATALOGS_STYLECATALOG_H
#define CATALOGS_STYLECATALOG_H
#pragma once

#include <QDialog>
#include <QEvent>

#include "editors/StyleEditor.h"
#include "model/Style.h"
#include "qtModels/sortFilterProxyModels/StyleSortFilterProxyModel.h"
#include "qtModels/tableModels/StyleTableModel.h"

// This needs to be the last include.  (I know, I know...)
#include "catalogs/CatalogBase.h"

#define StyleCatalogOptions CatalogBaseOptions{ .onePerRecipe = true }

/*!
 * \class StyleCatalog
 *
 * \brief View/controller class for showing/editing the list of styles in the database.
 */
class StyleCatalog : public QDialog, public CatalogBase<StyleCatalog,
                                                        Style,
                                                        StyleTableModel,
                                                        StyleSortFilterProxyModel,
                                                        StyleEditor,
                                                        StyleCatalogOptions> {
   Q_OBJECT

   CATALOG_COMMON_DECL(Style)
};

#endif
