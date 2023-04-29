/*
 * MashStepEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
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

   SMART_FIELD_INIT(MashStepEditor, label_stepTemp       , lineEdit_stepTemp       , MashStep, PropertyNames::MashStep::stepTemp_c       , 1);
   SMART_FIELD_INIT(MashStepEditor, label_infuseAmount   , lineEdit_infuseAmount   , MashStep, PropertyNames::MashStep::infuseAmount_l      );
   SMART_FIELD_INIT(MashStepEditor, label_infuseTemp     , lineEdit_infuseTemp     , MashStep, PropertyNames::MashStep::infuseTemp_c     , 1);
   SMART_FIELD_INIT(MashStepEditor, label_decoctionAmount, lineEdit_decoctionAmount, MashStep, PropertyNames::MashStep::decoctionAmount_l   );
   SMART_FIELD_INIT(MashStepEditor, label_stepTime       , lineEdit_stepTime       , MashStep, PropertyNames::MashStep::stepTime_min     , 0);
   SMART_FIELD_INIT(MashStepEditor, label_rampTime       , lineEdit_rampTime       , MashStep, PropertyNames::MashStep::rampTime_min     , 0);
   SMART_FIELD_INIT(MashStepEditor, label_endTemp        , lineEdit_endTemp        , MashStep, PropertyNames::MashStep::endTemp_c        , 1);

   connect(this->buttonBox,     &QDialogButtonBox::accepted,    this, &MashStepEditor::saveAndClose);
   connect(this->buttonBox,     &QDialogButtonBox::rejected,    this, &MashStepEditor::close       );
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

   if (updateAll || propName == PropertyNames::NamedEntity::name          ) { this->lineEdit_name           ->setText        (this->obs->name())             ; if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::MashStep::type             ) { this->comboBox_type           ->setCurrentIndex(static_cast<int>(obs->type())) ; if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::MashStep::infuseAmount_l   ) { this->lineEdit_infuseAmount   ->setAmount      (this->obs->infuseAmount_l   ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::MashStep::infuseTemp_c     ) { this->lineEdit_infuseTemp     ->setAmount      (this->obs->infuseTemp_c     ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::MashStep::decoctionAmount_l) { this->lineEdit_decoctionAmount->setAmount      (this->obs->decoctionAmount_l()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::MashStep::stepTemp_c       ) { this->lineEdit_stepTemp       ->setAmount      (this->obs->stepTemp_c       ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::MashStep::stepTime_min     ) { this->lineEdit_stepTime       ->setAmount      (this->obs->stepTime_min     ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::MashStep::rampTime_min     ) { this->lineEdit_rampTime       ->setAmount      (this->obs->rampTime_min     ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::MashStep::endTemp_c        ) { this->lineEdit_endTemp        ->setAmount      (this->obs->endTemp_c        ()); if (!updateAll) { return; } }
   return;
}

void MashStepEditor::clear() {
   this->lineEdit_name           ->setText(QString(""));
   this->comboBox_type           ->setCurrentIndex(0);
   this->lineEdit_infuseAmount   ->setText(QString(""));
   this->lineEdit_infuseTemp     ->setText(QString(""));
   this->lineEdit_decoctionAmount->setText(QString(""));
   this->lineEdit_stepTemp       ->setText(QString(""));
   this->lineEdit_stepTime       ->setText(QString(""));
   this->lineEdit_rampTime       ->setText(QString(""));
   this->lineEdit_endTemp        ->setText(QString(""));
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
   this->obs->setName             (lineEdit_name->text());
   this->obs->setType             (static_cast<MashStep::Type>(comboBox_type->currentIndex()));
   this->obs->setInfuseAmount_l   (lineEdit_infuseAmount   ->toCanonical().quantity());
   this->obs->setInfuseTemp_c     (lineEdit_infuseTemp     ->toCanonical().quantity());
   this->obs->setDecoctionAmount_l(lineEdit_decoctionAmount->toCanonical().quantity());
   this->obs->setStepTemp_c       (lineEdit_stepTemp       ->toCanonical().quantity());
   this->obs->setStepTime_min     (lineEdit_stepTime       ->toCanonical().quantity());
   this->obs->setRampTime_min     (lineEdit_rampTime       ->toCanonical().quantity());
   this->obs->setEndTemp_c        (lineEdit_endTemp        ->toCanonical().quantity());

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
