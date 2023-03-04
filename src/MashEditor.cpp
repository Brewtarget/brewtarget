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

   connect(pushButton_fromEquipment, &QAbstractButton::clicked, this, &MashEditor::fromEquipment );
   connect(this, &QDialog::accepted, this, &MashEditor::saveAndClose );
   connect(this, &QDialog::rejected, this, &MashEditor::closeEditor );
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

   mashObs->setEquipAdjust(true); // BeerXML won't like me, but it's just stupid not to adjust for the equipment when you're able.

   mashObs->setName(lineEdit_name->text());
   mashObs->setGrainTemp_c(lineEdit_grainTemp->toCanonical().quantity());
   mashObs->setSpargeTemp_c(lineEdit_spargeTemp->toCanonical().quantity());
   mashObs->setPh(lineEdit_spargePh->toCanonical().quantity());
   mashObs->setTunTemp_c(lineEdit_tunTemp->toCanonical().quantity());
   mashObs->setTunWeight_kg(lineEdit_tunMass->toCanonical().quantity());
   mashObs->setTunSpecificHeat_calGC(lineEdit_tunSpHeat->toCanonical().quantity());

   mashObs->setNotes(textEdit_notes->toPlainText());

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

   lineEdit_tunMass->setText(m_equip);
   lineEdit_tunSpHeat->setText(m_equip);
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

void MashEditor::setRecipe(Recipe* r) {
   if ( ! r )
      return;

   this->m_rec = r;
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

   if( mashObs == nullptr )
   {
      clear();
      return;
   }

   if (prop == nullptr) {
      updateAll = true;
   } else {
      propName = prop->name();
   }
   qDebug() << Q_FUNC_INFO << "Updating" << (updateAll ? "all" : "property") << propName;

   if( propName == PropertyNames::NamedEntity::name || updateAll ) {
      lineEdit_name->setText(mashObs->name());
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Mash::grainTemp_c || updateAll ) {
      lineEdit_grainTemp->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Mash::spargeTemp_c || updateAll ) {
      lineEdit_spargeTemp->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Mash::ph || updateAll ) {
      lineEdit_spargePh->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Mash::tunTemp_c || updateAll ) {
      lineEdit_tunTemp->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Mash::tunWeight_kg || updateAll ) {
      lineEdit_tunMass->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Mash::tunSpecificHeat_calGC || updateAll ) {
      lineEdit_tunSpHeat->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Mash::notes || updateAll ) {
      textEdit_notes->setPlainText(mashObs->notes());
      if( ! updateAll )
         return;
   }
}

void MashEditor::clear() {
   lineEdit_name->setText(QString(""));
   lineEdit_grainTemp->setText(QString(""));
   lineEdit_spargeTemp->setText(QString(""));
   lineEdit_spargePh->setText(QString(""));
   lineEdit_tunTemp->setText(QString(""));
   lineEdit_tunMass->setText(QString(""));
   lineEdit_tunSpHeat->setText(QString(""));

   textEdit_notes->setPlainText(QString(""));
   return;
}
