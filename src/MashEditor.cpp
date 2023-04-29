/*
 * MashEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "MashEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/Recipe.h"

MashEditor::MashEditor(QWidget* parent) : QDialog(parent), mashObs(nullptr) {
   setupUi(this);

   SMART_FIELD_INIT(MashEditor, label_name      , lineEdit_name      , Mash, PropertyNames::NamedEntity::name             );
   SMART_FIELD_INIT(MashEditor, label_grainTemp , lineEdit_grainTemp , Mash, PropertyNames::Mash::grainTemp_c          , 1);
   SMART_FIELD_INIT(MashEditor, label_spargeTemp, lineEdit_spargeTemp, Mash, PropertyNames::Mash::spargeTemp_c         , 1);
   SMART_FIELD_INIT(MashEditor, label_spargePh  , lineEdit_spargePh  , Mash, PropertyNames::Mash::ph                   , 0);
   SMART_FIELD_INIT(MashEditor, label_tunTemp   , lineEdit_tunTemp   , Mash, PropertyNames::Mash::tunTemp_c            , 1);
   SMART_FIELD_INIT(MashEditor, label_tunMass   , lineEdit_tunMass   , Mash, PropertyNames::Mash::tunWeight_kg            );
   SMART_FIELD_INIT(MashEditor, label_tunSpHeat , lineEdit_tunSpHeat , Mash, PropertyNames::Mash::tunSpecificHeat_calGC, 1);

   connect(pushButton_fromEquipment, &QAbstractButton::clicked, this, &MashEditor::fromEquipment);
   connect(this,                     &QDialog::accepted,        this, &MashEditor::saveAndClose );
   connect(this,                     &QDialog::rejected,        this, &MashEditor::closeEditor  );
   return;
}

void MashEditor::showEditor() {
   showChanges();
   setVisible(true);
   return;
}

void MashEditor::closeEditor() {
   setVisible(false);
   return;
}

void MashEditor::saveAndClose() {
   bool isNew = false;

   if (this->mashObs == nullptr) {
      this->mashObs = new Mash(lineEdit_name->text());
      isNew = true;
   }
   qDebug() << Q_FUNC_INFO << "Saving" << (isNew ? "new" : "existing") << "mash (#" << this->mashObs->key() << ")";

   this->mashObs->setEquipAdjust(true); // BeerXML won't like me, but it's just stupid not to adjust for the equipment when you're able.
   this->mashObs->setName                 (this->lineEdit_name      ->text()                  );
   this->mashObs->setGrainTemp_c          (this->lineEdit_grainTemp ->toCanonical().quantity());
   this->mashObs->setSpargeTemp_c         (this->lineEdit_spargeTemp->toCanonical().quantity());
   this->mashObs->setPh                   (this->lineEdit_spargePh  ->toCanonical().quantity());
   this->mashObs->setTunTemp_c            (this->lineEdit_tunTemp   ->toCanonical().quantity());
   this->mashObs->setTunWeight_kg         (this->lineEdit_tunMass   ->toCanonical().quantity());
   this->mashObs->setTunSpecificHeat_calGC(this->lineEdit_tunSpHeat ->toCanonical().quantity());
   this->mashObs->setNotes                (this->textEdit_notes     ->toPlainText()           );

   if (isNew) {
      ObjectStoreWrapper::insert(*mashObs);
      this->m_rec->setMash(this->mashObs);
   }

   return;
}

void MashEditor::fromEquipment() {
   if (this->mashObs == nullptr) {
      return;
   }

   if (this->m_equip == nullptr) {
      return;
   }

   lineEdit_tunMass  ->setAmount(this->m_equip->tunWeight_kg         ());
   lineEdit_tunSpHeat->setAmount(this->m_equip->tunSpecificHeat_calGC());
   return;
}

void MashEditor::setMash(Mash* mash) {
   if (mashObs) {
      disconnect( mashObs, nullptr, this, nullptr );
   }

   mashObs = mash;
   if( mashObs )
   {
      connect( mashObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
   return;
}

void MashEditor::setRecipe(Recipe * recipe) {
   if (!recipe) {
      return;
   }

   this->m_rec = recipe;
   this->m_equip = this->m_rec->equipment();

   if (this->mashObs && this->m_equip) {
      // Only do this if we have to. Otherwise, it causes some unnecessary updates to the database.
      if (this->mashObs->tunWeight_kg() != this->m_equip->tunWeight_kg()) {
         qDebug() <<
            Q_FUNC_INFO << "Overwriting mash tunWeight_kg (" << this->mashObs->tunWeight_kg() << ") with equipment "
            "tunWeight_kg (" << this->m_equip->tunWeight_kg() << ")";
         this->mashObs->setTunWeight_kg(this->m_equip->tunWeight_kg());
      }
      if (this->mashObs->tunSpecificHeat_calGC() != this->m_equip->tunSpecificHeat_calGC() ) {
         qDebug() <<
            Q_FUNC_INFO << "Overwriting mash tunSpecificHeat_calGC (" << this->mashObs->tunSpecificHeat_calGC() << ") "
            "with equipment tunSpecificHeat_calGC (" << this->m_equip->tunSpecificHeat_calGC() << ")";
         this->mashObs->setTunSpecificHeat_calGC(this->m_equip->tunSpecificHeat_calGC());
      }
   }
   return;
}

void MashEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == this->mashObs ) {
      this->showChanges(&prop);
   }

   if (sender() == this->m_rec) {
      this->m_equip = this->m_rec->equipment();
      this->showChanges();
   }
   return;
}

void MashEditor::showChanges(QMetaProperty* prop) {
   bool updateAll = false;
   QString propName;

   if (mashObs == nullptr) {
      clear();
      return;
   }

   if (prop == nullptr) {
      updateAll = true;
   } else {
      propName = prop->name();
   }
   qDebug() << Q_FUNC_INFO << "Updating" << (updateAll ? "all" : "property") << propName;

   if (updateAll || propName == PropertyNames::NamedEntity::name           ) {this->lineEdit_name      ->setText     (mashObs->name                 ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::grainTemp_c           ) {this->lineEdit_grainTemp ->setAmount   (mashObs->grainTemp_c          ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::spargeTemp_c          ) {this->lineEdit_spargeTemp->setAmount   (mashObs->spargeTemp_c         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::ph                    ) {this->lineEdit_spargePh  ->setAmount   (mashObs->ph                   ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::tunTemp_c             ) {this->lineEdit_tunTemp   ->setAmount   (mashObs->tunTemp_c            ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::tunWeight_kg          ) {this->lineEdit_tunMass   ->setAmount   (mashObs->tunWeight_kg         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::tunSpecificHeat_calGC ) {this->lineEdit_tunSpHeat ->setAmount   (mashObs->tunSpecificHeat_calGC()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::notes                 ) {this->textEdit_notes     ->setPlainText(mashObs->notes                ()); if (!updateAll) { return; } }
   return;
}

void MashEditor::clear() {
   this->lineEdit_name      ->setText     (QString(""));
   this->lineEdit_grainTemp ->setText     (QString(""));
   this->lineEdit_spargeTemp->setText     (QString(""));
   this->lineEdit_spargePh  ->setText     (QString(""));
   this->lineEdit_tunTemp   ->setText     (QString(""));
   this->lineEdit_tunMass   ->setText     (QString(""));
   this->lineEdit_tunSpHeat ->setText     (QString(""));
   this->textEdit_notes     ->setPlainText(QString(""));
   return;
}
