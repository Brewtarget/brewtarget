/*======================================================================================================================
 * editors/StockPurchaseSaltEditor.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "editors/StockPurchaseSaltEditor.h"

#include <QIcon>
#include <QInputDialog>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockPurchaseSaltEditor.cpp"
#endif

StockPurchaseSaltEditor::StockPurchaseSaltEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<StockPurchaseSaltEditor, StockPurchaseSalt, StockPurchaseSaltEditorOptions>(editorName) {
   setupUi(this);

   //
   // Do the ingredient-specific bits of the UI
   //
   this->comboBox_ingredient = new BtComboBoxSalt(this->tab_main);
   this->comboBox_ingredient->setObjectName("comboBox_ingredient");
   this->gridLayout_main->addWidget(this->comboBox_ingredient, 0, 2, 1, 4);

   this->enumeratedItemsWidget = new StockUseSaltsWidget(this->groupBox_Uses);
   this->enumeratedItemsWidget->setObjectName("enumeratedItemsWidget");
   this->layout_Uses->addWidget(this->enumeratedItemsWidget);

   this->retranslateUi();

   this->postSetupUiInit({
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_ingredient    , comboBox_ingredient    , StockPurchaseSalt::salt),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_supplier      , lineEdit_supplier      , StockPurchase::supplier   ),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_note          , lineEdit_note          , StockPurchase::note       ),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_dateOrdered   , dateEdit_dateOrdered   , StockPurchase::dateOrdered),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_dateReceived  , dateEdit_dateReceived  , StockPurchase::dateReceived),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_amountOrdered , lineEdit_amountOrdered , StockPurchaseBase::amountOrdered , 1, WhenToWriteField::Late),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_amountReceived, lineEdit_amountReceived, StockPurchaseBase::amountReceived, 1, WhenToWriteField::Late),
      EDITOR_FIELD_COPQ(StockPurchaseSalt, label_amountType    , comboBox_amountType    , StockPurchaseBase::amountReceived, {lineEdit_amountReceived,
                                                                                                                                     lineEdit_amountOrdered},
                                                                                                                                    WhenToWriteField::Never),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_amountRemaining, display_amountRemaining, StockPurchaseBase::amountRemaining, 1),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_purchasePrice  , lineEdit_purchasePrice , StockPurchase::purchasePrice ),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_purchaseTax    , lineEdit_purchaseTax   , StockPurchase::purchaseTax   ),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_shippingCost   , lineEdit_shippingCost  , StockPurchase::shippingCost  ),
      EDITOR_FIELD_NORM(StockPurchaseSalt, label_dateBestBefore , dateEdit_dateBestBefore, StockPurchase::dateBestBefore),
   });
   return;
}

StockPurchaseSaltEditor::~StockPurchaseSaltEditor() = default;

void StockPurchaseSaltEditor::retranslateUi() {
   this->setWindowTitle(QCoreApplication::translate("StockPurchaseSaltEditor", "Salt Stock Purchase Editor", nullptr));
   this->label_ingredient->setText(Salt::localisedName());

   this->Ui::stockPurchaseIngredientEditor::retranslateUi(this);
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(StockPurchaseSalt)
