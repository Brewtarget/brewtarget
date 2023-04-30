/*
 * MiscEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
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
#include "MiscEditor.h"

#include <QtGui>
#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Misc.h"

MiscEditor::MiscEditor(QWidget * parent) :
   QDialog(parent),
   obsMisc(nullptr) {
   setupUi(this);

   tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   SMART_FIELD_INIT(MiscEditor, label_name     , lineEdit_name     , Misc, PropertyNames::NamedEntity::name);
   SMART_FIELD_INIT(MiscEditor, label_inventory, lineEdit_inventory, Misc, PropertyNames::Misc::amount     );
   SMART_FIELD_INIT(MiscEditor, label_time     , lineEdit_time     , Misc, PropertyNames::Misc::time       );

   // Note, per https://wiki.qt.io/New_Signal_Slot_Syntax#Default_arguments_in_slot, the use of a trivial lambda
   // function to allow use of default argument on newStyle() slot
   connect(pushButton_new   , &QAbstractButton::clicked, this, [this]() { this->newMisc(); return; } );
   connect(pushButton_save,   &QAbstractButton::clicked, this, &MiscEditor::save                     );
   connect(pushButton_cancel, &QAbstractButton::clicked, this, &MiscEditor::clearAndClose            );
   connect(checkBox_isWeight, &QCheckBox::toggled,       this, &MiscEditor::setIsWeight              );

   return;
}

MiscEditor::~MiscEditor() = default;

void MiscEditor::setMisc(Misc * m) {
   if (obsMisc) {
      disconnect(obsMisc, nullptr, this, nullptr);
   }

   obsMisc = m;
   if (obsMisc) {
      connect(obsMisc, &NamedEntity::changed, this, &MiscEditor::changed);
      showChanges();
   }
   return;
}

void MiscEditor::save() {
   if (this->obsMisc == nullptr) {
      setVisible(false);
      return;
   }

   qDebug() << Q_FUNC_INFO << comboBox_type->currentIndex();
   qDebug() << Q_FUNC_INFO << comboBox_use->currentIndex();

   this->obsMisc->setName          (lineEdit_name->text());
   this->obsMisc->setType          (static_cast<Misc::Type>(comboBox_type->currentIndex()));
   this->obsMisc->setUse           (static_cast<Misc::Use>(comboBox_use->currentIndex()));
   this->obsMisc->setTime          (lineEdit_time->toCanonical().quantity());
   this->obsMisc->setAmountIsWeight(checkBox_isWeight->checkState() == Qt::Checked);
   this->obsMisc->setUseFor        (textEdit_useFor->toPlainText());
   this->obsMisc->setNotes         (textEdit_notes->toPlainText());

   if (this->obsMisc->key() < 0) {
      qDebug() << Q_FUNC_INFO << "Inserting into database";
      ObjectStoreWrapper::insert(*this->obsMisc);
   }
   // do this late to make sure we've the row in the inventory table
   this->obsMisc->setInventoryAmount(lineEdit_inventory->toCanonical().quantity());
   setVisible(false);
   return;
}

void MiscEditor::clearAndClose() {
   setMisc(nullptr);
   setVisible(false); // Hide the window.
}

void MiscEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == obsMisc) {
      showChanges(&prop);
   }
}

void MiscEditor::showChanges(QMetaProperty * metaProp) {
   if (obsMisc == nullptr) {
      return;
   }

   QString propName;
   QVariant value;
   bool updateAll = false;
   if (metaProp == nullptr) {
      updateAll = true;
   } else {
      propName = metaProp->name();
      value = metaProp->read(obsMisc);
   }

   if (propName == PropertyNames::NamedEntity::name || updateAll) {
      lineEdit_name->setText(obsMisc->name());
      lineEdit_name->setCursorPosition(0);
      tabWidget_editor->setTabText(0, obsMisc->name());
      if (!updateAll) {
         return;
      }
   }
   if (updateAll || propName == PropertyNames::Misc::type                         ) { this->comboBox_type     ->setCurrentIndex(static_cast<int>(this->obsMisc->type())); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Misc::use                          ) { this->comboBox_use      ->setCurrentIndex(static_cast<int>(this->obsMisc->use()) ); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Misc::time                         ) { this->lineEdit_time     ->setAmount      (obsMisc->time()                        ); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Misc::amountIsWeight               ) { this->checkBox_isWeight ->setCheckState  (obsMisc->amountIsWeight() ? // Continues to next line
                                                                                                                                Qt::Checked : Qt::Unchecked            ); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::NamedEntityWithInventory::inventory) { this->lineEdit_inventory->setAmount      (obsMisc->inventory()                   ); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Misc::useFor                       ) { this->textEdit_useFor   ->setPlainText   (obsMisc->useFor()                      ); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Misc::notes                        ) { this->textEdit_notes    ->setPlainText   (obsMisc->notes()                       ); if (!updateAll) { return; } }
}

void MiscEditor::newMisc(QString folder) {
   QString name = QInputDialog::getText(this, tr("Misc name"), tr("Misc name:"));
   if (name.isEmpty()) {
      return;
   }

   // .:TODO:. This leads to a memory leak in clearAndClose().  Change to shared_ptr
   Misc * m = new Misc(name);

   if (! folder.isEmpty()) {
      m->setFolder(folder);
   }

   setMisc(m);
   show();
   return;
}

void MiscEditor::setIsWeight(bool const state) {
   qDebug() << Q_FUNC_INFO << "state is" << state;
   // But you have to admit, this is clever
   this->lineEdit_inventory->selectPhysicalQuantity(
      state ? Measurement::PhysicalQuantity::Mass : Measurement::PhysicalQuantity::Volume
   );

   // maybe? My head hurts now
   this->lineEdit_inventory->onLineChanged();

   // Strictly, if we change a Misc to be measured by mass instead of volume (or vice versa) we should also somehow tell
   // any other bit of the UI that is showing that Misc (eg a RecipeEditor or MainWindow) to redisplay the relevant
   // field.  Currently we don't do this, on the assumption that it's rare you will change how a Misc is measured after
   // you started using it in recipes.
   return;
}
