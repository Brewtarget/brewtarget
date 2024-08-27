/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/YeastEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#include "editors/YeastEditor.h"

#include <cmath>

#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

YeastEditor::YeastEditor(QWidget * parent) :
   QDialog(parent),
   EditorBase<YeastEditor, Yeast>() {
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   SMART_FIELD_INIT(YeastEditor, label_name          , lineEdit_name          , Yeast, PropertyNames::NamedEntity::name         );
   SMART_FIELD_INIT(YeastEditor, label_laboratory    , lineEdit_laboratory    , Yeast, PropertyNames::Yeast::laboratory         );
   SMART_FIELD_INIT(YeastEditor, label_inventory     , lineEdit_inventory     , Yeast, PropertyNames::Ingredient::totalInventory, 1);
   SMART_FIELD_INIT(YeastEditor, label_productId     , lineEdit_productId     , Yeast, PropertyNames::Yeast::productId          );
   SMART_FIELD_INIT(YeastEditor, label_minTemperature, lineEdit_minTemperature, Yeast, PropertyNames::Yeast::minTemperature_c, 1);
   SMART_FIELD_INIT(YeastEditor, label_maxTemperature, lineEdit_maxTemperature, Yeast, PropertyNames::Yeast::maxTemperature_c, 1);
   SMART_FIELD_INIT(YeastEditor, label_maxReuse      , lineEdit_maxReuse      , Yeast, PropertyNames::Yeast::maxReuse        );

   BT_COMBO_BOX_INIT(HopEditor, comboBox_yeastType        , Yeast, type        );
   BT_COMBO_BOX_INIT(HopEditor, comboBox_yeastForm        , Yeast, form        );
   BT_COMBO_BOX_INIT(HopEditor, comboBox_yeastFlocculation, Yeast, flocculation);

   BT_COMBO_BOX_INIT_COPQ(YeastEditor, comboBox_amountType, Yeast, PropertyNames::Ingredient::totalInventory, lineEdit_inventory);

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞

   SMART_FIELD_INIT(YeastEditor, label_alcoholTolerance, lineEdit_alcoholTolerance, Yeast, PropertyNames::Yeast::alcoholTolerance_pct, 1);
   SMART_FIELD_INIT(YeastEditor, label_attenuationMin  , lineEdit_attenuationMin  , Yeast, PropertyNames::Yeast::attenuationMin_pct  , 1);
   SMART_FIELD_INIT(YeastEditor, label_attenuationMax  , lineEdit_attenuationMax  , Yeast, PropertyNames::Yeast::attenuationMax_pct  , 1);

   BT_BOOL_COMBO_BOX_INIT(YeastEditor, boolCombo_phenolicOffFlavorPositive, Yeast, phenolicOffFlavorPositive);
   BT_BOOL_COMBO_BOX_INIT(YeastEditor, boolCombo_glucoamylasePositive     , Yeast, glucoamylasePositive     );
   BT_BOOL_COMBO_BOX_INIT(YeastEditor, boolCombo_killerProducingK1Toxin   , Yeast, killerProducingK1Toxin   );
   BT_BOOL_COMBO_BOX_INIT(YeastEditor, boolCombo_killerProducingK2Toxin   , Yeast, killerProducingK2Toxin   );
   BT_BOOL_COMBO_BOX_INIT(YeastEditor, boolCombo_killerProducingK28Toxin  , Yeast, killerProducingK28Toxin  );
   BT_BOOL_COMBO_BOX_INIT(YeastEditor, boolCombo_killerProducingKlusToxin , Yeast, killerProducingKlusToxin );
   BT_BOOL_COMBO_BOX_INIT(YeastEditor, boolCombo_killerNeutral            , Yeast, killerNeutral            );

   this->connectSignalsAndSlots();
   return;
}

YeastEditor::~YeastEditor() = default;

void YeastEditor::writeFieldsToEditItem() {

   this->m_editItem->setName            (lineEdit_name             ->text()                            );
   this->m_editItem->setType            (comboBox_yeastType        ->getNonOptValue<Yeast::Type>()     );
   this->m_editItem->setForm            (comboBox_yeastForm        ->getNonOptValue<Yeast::Form>()     );
   this->m_editItem->setLaboratory      (lineEdit_laboratory       ->text()                            );
   this->m_editItem->setProductId       (lineEdit_productId        ->text()                            );
   this->m_editItem->setMinTemperature_c(lineEdit_minTemperature   ->getOptCanonicalQty()              ); // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
   this->m_editItem->setMaxTemperature_c(lineEdit_maxTemperature   ->getOptCanonicalQty()              ); // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
   this->m_editItem->setFlocculation    (comboBox_yeastFlocculation->getOptValue<Yeast::Flocculation>());
   this->m_editItem->setMaxReuse        (lineEdit_maxReuse         ->getOptValue<int>()                ); // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
   this->m_editItem->setBestFor         (textEdit_bestFor          ->toPlainText()                     );
   this->m_editItem->setNotes           (textEdit_notes            ->toPlainText()                     );
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   this->m_editItem->setAlcoholTolerance_pct     (this->lineEdit_alcoholTolerance          ->getOptValue<double>());
   this->m_editItem->setAttenuationMin_pct       (this->lineEdit_attenuationMin            ->getOptValue<double>());
   this->m_editItem->setAttenuationMax_pct       (this->lineEdit_attenuationMax            ->getOptValue<double>());
   this->m_editItem->setPhenolicOffFlavorPositive(this->boolCombo_phenolicOffFlavorPositive->getOptBoolValue    ());
   this->m_editItem->setGlucoamylasePositive     (this->boolCombo_glucoamylasePositive     ->getOptBoolValue    ());
   this->m_editItem->setKillerProducingK1Toxin   (this->boolCombo_killerProducingK1Toxin   ->getOptBoolValue    ());
   this->m_editItem->setKillerProducingK2Toxin   (this->boolCombo_killerProducingK2Toxin   ->getOptBoolValue    ());
   this->m_editItem->setKillerProducingK28Toxin  (this->boolCombo_killerProducingK28Toxin  ->getOptBoolValue    ());
   this->m_editItem->setKillerProducingKlusToxin (this->boolCombo_killerProducingKlusToxin ->getOptBoolValue    ());
   this->m_editItem->setKillerNeutral            (this->boolCombo_killerNeutral            ->getOptBoolValue    ());

   return;
}

