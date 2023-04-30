/*
 * HopEditor.cpp is part of Brewtarget, and is Copyright the following
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

   SMART_FIELD_INIT(HopEditor, label_name                 , lineEdit_name                 , Hop, PropertyNames::NamedEntity::name            );
   SMART_FIELD_INIT(HopEditor, label_alpha                , lineEdit_alpha                , Hop, PropertyNames::Hop::alpha_pct            , 0);
   SMART_FIELD_INIT(HopEditor, label_inventory            , lineEdit_inventory            , Hop, PropertyNames::Hop::amount_kg               );
   SMART_FIELD_INIT(HopEditor, label_time                 , lineEdit_time                 , Hop, PropertyNames::Hop::time_min             , 0);
   SMART_FIELD_INIT(HopEditor, label_beta                 , lineEdit_beta                 , Hop, PropertyNames::Hop::beta_pct             , 0);
   SMART_FIELD_INIT(HopEditor, label_HSI                  , lineEdit_HSI                  , Hop, PropertyNames::Hop::hsi_pct              , 0);
   SMART_FIELD_INIT(HopEditor, label_origin               , lineEdit_origin               , Hop, PropertyNames::Hop::origin                  );
   SMART_FIELD_INIT(HopEditor, label_humulene             , lineEdit_humulene             , Hop, PropertyNames::Hop::humulene_pct         , 0);
   SMART_FIELD_INIT(HopEditor, label_caryophyllene        , lineEdit_caryophyllene        , Hop, PropertyNames::Hop::caryophyllene_pct    , 0);
   SMART_FIELD_INIT(HopEditor, label_cohumulone           , lineEdit_cohumulone           , Hop, PropertyNames::Hop::cohumulone_pct       , 0);
   SMART_FIELD_INIT(HopEditor, label_myrcene              , lineEdit_myrcene              , Hop, PropertyNames::Hop::myrcene_pct          , 0);

   //
   // According to https://bugreports.qt.io/browse/QTBUG-50823 it is never going to be possible to specify the data (as
   // opposed to display text) for a combo box via the .ui file.  So we have to do it in code instead.
   // We could use the raw enum values as the data, but it would be a bit painful to debug if we ever had to, so for
   // small extra effort we use the same serialisation strings that we use for BeerJSON and the DB.
   //
   this->comboBox_hopType->addItem(tr("Bittering"                ), Hop::typeStringMapping.enumToString(Hop::Type::Bittering              ));
   this->comboBox_hopType->addItem(tr("Aroma"                    ), Hop::typeStringMapping.enumToString(Hop::Type::Aroma                  ));
   this->comboBox_hopType->addItem(tr("Aroma & Bittering"        ), Hop::typeStringMapping.enumToString(Hop::Type::Both                   ));

   this->comboBox_hopForm->addItem(tr("Leaf"      ), Hop::formStringMapping.enumToString(Hop::Form::Leaf   ));
   this->comboBox_hopForm->addItem(tr("Pellet"    ), Hop::formStringMapping.enumToString(Hop::Form::Pellet ));
   this->comboBox_hopForm->addItem(tr("Plug"      ), Hop::formStringMapping.enumToString(Hop::Form::Plug   ));

   // Same comment for hop use, even thought it's not stored in BeerJSON
   this->comboBox_hopUse->addItem(tr("Mash"      ), Hop::useStringMapping.enumToString(Hop::Use::Mash      ));
   this->comboBox_hopUse->addItem(tr("First Wort"), Hop::useStringMapping.enumToString(Hop::Use::First_Wort));
   this->comboBox_hopUse->addItem(tr("Boil"      ), Hop::useStringMapping.enumToString(Hop::Use::Boil      ));
   this->comboBox_hopUse->addItem(tr("Post-Boil" ), Hop::useStringMapping.enumToString(Hop::Use::Aroma     ));
   this->comboBox_hopUse->addItem(tr("Dry Hop"   ), Hop::useStringMapping.enumToString(Hop::Use::Dry_Hop   ));

   connect(this->pushButton_new,    &QAbstractButton::clicked, this, &HopEditor::clickedNewHop);
   connect(this->pushButton_save,   &QAbstractButton::clicked, this, &HopEditor::save);
   connect(this->pushButton_cancel, &QAbstractButton::clicked, this, &HopEditor::clearAndClose);

   return;
}

HopEditor::~HopEditor() = default;

void HopEditor::setHop(Hop * h) {
   if (obsHop) {
      disconnect(obsHop, nullptr, this, nullptr);
   }

   obsHop = h;
   if (obsHop) {
      connect(obsHop, &NamedEntity::changed, this, &HopEditor::changed);
      showChanges();
   }
   return;
}

void HopEditor::save() {
   if (!this->obsHop) {
      setVisible(false);
      return;
   }

   this->obsHop->setName             (this->lineEdit_name         ->text                  ());
   this->obsHop->setAlpha_pct        (this->lineEdit_alpha        ->getValueAs<double>    ());
   this->obsHop->setTime_min         (this->lineEdit_time         ->toCanonical().quantity());
   this->obsHop->setBeta_pct         (this->lineEdit_beta         ->getValueAs<double>    ());
   this->obsHop->setHsi_pct          (this->lineEdit_HSI          ->getValueAs<double>    ());
   this->obsHop->setOrigin           (this->lineEdit_origin       ->text                  ());
   this->obsHop->setHumulene_pct     (this->lineEdit_humulene     ->getValueAs<double>    ());
   this->obsHop->setCaryophyllene_pct(this->lineEdit_caryophyllene->getValueAs<double>    ());
   this->obsHop->setCohumulone_pct   (this->lineEdit_cohumulone   ->getValueAs<double>    ());
   this->obsHop->setMyrcene_pct      (this->lineEdit_myrcene      ->getValueAs<double>    ());
   this->obsHop->setSubstitutes      (this->textEdit_substitutes  ->toPlainText           ());
   this->obsHop->setNotes            (this->textEdit_notes        ->toPlainText           ());

   //
   // It's a coding error if we don't recognise the values in our own combo boxes, so it's OK that we'd get a
   // std::bad_optional_access exception in such a case
   //
   this->obsHop->setType(Hop::typeStringMapping.stringToEnum<Hop::Type>(comboBox_hopType->currentData().toString()));
   this->obsHop->setForm(Hop::formStringMapping.stringToEnum<Hop::Form>(comboBox_hopForm->currentData().toString()));
   this->obsHop->setUse (Hop::useStringMapping.stringToEnum<Hop::Use>  (comboBox_hopUse->currentData().toString()));

   if (this->obsHop->key() < 0) {
      ObjectStoreWrapper::insert(*this->obsHop);
   }

   // do this late to make sure we've the row in the inventory table
   this->obsHop->setInventoryAmount(lineEdit_inventory->toCanonical().quantity());
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
   if (updateAll || propName == PropertyNames::NamedEntity::name                  ) { this->lineEdit_name                 ->setText     (obsHop->name                 ()); // Continues to next line
                                                                                      this->lineEdit_name                 ->setCursorPosition(0);                          // Continues to next line
                                                                                      this->tabWidget_editor              ->setTabText(0, obsHop->name());                 if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::origin                        ) { this->lineEdit_origin               ->setText     (obsHop->origin               ()); // Continues to next line
                                                                                      this->lineEdit_origin               ->setCursorPosition(0);                          if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::alpha_pct                     ) { this->lineEdit_alpha                ->setAmount   (obsHop->alpha_pct            ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::time_min                      ) { this->lineEdit_time                 ->setAmount   (obsHop->time_min             ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::beta_pct                      ) { this->lineEdit_beta                 ->setAmount   (obsHop->beta_pct             ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::hsi_pct                       ) { this->lineEdit_HSI                  ->setAmount   (obsHop->hsi_pct              ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::humulene_pct                  ) { this->lineEdit_humulene             ->setAmount   (obsHop->humulene_pct         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::caryophyllene_pct             ) { this->lineEdit_caryophyllene        ->setAmount   (obsHop->caryophyllene_pct    ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::cohumulone_pct                ) { this->lineEdit_cohumulone           ->setAmount   (obsHop->cohumulone_pct       ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::myrcene_pct                   ) { this->lineEdit_myrcene              ->setAmount   (obsHop->myrcene_pct          ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::substitutes                   ) { this->textEdit_substitutes          ->setPlainText(obsHop->substitutes          ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Hop::notes                         ) { this->textEdit_notes                ->setPlainText(obsHop->notes                ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::NamedEntityWithInventory::inventory) { this->lineEdit_inventory            ->setAmount   (obsHop->inventory            ()); if (!updateAll) { return; } }

   return;
}


void HopEditor::newHop(QString folder) {
   QString name = QInputDialog::getText(this, tr("Hop name"), tr("Hop name:"));
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

void HopEditor::clickedNewHop() {
   newHop(QString());
   return;
}
