/*======================================================================================================================
 * editors/StockPurchaseHopEditor.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "editors/StockPurchaseHopEditor.h"

#include <QIcon>
#include <QInputDialog>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockPurchaseHopEditor.cpp"
#endif

StockPurchaseHopEditor::StockPurchaseHopEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<StockPurchaseHopEditor, StockPurchaseHop, StockPurchaseHopEditorOptions>(editorName) {
   setupUi(this);

   //
   // Do the ingredient-specific bits of the UI
   //
   this->comboBox_ingredient = new BtComboBoxHop(this->tab_main);
   this->comboBox_ingredient->setObjectName("comboBox_ingredient");
   this->gridLayout_main->addWidget(this->comboBox_ingredient, 0, 2, 1, 4);

   this->enumeratedItemsWidget = new StockUseHopsWidget(this->groupBox_Uses);
   this->enumeratedItemsWidget->setObjectName("enumeratedItemsWidget");
   this->layout_Uses->addWidget(this->enumeratedItemsWidget);

   this->retranslateUi();

   this->postSetupUiInit({
      EDITOR_FIELD_NORM(StockPurchaseHop, label_ingredient    , comboBox_ingredient    , StockPurchaseHop::hop),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_supplier      , lineEdit_supplier      , StockPurchase::supplier   ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_dateOrdered   , dateEdit_dateOrdered   , StockPurchase::dateOrdered),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_dateReceived  , dateEdit_dateReceived  , StockPurchase::dateReceived),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_amountOrdered , lineEdit_amountOrdered , StockPurchaseBase::amountOrdered , 1, WhenToWriteField::Late),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_amountReceived, lineEdit_amountReceived, StockPurchaseBase::amountReceived, 1, WhenToWriteField::Late),
      EDITOR_FIELD_COPQ(StockPurchaseHop, label_amountType    , comboBox_amountType    , StockPurchaseBase::amountReceived, {lineEdit_amountReceived,
                                                                                                                                     lineEdit_amountOrdered},
                                                                                                                                    WhenToWriteField::Never),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_amountRemaining, label_amountRemaining_value, StockPurchaseBase::amountRemaining, 1),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_purchasePrice  , lineEdit_purchasePrice     , StockPurchase::purchasePrice ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_purchaseTax    , lineEdit_purchaseTax       , StockPurchase::purchaseTax   ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_shippingCost   , lineEdit_shippingCost      , StockPurchase::shippingCost  ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_dateBestBefore , dateEdit_dateBestBefore    , StockPurchase::dateBestBefore),
   });
   return;
}

StockPurchaseHopEditor::~StockPurchaseHopEditor() = default;

void StockPurchaseHopEditor::retranslateUi() {
   this->setWindowTitle(QCoreApplication::translate("StockPurchaseHopEditor", "Hop Stock Purchase Editor", nullptr));
   this->label_ingredient->setText(Hop::localisedName());

   this->Ui::stockPurchaseIngredientEditor::retranslateUi(this);
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(StockPurchaseHop)