void YeastEditor::writeLateFieldsToEditItem() {
   //
   // Do this late to make sure we've the row in the inventory table (because total inventory amount isn't really an
   // attribute of the Misc).
   //
   // Note that we do not need to store the value of comboBox_amountType.  It merely controls the available unit for
   // lineEdit_inventory
   //
   // Note that, if the inventory field is blank, we'll treat that as meaning "don't change the inventory"
   //
   if (!this->lineEdit_inventory->isEmptyOrBlank()) {
      this->m_editItem->setTotalInventory(lineEdit_inventory->getNonOptCanonicalAmt());
   }
   return;
}

void YeastEditor::readFieldsFromEditItem(std::optional<QString> propName) {
   if (!propName || *propName == PropertyNames::NamedEntity::name         ) { this->lineEdit_name             ->setTextCursor(m_editItem->name           ()); // Continues to next line
                                                                              this->tabWidget_editor          ->setTabText(0, m_editItem->name           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::type               ) { this->comboBox_yeastType        ->setValue    (m_editItem->type            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::form               ) { this->comboBox_yeastForm        ->setValue    (m_editItem->form            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Ingredient::totalInventory) { this->lineEdit_inventory        ->setAmount   (m_editItem->totalInventory  ());
                                                                              this->comboBox_amountType       ->autoSetFromControlledField();
                                                                              if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::laboratory         ) { this->lineEdit_laboratory       ->setText     (m_editItem->laboratory      ()); // Continues to next line
                                                                              this->lineEdit_laboratory       ->setCursorPosition(0)                        ; if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::productId          ) { this->lineEdit_productId        ->setText     (m_editItem->productId       ()); // Continues to next line
                                                                              this->lineEdit_productId        ->setCursorPosition(0)                        ; if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::minTemperature_c   ) { this->lineEdit_minTemperature   ->setQuantity (m_editItem->minTemperature_c()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::maxTemperature_c   ) { this->lineEdit_maxTemperature   ->setQuantity (m_editItem->maxTemperature_c()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::flocculation       ) { this->comboBox_yeastFlocculation->setValue    (m_editItem->flocculation    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::maxReuse           ) { this->lineEdit_maxReuse         ->setQuantity (m_editItem->maxReuse        ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::bestFor            ) { this->textEdit_bestFor          ->setPlainText(m_editItem->bestFor         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::notes              ) { this->textEdit_notes            ->setPlainText(m_editItem->notes           ()); if (propName) { return; } }
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   if (!propName || *propName == PropertyNames::Yeast::alcoholTolerance_pct) { this->lineEdit_alcoholTolerance->setQuantity(m_editItem->alcoholTolerance_pct()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::attenuationMin_pct  ) { this->lineEdit_attenuationMin  ->setQuantity(m_editItem->attenuationMin_pct  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::attenuationMax_pct  ) { this->lineEdit_attenuationMax  ->setQuantity(m_editItem->attenuationMax_pct  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::phenolicOffFlavorPositive) { this->boolCombo_phenolicOffFlavorPositive->setValue(m_editItem->phenolicOffFlavorPositive()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::glucoamylasePositive     ) { this->boolCombo_glucoamylasePositive     ->setValue(m_editItem->glucoamylasePositive     ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::killerProducingK1Toxin   ) { this->boolCombo_killerProducingK1Toxin   ->setValue(m_editItem->killerProducingK1Toxin   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::killerProducingK2Toxin   ) { this->boolCombo_killerProducingK2Toxin   ->setValue(m_editItem->killerProducingK2Toxin   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::killerProducingK28Toxin  ) { this->boolCombo_killerProducingK28Toxin  ->setValue(m_editItem->killerProducingK28Toxin  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::killerProducingKlusToxin ) { this->boolCombo_killerProducingKlusToxin ->setValue(m_editItem->killerProducingKlusToxin ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Yeast::killerNeutral            ) { this->boolCombo_killerNeutral            ->setValue(m_editItem->killerNeutral            ()); if (propName) { return; } }

   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_SLOT_DEFINITIONS(YeastEditor)
