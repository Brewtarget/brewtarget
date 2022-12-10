/*
 * HopEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#include "HopEditor.h"

#include <QtGui>
#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Hop.h"

HopEditor::HopEditor(QWidget * parent) :
   QDialog(parent),
   obsHop(nullptr) {
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   //
   // According to https://bugreports.qt.io/browse/QTBUG-50823 it is never going to be possible to specify the data (as
   // opposed to display text) for a combo box via the .ui file.  So we have to do it in code instead.
   // We could use the raw enum values as the data, but it would be a bit painful to debug if we ever had to, so for
   // small extra effort we use the same serialisation strings that we use for BeerJSON and the DB.
   //
   this->comboBox_hopType->addItem(tr("Bittering"                ), Hop::typeStringMapping.enumToString(Hop::Type::Bittering              ));
   this->comboBox_hopType->addItem(tr("Aroma"                    ), Hop::typeStringMapping.enumToString(Hop::Type::Aroma                  ));
   this->comboBox_hopType->addItem(tr("Aroma & Bittering"        ), Hop::typeStringMapping.enumToString(Hop::Type::AromaAndBittering      ));
   this->comboBox_hopType->addItem(tr("Flavor"                   ), Hop::typeStringMapping.enumToString(Hop::Type::Flavor                 ));
   this->comboBox_hopType->addItem(tr("Bittering & Flavor"       ), Hop::typeStringMapping.enumToString(Hop::Type::BitteringAndFlavor     ));
   this->comboBox_hopType->addItem(tr("Aroma & Flavor"           ), Hop::typeStringMapping.enumToString(Hop::Type::AromaAndFlavor         ));
   this->comboBox_hopType->addItem(tr("Aroma, Bittering & Flavor"), Hop::typeStringMapping.enumToString(Hop::Type::AromaBitteringAndFlavor));

   this->comboBox_hopForm->addItem(tr("Extract"   ), Hop::formStringMapping.enumToString(Hop::Form::Extract));
   this->comboBox_hopForm->addItem(tr("Leaf"      ), Hop::formStringMapping.enumToString(Hop::Form::Leaf   ));
   this->comboBox_hopForm->addItem(tr("Leaf (wet)"), Hop::formStringMapping.enumToString(Hop::Form::WetLeaf));
   this->comboBox_hopForm->addItem(tr("Pellet"    ), Hop::formStringMapping.enumToString(Hop::Form::Pellet ));
   this->comboBox_hopForm->addItem(tr("Powder"    ), Hop::formStringMapping.enumToString(Hop::Form::Powder ));
   this->comboBox_hopForm->addItem(tr("Plug"      ), Hop::formStringMapping.enumToString(Hop::Form::Plug   ));

   // Same comment for hop use, even thought it's not stored in BeerJSON
   this->comboBox_hopUse->addItem(tr("Mash"      ), Hop::useStringMapping.enumToString(Hop::Use::Mash      ));
   this->comboBox_hopUse->addItem(tr("First Wort"), Hop::useStringMapping.enumToString(Hop::Use::First_Wort));
   this->comboBox_hopUse->addItem(tr("Boil"      ), Hop::useStringMapping.enumToString(Hop::Use::Boil      ));
   this->comboBox_hopUse->addItem(tr("Post-Boil" ), Hop::useStringMapping.enumToString(Hop::Use::Aroma     ));
   this->comboBox_hopUse->addItem(tr("Dry Hop"   ), Hop::useStringMapping.enumToString(Hop::Use::Dry_Hop   ));

   connect(pushButton_new, SIGNAL(clicked()), this, SLOT(newHop()));
   connect(pushButton_save,   &QAbstractButton::clicked, this, &HopEditor::save);
   connect(pushButton_cancel, &QAbstractButton::clicked, this, &HopEditor::clearAndClose);

   return;
}

void HopEditor::setHop(Hop * h) {
   if (obsHop) {
      disconnect(obsHop, nullptr, this, nullptr);
   }

   obsHop = h;
   if (obsHop) {
      connect(obsHop, &NamedEntity::changed, this, &HopEditor::changed);
      showChanges();
   }
}

void HopEditor::save() {
   if (!this->obsHop) {
      setVisible(false);
      return;
   }

   this->obsHop->setName(lineEdit_name->text());
   this->obsHop->setAlpha_pct(lineEdit_alpha->toSI().quantity);
   this->obsHop->setTime_min(lineEdit_time->toSI().quantity);

   //
   // It's a coding error if we don't recognise the values in our own combo boxes, so it's OK that we'd get a
   // std::bad_optional_access exception in such a case
   //
   this->obsHop->setUse (Hop::useStringMapping.stringToEnum<Hop::Use>  (comboBox_hopUse->currentData().toString()));
   this->obsHop->setType(Hop::typeStringMapping.stringToEnum<Hop::Type>(comboBox_hopType->currentData().toString()));
   this->obsHop->setForm(Hop::formStringMapping.stringToEnum<Hop::Form>(comboBox_hopForm->currentData().toString()));

   this->obsHop->setBeta_pct(lineEdit_beta->toSI().quantity);
   this->obsHop->setHsi_pct(lineEdit_HSI->toSI().quantity);
   this->obsHop->setOrigin(lineEdit_origin->text());
   this->obsHop->setHumulene_pct(lineEdit_humulene->toSI().quantity);
   this->obsHop->setCaryophyllene_pct(lineEdit_caryophyllene->toSI().quantity);
   this->obsHop->setCohumulone_pct(lineEdit_cohumulone->toSI().quantity);
   this->obsHop->setMyrcene_pct(lineEdit_myrcene->toSI().quantity);

   this->obsHop->setSubstitutes(textEdit_substitutes->toPlainText());
   this->obsHop->setNotes(textEdit_notes->toPlainText());

   if (this->obsHop->key() < 0) {
      ObjectStoreWrapper::insert(*this->obsHop);
   }

   // do this late to make sure we've the row in the inventory table
   this->obsHop->setInventoryAmount(lineEdit_inventory->toSI().quantity);
   setVisible(false);
   return;
}

void HopEditor::clearAndClose() {
   setHop(nullptr);
   setVisible(false); // Hide the window.
}

void HopEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == obsHop) {
      showChanges(&prop);
   }
}

void HopEditor::showChanges(QMetaProperty * prop) {
   if (!this->obsHop) {
      return;
   }

   bool updateAll = false;
   QString propName = "";

   if (prop == nullptr) {
      updateAll = true;
   } else {
      propName = prop->name();
   }

   if (updateAll || propName == PropertyNames::Hop::use) {
      // As above, it's a coding error if there isn't a combo box entry corresponding to the Hop type
      comboBox_hopUse->setCurrentIndex(
         comboBox_hopUse->findData(Hop::useStringMapping.enumToString(obsHop->use()))
      );
      if (!updateAll) {
         return;
      }
   }
   if (updateAll || propName == PropertyNames::Hop::type) {
      // As above, it's a coding error if there isn't a combo box entry corresponding to the Hop type
      comboBox_hopType->setCurrentIndex(
         comboBox_hopType->findData(Hop::typeStringMapping.enumToString(obsHop->type()))
      );
      if (!updateAll) {
         return;
      }
   }
   if (updateAll || propName == PropertyNames::Hop::form) {
      // As above, it's a coding error if there isn't a combo box entry corresponding to the Hop form
      comboBox_hopForm->setCurrentIndex(
         comboBox_hopForm->findData(Hop::formStringMapping.enumToString(obsHop->form()))
      );
      if (!updateAll) {
         return;
      }
   }
   if (updateAll || propName == PropertyNames::NamedEntity::name     ) { lineEdit_name         ->setText(obsHop->name());   lineEdit_name  ->setCursorPosition(0); tabWidget_editor->setTabText(0, obsHop->name()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::origin           ) { lineEdit_origin       ->setText(obsHop->origin()); lineEdit_origin->setCursorPosition(0); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::alpha_pct        ) { lineEdit_alpha        ->setText(obsHop); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::time_min         ) { lineEdit_time         ->setText(obsHop); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::beta_pct         ) { lineEdit_beta         ->setText(obsHop); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::hsi_pct          ) { lineEdit_HSI          ->setText(obsHop); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::humulene_pct     ) { lineEdit_humulene     ->setText(obsHop); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::caryophyllene_pct) { lineEdit_caryophyllene->setText(obsHop); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::cohumulone_pct   ) { lineEdit_cohumulone   ->setText(obsHop); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::myrcene_pct      ) { lineEdit_myrcene      ->setText(obsHop); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::substitutes      ) { textEdit_substitutes  ->setPlainText(obsHop->substitutes()); if (!updateAll) { return; }}
   if (updateAll || propName == PropertyNames::Hop::notes            ) { textEdit_notes        ->setPlainText(obsHop->notes());       if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::NamedEntityWithInventory::inventory) { lineEdit_inventory->setText(obsHop); if (!updateAll) { return; } }

   return;
}


void HopEditor::newHop(QString folder) {
   QString name = QInputDialog::getText(this, tr("Hop name"),
                                        tr("Hop name:"));
   if (name.isEmpty()) {
      return;
   }

   // .:TODO:. Change this to shared_ptr as currently results in memory leak in clearAndClose()
   Hop * h = new Hop(name);

   if (! folder.isEmpty()) {
      h->setFolder(folder);
   }

   setHop(h);
   show();
   return;
}

void HopEditor::newHop() {
   newHop(QString());
   return;
}
