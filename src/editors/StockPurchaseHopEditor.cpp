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

   this->   label_year = new QLabel   {this->tab_extras};
   this->lineEdit_year = new QLineEdit{this->tab_extras};
   this->label_year->setBuddy(this->lineEdit_year);
   this->gridLayout_extras->addWidget(this->   label_year, 2, 1, 1, 1);
   this->gridLayout_extras->addWidget(this->lineEdit_year, 2, 2, 1, 1);

   this->   label_alpha = new QLabel       {this->tab_extras};
   this->lineEdit_alpha = new SmartLineEdit{this->tab_extras};
   this->label_alpha->setBuddy(this->lineEdit_alpha);
   this->gridLayout_extras->addWidget(this->   label_alpha, 2, 4, 1, 1);
   this->gridLayout_extras->addWidget(this->lineEdit_alpha, 2, 5, 1, 1);

   this->   label_hopForm = new QLabel        {this->tab_extras};
   this->comboBox_hopForm = new BtComboBoxEnum{this->tab_extras};
   this->label_hopForm->setBuddy(this->comboBox_hopForm);
   this->gridLayout_extras->addWidget(this->   label_hopForm, 4, 4, 1, 1);
   this->gridLayout_extras->addWidget(this->comboBox_hopForm, 4, 5, 1, 1);

   this->retranslateUi();

   this->postSetupUiInit({
      EDITOR_FIELD_NORM(StockPurchaseHop, label_ingredient    , comboBox_ingredient    , StockPurchaseHop::hop),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_supplier      , lineEdit_supplier      , StockPurchase::supplier   ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_note          , lineEdit_note          , StockPurchase::note       ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_dateOrdered   , dateEdit_dateOrdered   , StockPurchase::dateOrdered),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_dateReceived  , dateEdit_dateReceived  , StockPurchase::dateReceived),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_amountOrdered , lineEdit_amountOrdered , StockPurchaseBase::amountOrdered , 1, WhenToWriteField::Late),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_amountReceived, lineEdit_amountReceived, StockPurchaseBase::amountReceived, 1, WhenToWriteField::Late),
      EDITOR_FIELD_COPQ(StockPurchaseHop, label_amountType    , comboBox_amountType    , StockPurchaseBase::amountReceived, {lineEdit_amountReceived,
                                                                                                                             lineEdit_amountOrdered},
                                                                                                                            WhenToWriteField::Never),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_amountRemaining, display_amountRemaining, StockPurchaseBase::amountRemaining, 1),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_purchasePrice  , lineEdit_purchasePrice , StockPurchase::purchasePrice ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_purchaseTax    , lineEdit_purchaseTax   , StockPurchase::purchaseTax   ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_shippingCost   , lineEdit_shippingCost  , StockPurchase::shippingCost  ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_dateBestBefore , dateEdit_dateBestBefore, StockPurchase::dateBestBefore),
      //
      // Extra fields specific to \c StockPurchaseHop
      //
      // Note that we use EDITOR_FIELD_NORM rather than EDITOR_FIELD_ENUM for hopForm because we want to use the
      // mappings from Hop even though this is a field/property on StockPurchaseHop.
      //
      EDITOR_FIELD_NORM(StockPurchaseHop, label_year   , lineEdit_year   , StockPurchaseHop::year     ),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_alpha  , lineEdit_alpha  , StockPurchaseHop::alpha_pct),
      EDITOR_FIELD_NORM(StockPurchaseHop, label_hopForm, comboBox_hopForm, StockPurchaseHop::form     , Hop::formStringMapping,
                                                                                                        Hop::formDisplayNames),
   });
   return;
}

StockPurchaseHopEditor::~StockPurchaseHopEditor() = default;

void StockPurchaseHopEditor::retranslateUi() {
   this->setWindowTitle(QCoreApplication::translate("StockPurchaseHopEditor", "Hop Stock Purchase Editor", nullptr));
   this->label_ingredient->setText(Hop::localisedName());

   this->label_year   ->setText(StockPurchaseHop::localisedName_year     ());
   this->label_alpha  ->setText(StockPurchaseHop::localisedName_alpha_pct());
   this->label_hopForm->setText(StockPurchaseHop::localisedName_form());

   this->Ui::stockPurchaseIngredientEditor::retranslateUi(this);
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(StockPurchaseHop)
