/*
 * MashStepEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#include "MashStepEditor.h"

#include "MainWindow.h"
#include "measurement/Unit.h"
#include "model/MashStep.h"

MashStepEditor::MashStepEditor(QWidget* parent) : QDialog{parent}, obs(nullptr) {
   this->setupUi(this);

   this->comboBox_type->setCurrentIndex(-1);

   connect(buttonBox, &QDialogButtonBox::accepted, this, &MashStepEditor::saveAndClose);
   connect(buttonBox, &QDialogButtonBox::rejected, this, &MashStepEditor::close);
   connect(this->comboBox_type, &QComboBox::currentTextChanged, this, &MashStepEditor::grayOutStuff);
   return;
}

void MashStepEditor::showChanges(QMetaProperty* metaProp) {
   if (!this->obs) {
      this->clear();
      return;
   }

   QString propName;
   QVariant value;
   bool updateAll = false;

   if (metaProp == nullptr) {
      updateAll = true;
   } else {
      propName = metaProp->name();
      value = metaProp->read(this->obs.get());
   }

   if (updateAll) {
      lineEdit_name->setText(obs->name());
      comboBox_type->setCurrentIndex(static_cast<int>(obs->type()));
      lineEdit_infuseAmount->setText(this->obs.get());
      lineEdit_infuseTemp->setText(this->obs.get());
      lineEdit_decoctionAmount->setText(this->obs.get());
      lineEdit_stepTemp->setText(this->obs.get());
      lineEdit_stepTime->setText(this->obs.get());
      lineEdit_rampTime->setText(this->obs.get());
      lineEdit_endTemp->setText(this->obs.get());
   } else if (propName == PropertyNames::NamedEntity::name) {
      lineEdit_name->setText(obs->name());
   } else if (propName == PropertyNames::MashStep::type) {
      comboBox_type->setCurrentIndex(static_cast<int>(obs->type()));
   } else if (propName == PropertyNames::MashStep::infuseAmount_l) {
      lineEdit_infuseAmount->setText(this->obs.get());
   } else if (propName == PropertyNames::MashStep::infuseTemp_c) {
      lineEdit_infuseTemp->setText(this->obs.get());
   } else if (propName == PropertyNames::MashStep::decoctionAmount_l) {
      lineEdit_decoctionAmount->setText(this->obs.get());
   } else if (propName == PropertyNames::MashStep::stepTemp_c) {
      lineEdit_stepTemp->setText(this->obs.get());
   } else if (propName == PropertyNames::MashStep::stepTime_min) {
      lineEdit_stepTime->setText(this->obs.get());
   } else if (propName == PropertyNames::MashStep::rampTime_min) {
      lineEdit_rampTime->setText(this->obs.get());
   } else if (propName == PropertyNames::MashStep::endTemp_c) {
      lineEdit_endTemp->setText(this->obs.get());
   }
   return;
}

void MashStepEditor::clear() {
   lineEdit_name->setText(QString(""));
   comboBox_type->setCurrentIndex(0);
   lineEdit_infuseAmount->setText(QString(""));
   lineEdit_infuseTemp->setText(QString(""));
   lineEdit_decoctionAmount->setText(QString(""));
   lineEdit_stepTemp->setText(QString(""));
   lineEdit_stepTime->setText(QString(""));
   lineEdit_rampTime->setText(QString(""));
   lineEdit_endTemp->setText(QString(""));
   return;
}

void MashStepEditor::close() {
   setVisible(false);
   return;
}

void MashStepEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() != this->obs.get()) {
      return;
   }

   showChanges(&prop);
   return;
}

void MashStepEditor::setMashStep(std::shared_ptr<MashStep> step) {
   if (this->obs) {
      disconnect(this->obs.get(), nullptr, this, nullptr);
   }

   this->obs = step;

   if (this->obs) {
      connect(this->obs.get(), &MashStep::changed, this, &MashStepEditor::changed);
      showChanges();
   }
   return;
}

void MashStepEditor::saveAndClose() {
   obs->setName(lineEdit_name->text());
   obs->setType(static_cast<MashStep::Type>(comboBox_type->currentIndex()));
   obs->setInfuseAmount_l(lineEdit_infuseAmount->toCanonical().quantity());
   obs->setInfuseTemp_c(lineEdit_infuseTemp->toCanonical().quantity());
   obs->setDecoctionAmount_l(lineEdit_decoctionAmount->toCanonical().quantity());
   obs->setStepTemp_c(lineEdit_stepTemp->toCanonical().quantity());
   obs->setStepTime_min(lineEdit_stepTime->toCanonical().quantity());
   obs->setRampTime_min(lineEdit_rampTime->toCanonical().quantity());
   obs->setEndTemp_c(lineEdit_endTemp->toCanonical().quantity());

   if (this->obs->key() < 0) {
      // This is a new MashStep, so we need to store it.
      // We'll ask MainWindow to do this for us, because then it can be an undoable action.
      //
      // The Mash of this MashStep should already have been set by the caller
      MainWindow::instance().addMashStepToMash(this->obs);
   }

   setVisible(false);
   return;
}

void MashStepEditor::grayOutStuff(const QString& text) {
   if (text == "Infusion") {
      lineEdit_infuseAmount->setEnabled(true);
      lineEdit_infuseTemp->setEnabled(true);
      lineEdit_decoctionAmount->setEnabled(false);
   } else if (text == "Decoction") {
      lineEdit_infuseAmount->setEnabled(false);
      lineEdit_infuseTemp->setEnabled(false);
      lineEdit_decoctionAmount->setEnabled(true);
   } else if (text == "Temperature") {
      lineEdit_infuseAmount->setEnabled(false);
      lineEdit_infuseTemp->setEnabled(false);
      lineEdit_decoctionAmount->setEnabled(false);
   } else {
      lineEdit_infuseAmount->setEnabled(true);
      lineEdit_infuseTemp->setEnabled(true);
      lineEdit_decoctionAmount->setEnabled(true);
   }
   return;
}
