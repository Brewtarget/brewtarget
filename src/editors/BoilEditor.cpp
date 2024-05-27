/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/BoilEditor.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "editors/BoilEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "model/Boil.h"
#include "model/Recipe.h"

BoilEditor::BoilEditor(QWidget* parent) : QDialog(parent), m_boilObs{nullptr} {
   setupUi(this);

   // NB: label_description / textEdit_description don't need initialisation here as neither is a smart field
   // NB: label_notes / textEdit_notes don't need initialisation here as neither is a smart field
   SMART_FIELD_INIT(BoilEditor, label_name       , lineEdit_name       , Boil, PropertyNames::NamedEntity::name  );
   SMART_FIELD_INIT(BoilEditor, label_preBoilSize, lineEdit_preBoilSize, Boil, PropertyNames::Boil::preBoilSize_l, 1);
   SMART_FIELD_INIT(BoilEditor, label_boilTime   , lineEdit_boilTime   , Boil, PropertyNames::Boil::boilTime_mins, 0);

   connect(this, &QDialog::accepted, this, &BoilEditor::saveAndClose);
   connect(this, &QDialog::rejected, this, &BoilEditor::closeEditor );
   return;
}

BoilEditor::~BoilEditor() = default;

void BoilEditor::showEditor() {
   showChanges();
   setVisible(true);
   return;
}

void BoilEditor::closeEditor() {
   setVisible(false);
   return;
}

void BoilEditor::saveAndClose() {
   bool isNew = false;

   if (!this->m_boilObs) {
      this->m_boilObs = std::make_shared<Boil>(lineEdit_name->text());
      isNew = true;
   }
   qDebug() << Q_FUNC_INFO << "Saving" << (isNew ? "new" : "existing") << "boil (#" << this->m_boilObs->key() << ")";

   this->m_boilObs->setName         (this->lineEdit_name       ->text                 ());
   this->m_boilObs->setDescription  (this->textEdit_description->toPlainText          ());
   this->m_boilObs->setPreBoilSize_l(this->lineEdit_preBoilSize->getOptCanonicalQty   ());
   this->m_boilObs->setBoilTime_mins(this->lineEdit_boilTime   ->getNonOptCanonicalQty());
   this->m_boilObs->setNotes        (this->textEdit_notes      ->toPlainText          ());

   if (isNew) {
      ObjectStoreWrapper::insert(*this->m_boilObs);
      this->m_rec->setBoil(this->m_boilObs);
   }

   return;
}

void BoilEditor::setBoil(std::shared_ptr<Boil> boil) {
   if (this->m_boilObs) {
      disconnect(this->m_boilObs.get(), nullptr, this, nullptr);
   }

   this->m_boilObs = boil;
   if (this->m_boilObs) {
      connect(this->m_boilObs.get(), &NamedEntity::changed, this, &BoilEditor::changed);
      showChanges();
   }
   return;
}

void BoilEditor::setRecipe(Recipe * recipe) {
   if (!recipe) {
      return;
   }

   this->m_rec = recipe;

   return;
}

void BoilEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (!this->m_boilObs) {
      return;
   }


   if (sender() == this->m_boilObs.get()) {
      this->showChanges(&prop);
   }

   if (sender() == this->m_rec) {
      this->showChanges();
   }
   return;
}

void BoilEditor::showChanges(QMetaProperty* prop) {
   if (!this->m_boilObs) {
      this->clear();
      return;
   }

   QString propName;
   bool updateAll = false;
   if (prop == nullptr) {
      updateAll = true;
   } else {
      propName = prop->name();
   }
   qDebug() << Q_FUNC_INFO << "Updating" << (updateAll ? "all" : "property") << propName;

   if (updateAll || propName == PropertyNames::NamedEntity::name  ) {this->lineEdit_name       ->setText     (this->m_boilObs->name         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Boil::description  ) {this->textEdit_description->setPlainText(this->m_boilObs->description  ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Boil::preBoilSize_l) {this->lineEdit_preBoilSize->setQuantity (this->m_boilObs->preBoilSize_l()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Boil::boilTime_mins) {this->lineEdit_boilTime   ->setQuantity (this->m_boilObs->boilTime_mins()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Boil::notes        ) {this->textEdit_notes      ->setPlainText(this->m_boilObs->notes        ()); if (!updateAll) { return; } }
   return;
}

void BoilEditor::clear() {
   this->lineEdit_name       ->setText     ("");
   this->textEdit_description->setText     ("");
   this->lineEdit_preBoilSize->setText     ("");
   this->lineEdit_boilTime   ->setText     ("");
   this->textEdit_notes      ->setPlainText("");
   return;
}
