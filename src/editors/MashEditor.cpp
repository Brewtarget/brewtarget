/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/MashEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "editors/MashEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/Recipe.h"

MashEditor::MashEditor(QWidget* parent) :
   QDialog(parent),
   m_recipe{nullptr},
   m_mashObs{nullptr} {
   setupUi(this);

   SMART_FIELD_INIT(MashEditor, label_name      , lineEdit_name      , Mash, PropertyNames::NamedEntity::name             );
   SMART_FIELD_INIT(MashEditor, label_grainTemp , lineEdit_grainTemp , Mash, PropertyNames::Mash::grainTemp_c          , 1);
   SMART_FIELD_INIT(MashEditor, label_spargeTemp, lineEdit_spargeTemp, Mash, PropertyNames::Mash::spargeTemp_c         , 1);
   SMART_FIELD_INIT(MashEditor, label_spargePh  , lineEdit_spargePh  , Mash, PropertyNames::Mash::ph                   , 0);
   SMART_FIELD_INIT(MashEditor, label_tunTemp   , lineEdit_tunTemp   , Mash, PropertyNames::Mash::tunTemp_c            , 1);
   SMART_FIELD_INIT(MashEditor, label_tunMass   , lineEdit_tunMass   , Mash, PropertyNames::Mash::mashTunWeight_kg            );
   SMART_FIELD_INIT(MashEditor, label_tunSpHeat , lineEdit_tunSpHeat , Mash, PropertyNames::Mash::mashTunSpecificHeat_calGC, 1);

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

   if (!this->m_mashObs) {
      this->m_mashObs = std::make_shared<Mash>(lineEdit_name->text());
      isNew = true;
   }
   qDebug() << Q_FUNC_INFO << "Saving" << (isNew ? "new" : "existing") << "mash (#" << this->m_mashObs->key() << ")";

   this->m_mashObs->setEquipAdjust(true); // BeerXML won't like me, but it's just stupid not to adjust for the equipment when you're able.
   this->m_mashObs->setName                 (this->lineEdit_name      ->text()                  );
   this->m_mashObs->setGrainTemp_c          (this->lineEdit_grainTemp ->getNonOptCanonicalQty());
   this->m_mashObs->setSpargeTemp_c         (this->lineEdit_spargeTemp->getNonOptCanonicalQty());
   this->m_mashObs->setPh                   (this->lineEdit_spargePh  ->getNonOptCanonicalQty());
   this->m_mashObs->setTunTemp_c            (this->lineEdit_tunTemp   ->getNonOptCanonicalQty());
   this->m_mashObs->setTunWeight_kg         (this->lineEdit_tunMass   ->getNonOptCanonicalQty());
   this->m_mashObs->setMashTunSpecificHeat_calGC(this->lineEdit_tunSpHeat ->getNonOptCanonicalQty());
   this->m_mashObs->setNotes                (this->textEdit_notes     ->toPlainText()           );

   if (isNew) {
      ObjectStoreWrapper::insert(*this->m_mashObs);
      this->m_recipe->setMash(this->m_mashObs);
   }

   return;
}

void MashEditor::fromEquipment() {
   if (!this->m_mashObs) {
      return;
   }

   if (!this->m_recipe) {
      return;
   }

   auto equipment = this->m_recipe->equipment();
   if (!equipment) {
      return;
   }

   lineEdit_tunMass  ->setQuantity(equipment->mashTunWeight_kg         ());
   lineEdit_tunSpHeat->setQuantity(equipment->mashTunSpecificHeat_calGC());
   return;
}

void MashEditor::setMash(std::shared_ptr<Mash> mash) {
   if (this->m_mashObs) {
      disconnect(this->m_mashObs.get(), nullptr, this, nullptr );
   }

   this->m_mashObs = mash;
   if (this->m_mashObs) {
      connect(this->m_mashObs.get(), SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
   return;
}

void MashEditor::setRecipe(Recipe * recipe) {
   if (!recipe) {
      return;
   }

   this->m_recipe = recipe;
   auto equipment = this->m_recipe->equipment();

   if (this->m_mashObs && equipment) {
      // Only do this if we have to. Otherwise, it causes some unnecessary updates to the database.
      if (this->m_mashObs->mashTunWeight_kg() != equipment->mashTunWeight_kg()) {
         qDebug() <<
            Q_FUNC_INFO << "Overwriting mash mashTunWeight_kg (" << this->m_mashObs->mashTunWeight_kg() << ") with equipment "
            "mashTunWeight_kg (" << equipment->mashTunWeight_kg() << ")";
         this->m_mashObs->setTunWeight_kg(equipment->mashTunWeight_kg().value_or(0.0)); // TBD: Maybe Mash::setTunWeight_kg should take an optional value
      }
      if (this->m_mashObs->mashTunSpecificHeat_calGC() != equipment->mashTunSpecificHeat_calGC() ) {
         qDebug() <<
            Q_FUNC_INFO << "Overwriting mash mashTunSpecificHeat_calGC (" << this->m_mashObs->mashTunSpecificHeat_calGC() << ") "
            "with equipment mashTunSpecificHeat_calGC (" << equipment->mashTunSpecificHeat_calGC() << ")";
         this->m_mashObs->setMashTunSpecificHeat_calGC(equipment->mashTunSpecificHeat_calGC().value_or(Equipment::default_mashTunSpecificHeat_calGC));
      }
   }
   return;
}

void MashEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == this->m_mashObs.get()) {
      this->showChanges(&prop);
   }

   if (sender() == this->m_recipe) {
      this->showChanges();
   }
   return;
}

void MashEditor::showChanges(QMetaProperty* prop) {
   if (!this->m_mashObs) {
      this->clear();
      return;
   }

   bool updateAll = false;
   QString propName;

   if (prop == nullptr) {
      updateAll = true;
   } else {
      propName = prop->name();
   }
   qDebug() << Q_FUNC_INFO << "Updating" << (updateAll ? "all" : "property") << propName;

   if (updateAll || propName == PropertyNames::NamedEntity::name     ) {this->lineEdit_name      ->setText    (m_mashObs->name            ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::grainTemp_c     ) {this->lineEdit_grainTemp ->setQuantity(m_mashObs->grainTemp_c     ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::spargeTemp_c    ) {this->lineEdit_spargeTemp->setQuantity(m_mashObs->spargeTemp_c    ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::ph              ) {this->lineEdit_spargePh  ->setQuantity(m_mashObs->ph              ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::tunTemp_c       ) {this->lineEdit_tunTemp   ->setQuantity(m_mashObs->tunTemp_c       ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::mashTunWeight_kg) {this->lineEdit_tunMass   ->setQuantity(m_mashObs->mashTunWeight_kg()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::mashTunSpecificHeat_calGC) {this->lineEdit_tunSpHeat->setQuantity(m_mashObs->mashTunSpecificHeat_calGC()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::notes           ) {this->textEdit_notes     ->setPlainText(m_mashObs->notes          ()); if (!updateAll) { return; } }
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
