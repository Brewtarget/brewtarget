/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/StyleTableModel.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef TABLEMODELS_STYLETABLEMODEL_H
#define TABLEMODELS_STYLETABLEMODEL_H
#pragma once

#include <QStyledItemDelegate>

#include "model/Style.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Define the columns on this table
COLUMN_NAMES(StyleTableModel, Name            ,
                              Type            ,
                              Category        ,
                              CategoryNumber  ,
                              StyleLetter     ,
                              StyleGuide      ,
                              NumRecipesUsedIn,)

/*!
 * \class StyleTableModel
 *
 * \brief Table model for a list of styles.
 */
class StyleTableModel : public BtTableModel, public TableModelBase<StyleTableModel, Style> {
   Q_OBJECT
   TABLE_MODEL_COMMON_DECL(Style)
};

//============================================== CLASS StyleItemDelegate ===============================================
/*!
 * \class StyleItemDelegate
 *
 * \brief Item delegate for style tables.
 * \sa StyleTableModel
 */
class StyleItemDelegate : public QStyledItemDelegate,
                          public ItemDelegate<StyleItemDelegate, StyleTableModel> {
   Q_OBJECT
   ITEM_DELEGATE_COMMON_DECL(Style)
};

#endif
