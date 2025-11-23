/*======================================================================================================================
 * editors/StockPurchaseYeastEditor.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef EDITORS_STOCKPURCHASEYEASTEDITOR_H
#define EDITORS_STOCKPURCHASEYEASTEDITOR_H
#pragma once

#include "ui_stockPurchaseIngredientEditor.h"

#include <QDialog>
#include <QMetaProperty>
#include <QString>

#include "editors/EditorBase.h"
#include "model/StockPurchaseYeast.h"
#include "widgets/EnumeratedItemsWidget.h"

#define StockPurchaseYeastEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
/*!
 * \class StockPurchaseYeastEditor
 *
 * \brief View/controller class for creating and editing StockPurchaseYeasts.
 *
 *        See comment on EditorBase::connectSignalsAndSlots for why we need to have \c public, not \c private
 *        inheritance from the Ui base.
 */
class StockPurchaseYeastEditor :
   public QDialog,
   public Ui::stockPurchaseIngredientEditor,
   public EditorBase<StockPurchaseYeastEditor, StockPurchaseYeast, StockPurchaseYeastEditorOptions> {

   Q_OBJECT

   EDITOR_COMMON_DECL(StockPurchaseYeast, StockPurchaseYeastEditorOptions)

public:
   void retranslateUi();

   BtComboBoxYeast *      comboBox_ingredient   = nullptr;
   StockUseYeastsWidget * enumeratedItemsWidget = nullptr;
};

#endif
