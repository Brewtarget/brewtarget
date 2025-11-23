/*======================================================================================================================
 * editors/StockPurchaseHopEditor.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef EDITORS_STOCKPURCHASEHOPEDITOR_H
#define EDITORS_STOCKPURCHASEHOPEDITOR_H
#pragma once

#include "ui_stockPurchaseIngredientEditor.h"

#include <QDialog>
#include <QMetaProperty>
#include <QString>

#include "editors/EditorBase.h"
#include "model/StockPurchaseHop.h"
#include "widgets/EnumeratedItemsWidget.h"

#define StockPurchaseHopEditorOptions EditorBaseOptions{ .nameTab = false, .idDisplay = true, .numRecipesUsing = false }
/*!
 * \class StockPurchaseHopEditor
 *
 * \brief View/controller class for creating and editing StockPurchaseHops.
 *
 *        See comment on EditorBase::connectSignalsAndSlots for why we need to have \c public, not \c private
 *        inheritance from the Ui base.
 */
class StockPurchaseHopEditor :
   public QDialog,
   public Ui::stockPurchaseIngredientEditor,
   public EditorBase<StockPurchaseHopEditor, StockPurchaseHop, StockPurchaseHopEditorOptions> {

   Q_OBJECT

   EDITOR_COMMON_DECL(StockPurchaseHop, StockPurchaseHopEditorOptions)

public:
   void retranslateUi();

   BtComboBoxHop *      comboBox_ingredient   = nullptr;
   StockUseHopsWidget * enumeratedItemsWidget = nullptr;

   /**
    * Extra fields specific to \c StockPurchaseHop
    */
   QLabel         *       label_year = nullptr;
   QLineEdit      *    lineEdit_year = nullptr;
   QLabel         *      label_alpha = nullptr;
   SmartLineEdit  *   lineEdit_alpha = nullptr;
   QLabel         *    label_hopForm = nullptr;
   BtComboBoxEnum * comboBox_hopForm = nullptr;
};

#endif
