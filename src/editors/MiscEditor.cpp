/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/MiscEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
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
#include "editors/MiscEditor.h"

#include <QtGui>
#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

MiscEditor::MiscEditor(QWidget * parent) :
   QDialog(parent),
   EditorBase<MiscEditor, Misc>() {
   setupUi(this);

   tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   SMART_FIELD_INIT(MiscEditor, label_name     , lineEdit_name     , Misc, PropertyNames::NamedEntity::name);
   SMART_FIELD_INIT(MiscEditor, label_inventory, lineEdit_inventory, Misc, PropertyNames::Ingredient::totalInventory, 1);

   BT_COMBO_BOX_INIT(MiscEditor, comboBox_type,  Misc, type);

   BT_COMBO_BOX_INIT_COPQ(MiscEditor, comboBox_amountType, Misc, PropertyNames::Ingredient::totalInventory, lineEdit_inventory);

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SMART_FIELD_INIT(MiscEditor, label_producer , lineEdit_producer , Misc, PropertyNames::Misc::producer   );
   SMART_FIELD_INIT(MiscEditor, label_productId, lineEdit_productId, Misc, PropertyNames::Misc::productId  );

   this->connectSignalsAndSlots();
   return;
}

MiscEditor::~MiscEditor() = default;

void MiscEditor::writeFieldsToEditItem() {
   this->m_editItem->setType(this->comboBox_type->getNonOptValue<Misc::Type>());
   this->m_editItem->setName          (this->lineEdit_name          ->text                  ());
   this->m_editItem->setUseFor        (this->textEdit_useFor        ->toPlainText           ());
   this->m_editItem->setNotes         (this->textEdit_notes         ->toPlainText           ());
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   this->m_editItem->setProducer      (this->lineEdit_producer      ->text                  ());
   this->m_editItem->setProductId     (this->lineEdit_productId     ->text                  ());
   return;
}

void MiscEditor::writeLateFieldsToEditItem() {
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

void MiscEditor::readFieldsFromEditItem(std::optional<QString> propName) {
   if (!propName || *propName == PropertyNames::NamedEntity::name) { this->lineEdit_name     ->setTextCursor(m_editItem->name     ()); // Continues to next line
                                                                     this->tabWidget_editor->setTabText(0, m_editItem->name());        if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Misc::type       ) { this->comboBox_type     ->setValue     (m_editItem->type     ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Ingredient::totalInventory) { this->lineEdit_inventory->setAmount(m_editItem->totalInventory());
                                                                              this->comboBox_amountType->autoSetFromControlledField();
                                                                              if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Misc::useFor     ) { this->textEdit_useFor   ->setPlainText (m_editItem->useFor   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Misc::notes      ) { this->textEdit_notes    ->setPlainText (m_editItem->notes    ()); if (propName) { return; } }
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   if (!propName || *propName == PropertyNames::Misc::producer   ) { this->lineEdit_producer ->setTextCursor(m_editItem->producer ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Misc::productId  ) { this->lineEdit_productId->setTextCursor(m_editItem->productId()); if (propName) { return; } }

   this->label_id_value->setText(QString::number(m_editItem->key()));
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_SLOT_DEFINITIONS(MiscEditor)
