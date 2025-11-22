/*======================================================================================================================
 * editors/StockPurchaseFermentableEditor.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef EDITORS_STOCKPURCHASEFERMENTABLEEDITOR_H
#define EDITORS_STOCKPURCHASEFERMENTABLEEDITOR_H
#pragma once

#include "ui_stockPurchaseIngredientEditor.h"

#include <QDialog>
#include <QMetaProperty>
#include <QString>

#include "editors/EditorBase.h"
#include "model/StockPurchaseFermentable.h"
#include "widgets/EnumeratedItemsWidget.h"

#define StockPurchaseFermentableEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
/*!
 * \class StockPurchaseFermentableEditor
 *
 * \brief View/controller class for creating and editing StockPurchaseFermentables.
 *
 *        See comment on EditorBase::connectSignalsAndSlots for why we need to have \c public, not \c private
 *        inheritance from the Ui base.
 */
class StockPurchaseFermentableEditor :
   public QDialog,
   public Ui::stockPurchaseIngredientEditor,
   public EditorBase<StockPurchaseFermentableEditor, StockPurchaseFermentable, StockPurchaseFermentableEditorOptions> {

   Q_OBJECT

   EDITOR_COMMON_DECL(StockPurchaseFermentable, StockPurchaseFermentableEditorOptions)

public:
   void retranslateUi();

   BtComboBoxFermentable *      comboBox_ingredient   = nullptr;
   StockUseFermentablesWidget * enumeratedItemsWidget = nullptr;

    /**
     * Extra fields specific to \c StockPurchaseFermentable
     */
    SmartLabel     *    label_color = nullptr;
    SmartLineEdit  * lineEdit_color = nullptr;
};

#endif
