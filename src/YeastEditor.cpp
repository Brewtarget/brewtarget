/*
 * YeastEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "YeastEditor.h"

#include <cmath>

#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Yeast.h"

YeastEditor::YeastEditor(QWidget * parent) :
   QDialog(parent),
   obsYeast(nullptr) {
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   SMART_FIELD_INIT(YeastEditor, label_name          , lineEdit_name          , Yeast, PropertyNames::NamedEntity::name         );
   SMART_FIELD_INIT(YeastEditor, label_laboratory    , lineEdit_laboratory    , Yeast, PropertyNames::Yeast::laboratory         );
   SMART_FIELD_INIT(YeastEditor, label_inventory     , lineEdit_inventory     , Yeast, PropertyNames::Yeast::amount          , 0);
   SMART_FIELD_INIT(YeastEditor, label_productID     , lineEdit_productID     , Yeast, PropertyNames::Yeast::productID          );
   SMART_FIELD_INIT(YeastEditor, label_minTemperature, lineEdit_minTemperature, Yeast, PropertyNames::Yeast::minTemperature_c, 1);
   SMART_FIELD_INIT(YeastEditor, label_attenuation   , lineEdit_attenuation   , Yeast, PropertyNames::Yeast::attenuation_pct , 0);
   SMART_FIELD_INIT(YeastEditor, label_maxTemperature, lineEdit_maxTemperature, Yeast, PropertyNames::Yeast::maxTemperature_c, 1);
   SMART_FIELD_INIT(YeastEditor, label_timesCultured , lineEdit_timesCultured , Yeast, PropertyNames::Yeast::timesCultured   , 0);
   SMART_FIELD_INIT(YeastEditor, label_maxReuse      , lineEdit_maxReuse      , Yeast, PropertyNames::Yeast::maxReuse        , 0);

   // Note, per https://wiki.qt.io/New_Signal_Slot_Syntax#Default_arguments_in_slot, the use of a trivial lambda
   // function to allow use of default argument on newYeast() slot
   connect(this->pushButton_new,    &QAbstractButton::clicked, this, [this]() { this->newYeast(); return; } );
   connect(this->pushButton_save,   &QAbstractButton::clicked, this, &YeastEditor::save                     );
   connect(this->pushButton_cancel, &QAbstractButton::clicked, this, &YeastEditor::clearAndClose            );
   return;
}

YeastEditor::~YeastEditor() = default;

void YeastEditor::setYeast(Yeast * y) {
   if (this->obsYeast) {
      disconnect(this->obsYeast, nullptr, this, nullptr);
   }

   this->obsYeast = y;
   if (this->obsYeast) {
      connect(this->obsYeast, SIGNAL(changed(QMetaProperty, QVariant)), this, SLOT(changed(QMetaProperty, QVariant)));
      showChanges();
   }
}

void YeastEditor::save() {
   if (!this->obsYeast) {
      setVisible(false);
      return;
   }

   this->obsYeast->setName            (lineEdit_name          ->text()                                        );
   this->obsYeast->setType            (static_cast<Yeast::Type>(comboBox_type->currentIndex())                );
   this->obsYeast->setForm            (static_cast<Yeast::Form>(comboBox_form->currentIndex())                );
   this->obsYeast->setAmountIsWeight  (checkBox_amountIsWeight->checkState() == Qt::Checked                   );
   this->obsYeast->setLaboratory      (lineEdit_laboratory    ->text()                                        );
   this->obsYeast->setProductID       (lineEdit_productID     ->text()                                        );
   this->obsYeast->setMinTemperature_c(lineEdit_minTemperature->toCanonical().quantity()                      );
   this->obsYeast->setMaxTemperature_c(lineEdit_maxTemperature->toCanonical().quantity()                      );
   this->obsYeast->setFlocculation    (static_cast<Yeast::Flocculation>(comboBox_flocculation->currentIndex()));
   this->obsYeast->setAttenuation_pct (lineEdit_attenuation   ->getNonOptValueAs<double>()                          );
   this->obsYeast->setTimesCultured   (lineEdit_timesCultured ->getNonOptValueAs<int>()                             );
   this->obsYeast->setMaxReuse        (lineEdit_maxReuse      ->getNonOptValueAs<int>()                             );
   this->obsYeast->setAddToSecondary  (checkBox_addToSecondary->checkState() == Qt::Checked                   );
   this->obsYeast->setBestFor         (textEdit_bestFor       ->toPlainText()                                 );
   this->obsYeast->setNotes           (textEdit_notes         ->toPlainText()                                 );

   if (this->obsYeast->key() < 0) {
      ObjectStoreWrapper::insert(*this->obsYeast);
   }
   // do this late to make sure we've the row in the inventory table
   this->obsYeast->setInventoryQuanta(lineEdit_inventory->text().toInt());
   setVisible(false);
   return;
}

void YeastEditor::clearAndClose() {
   setYeast(nullptr);
   setVisible(false); // Hide the window.
}

void YeastEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == obsYeast) {
      showChanges(&prop);
   }
}

void YeastEditor::showChanges(QMetaProperty * metaProp) {
   Yeast * y = obsYeast;
   if (y == nullptr) {
      return;
   }

   QString propName;
   QVariant value;
   bool updateAll = false;
   if (metaProp == nullptr) {
      updateAll = true;
   } else {
      propName = metaProp->name();
      value = metaProp->read(y);
   }

   if (propName == PropertyNames::NamedEntity::name || updateAll) {
      lineEdit_name->setText(obsYeast->name());
      lineEdit_name->setCursorPosition(0);

      tabWidget_editor->setTabText(0, obsYeast->name());
      if (!updateAll) {
         return;
      }
   }
   if (updateAll || propName == PropertyNames::Yeast::type                        ) { this->comboBox_type->setCurrentIndex(static_cast<int>(obsYeast->type()));                                             if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::form                        ) { this->comboBox_form->setCurrentIndex(static_cast<int>(obsYeast->form()));                                             if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::NamedEntityWithInventory::inventory) { this->lineEdit_inventory     ->setAmount   (obsYeast->inventory       ());                                            if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::amountIsWeight              ) { this->checkBox_amountIsWeight->setCheckState((obsYeast->amountIsWeight()) ? Qt::Checked : Qt::Unchecked);             if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::laboratory                  ) { this->lineEdit_laboratory    ->setText     (obsYeast->laboratory      ()); lineEdit_laboratory->setCursorPosition(0); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::productID                   ) { this->lineEdit_productID     ->setText     (obsYeast->productID       ()); lineEdit_productID ->setCursorPosition(0); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::minTemperature_c            ) { this->lineEdit_minTemperature->setAmount   (obsYeast->minTemperature_c());                                            if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::maxTemperature_c            ) { this->lineEdit_maxTemperature->setAmount   (obsYeast->maxTemperature_c());                                            if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::flocculation                ) { this->comboBox_flocculation  ->setCurrentIndex(static_cast<int>(obsYeast->flocculation()));                           if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::attenuation_pct             ) { this->lineEdit_attenuation   ->setAmount   (obsYeast->attenuation_pct ());                                            if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::timesCultured               ) { this->lineEdit_timesCultured ->setAmount   (obsYeast->timesCultured   ());                                            if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::maxReuse                    ) { this->lineEdit_maxReuse      ->setAmount   (obsYeast->maxReuse        ());                                            if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::addToSecondary              ) { this->checkBox_addToSecondary->setCheckState((obsYeast->addToSecondary()) ? Qt::Checked : Qt::Unchecked);             if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::bestFor                     ) { this->textEdit_bestFor       ->setPlainText(obsYeast->bestFor         ());                                            if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Yeast::notes                       ) { this->textEdit_notes         ->setPlainText(obsYeast->notes           ());                                            if (!updateAll) { return; } }
   return;
}

void YeastEditor::newYeast(QString folder) {
   QString name = QInputDialog::getText(this, tr("Yeast name"), tr("Yeast name:"));
   if (name.isEmpty()) {
      return;
   }

   // .:TODO:. Change to shared_ptr as currently leads to memory leak in clearAndClose()
   Yeast * y = new Yeast(name);
   if (! folder.isEmpty()) {
      y->setFolder(folder);
   }

   setYeast(y);
   show();
   return;
}
