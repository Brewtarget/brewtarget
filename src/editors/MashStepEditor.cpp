/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/MashStepEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
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
#include "editors/MashStepEditor.h"

#include "MainWindow.h"
#include "measurement/Unit.h"

MashStepEditor::MashStepEditor(QWidget* parent) :
   QDialog{parent},
   EditorBase<MashStepEditor, MashStep>() {
   this->setupUi(this);

   // NB: label_description / textEdit_description don't need initialisation here as neither is a smart field
   SMART_FIELD_INIT(MashStepEditor, label_name           , lineEdit_name           , MashStep, PropertyNames::NamedEntity::name                  );
   SMART_FIELD_INIT(MashStepEditor, label_amount         , lineEdit_amount         , MashStep, PropertyNames::   MashStep::amount_l              );
   SMART_FIELD_INIT(MashStepEditor, label_infuseTemp     , lineEdit_infuseTemp     , MashStep, PropertyNames::   MashStep::infuseTemp_c          , 1);
   SMART_FIELD_INIT(MashStepEditor, label_stepTemp       , lineEdit_stepTemp       , MashStep, PropertyNames::       Step::startTemp_c           , 1);
   SMART_FIELD_INIT(MashStepEditor, label_stepTime       , lineEdit_stepTime       , MashStep, PropertyNames::       Step::stepTime_mins         , 0);
   SMART_FIELD_INIT(MashStepEditor, label_rampTime       , lineEdit_rampTime       , MashStep, PropertyNames::       Step::rampTime_mins         , 0);
   SMART_FIELD_INIT(MashStepEditor, label_endTemp        , lineEdit_endTemp        , MashStep, PropertyNames::       Step::endTemp_c             , 1);
   // ⮜⮜⮜ All three below added for BeerJSON support ⮞⮞⮞
   SMART_FIELD_INIT(MashStepEditor, label_thickness      , lineEdit_thickness      , MashStep, PropertyNames::   MashStep::liquorToGristRatio_lKg, 1);
   SMART_FIELD_INIT(MashStepEditor, label_startAcidity   , lineEdit_startAcidity   , MashStep, PropertyNames::       Step::startAcidity_pH       , 1);
   SMART_FIELD_INIT(MashStepEditor, label_endAcidity     , lineEdit_endAcidity     , MashStep, PropertyNames::       Step::endAcidity_pH         , 1);

   BT_COMBO_BOX_INIT(MashStepEditor, comboBox_mashStepType, MashStep, type);

   this->connectSignalsAndSlots();

   // This is extra for this editor
   connect(this->comboBox_mashStepType, &QComboBox::currentTextChanged, this, &MashStepEditor::grayOutStuff);
   return;
}

MashStepEditor::~MashStepEditor() = default;

void MashStepEditor::readFieldsFromEditItem(std::optional<QString> propName) {
   if (!propName || *propName == PropertyNames::NamedEntity::name               ) { this->lineEdit_name        ->setTextCursor(m_editItem->name                  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::MashStep::type                  ) { this->comboBox_mashStepType->setValue     (m_editItem->type                  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::MashStep::infuseAmount_l        ) { this->lineEdit_amount      ->setQuantity  (m_editItem->amount_l              ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::MashStep::infuseTemp_c          ) { this->lineEdit_infuseTemp  ->setQuantity  (m_editItem->infuseTemp_c          ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::    Step::startTemp_c           ) { this->lineEdit_stepTemp    ->setQuantity  (m_editItem->startTemp_c           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::    Step::stepTime_mins         ) { this->lineEdit_stepTime    ->setQuantity  (m_editItem->stepTime_mins         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::    Step::rampTime_mins         ) { this->lineEdit_rampTime    ->setQuantity  (m_editItem->rampTime_mins         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::    Step::endTemp_c             ) { this->lineEdit_endTemp     ->setQuantity  (m_editItem->endTemp_c             ()); if (propName) { return; } }
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   if (!propName || *propName == PropertyNames::MashStep::liquorToGristRatio_lKg) { this->lineEdit_thickness   ->setQuantity  (m_editItem->liquorToGristRatio_lKg()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::    Step::startAcidity_pH       ) { this->lineEdit_startAcidity->setQuantity  (m_editItem->startAcidity_pH       ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::    Step::endAcidity_pH         ) { this->lineEdit_endAcidity  ->setQuantity  (m_editItem->endAcidity_pH         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::    Step::description           ) { this->textEdit_description ->setPlainText (m_editItem->description           ()); if (propName) { return; } }
   return;
}

void MashStepEditor::writeFieldsToEditItem() {
   this->m_editItem->setName                  (this->lineEdit_name->text());
   this->m_editItem->setType                  (this->comboBox_mashStepType->getNonOptValue<MashStep::Type>());
   this->m_editItem->setAmount_l              (this->lineEdit_amount      ->getNonOptCanonicalQty());
   this->m_editItem->setInfuseTemp_c          (this->lineEdit_infuseTemp  ->getOptCanonicalQty   ());
   this->m_editItem->setStartTemp_c           (this->lineEdit_stepTemp    ->getNonOptCanonicalQty());
   this->m_editItem->setStepTime_mins         (this->lineEdit_stepTime    ->getNonOptCanonicalQty());
   this->m_editItem->setRampTime_mins         (this->lineEdit_rampTime    ->getOptCanonicalQty   ());
   this->m_editItem->setEndTemp_c             (this->lineEdit_endTemp     ->getOptCanonicalQty   ());
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   this->m_editItem->setLiquorToGristRatio_lKg(this->lineEdit_thickness   ->getOptCanonicalQty   ());
   this->m_editItem->setStartAcidity_pH       (this->lineEdit_startAcidity->getOptCanonicalQty   ());
   this->m_editItem->setEndAcidity_pH         (this->lineEdit_endAcidity  ->getOptCanonicalQty   ());
   this->m_editItem->setDescription           (this->textEdit_description ->toPlainText          ());
   return;
}

void MashStepEditor::writeLateFieldsToEditItem() {
   // Nothing to do here
   return;
}

void MashStepEditor::grayOutStuff([[maybe_unused]] QString const & text) {
   auto const msType = this->comboBox_mashStepType->getNonOptValue<MashStep::Type>();
   switch (msType) {
      case MashStep::Type::Infusion:
      case MashStep::Type::FlySparge  :
      case MashStep::Type::BatchSparge:
      case MashStep::Type::SouringMash:
      case MashStep::Type::SouringWort:
         lineEdit_infuseTemp->setEnabled(true);
         break;

      case MashStep::Type::Decoction:
      case MashStep::Type::Temperature:
         lineEdit_infuseTemp->setEnabled(false);
         break;

      // No default case as we want the compiler to warn us if we missed one
   }
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(MashStepEditor)
