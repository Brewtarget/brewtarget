/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/FermentationStepEditor.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "editors/FermentationStepEditor.h"

#include "MainWindow.h"
#include "measurement/Unit.h"

FermentationStepEditor::FermentationStepEditor(QWidget* parent) :
   QDialog{parent},
   EditorBase<FermentationStepEditor, FermentationStep>() {
   this->setupUi(this);

   // NB: Although FermentationStep inherits (via StepExtended) from Step, the rampTime_mins field is not used and
   //     should not be stored in the DB or serialised.  See comment in model/Step.h.  There should therefore not be
   //     any label_rampTime or lineEdit_rampTime fields in the .ui file!
   //
   // NB: label_description / textEdit_description don't need initialisation here as neither is a smart field
   SMART_FIELD_INIT(FermentationStepEditor, label_name           , lineEdit_name           , FermentationStep, PropertyNames::     NamedEntity::name           );
   SMART_FIELD_INIT(FermentationStepEditor, label_startTemp      , lineEdit_startTemp      , FermentationStep, PropertyNames::            Step::startTemp_c    , 1);
   SMART_FIELD_INIT(FermentationStepEditor, label_stepTime       , lineEdit_stepTime       , FermentationStep, PropertyNames::            Step::stepTime_mins  , 0);
   SMART_FIELD_INIT(FermentationStepEditor, label_endTemp        , lineEdit_endTemp        , FermentationStep, PropertyNames::            Step::endTemp_c      , 1);
   SMART_FIELD_INIT(FermentationStepEditor, label_startAcidity   , lineEdit_startAcidity   , FermentationStep, PropertyNames::            Step::startAcidity_pH, 1);
   SMART_FIELD_INIT(FermentationStepEditor, label_endAcidity     , lineEdit_endAcidity     , FermentationStep, PropertyNames::            Step::endAcidity_pH  , 1);
   SMART_FIELD_INIT(FermentationStepEditor, label_startGravity   , lineEdit_startGravity   , FermentationStep, PropertyNames::    StepExtended::startGravity_sg, 3);
   SMART_FIELD_INIT(FermentationStepEditor, label_endGravity     , lineEdit_endGravity     , FermentationStep, PropertyNames::    StepExtended::endGravity_sg  , 3);
   SMART_FIELD_INIT(FermentationStepEditor, label_vessel         , lineEdit_vessel         , FermentationStep, PropertyNames::FermentationStep::vessel         );
   BT_BOOL_COMBO_BOX_INIT(FermentationStepEditor, boolCombo_freeRise, FermentationStep, freeRise);

   this->connectSignalsAndSlots();

   return;
}

FermentationStepEditor::~FermentationStepEditor() = default;

void FermentationStepEditor::readFieldsFromEditItem(std::optional<QString> propName) {
   // NB: Although FermentationStep inherits (via StepExtended) from Step, the rampTime_mins field is not used and
   //     should not be stored in the DB or serialised.  See comment in model/Step.h.
   if (!propName || *propName == PropertyNames:: NamedEntity::name           ) { this->lineEdit_name        ->setTextCursor(m_editItem->name           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::description    ) { this->textEdit_description ->setPlainText (m_editItem->description    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::startTemp_c    ) { this->lineEdit_startTemp   ->setQuantity  (m_editItem->startTemp_c    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::stepTime_mins  ) { this->lineEdit_stepTime    ->setQuantity  (m_editItem->stepTime_mins  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::endTemp_c      ) { this->lineEdit_endTemp     ->setQuantity  (m_editItem->endTemp_c      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::startAcidity_pH) { this->lineEdit_startAcidity->setQuantity  (m_editItem->startAcidity_pH()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::endAcidity_pH  ) { this->lineEdit_endAcidity  ->setQuantity  (m_editItem->endAcidity_pH  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::StepExtended::startGravity_sg) { this->lineEdit_startGravity->setQuantity  (m_editItem->startGravity_sg()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::StepExtended::endGravity_sg  ) { this->lineEdit_endGravity  ->setQuantity  (m_editItem->endGravity_sg  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::FermentationStep::vessel     ) { this->lineEdit_vessel      ->setTextCursor(m_editItem->vessel         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::FermentationStep::freeRise   ) { this->boolCombo_freeRise   ->setValue     (m_editItem->freeRise       ()); if (propName) { return; } }

   return;
}

void FermentationStepEditor::writeFieldsToEditItem() {
   // NB: Although FermentationStep inherits (via StepExtended) from Step, the rampTime_mins field is not used and
   //     should not be stored in the DB or serialised.  See comment in model/Step.h.
   this->m_editItem->setName           (this->lineEdit_name        ->text              ());
   this->m_editItem->setDescription    (this->textEdit_description ->toPlainText       ());
   this->m_editItem->setStartTemp_c    (this->lineEdit_startTemp   ->getOptCanonicalQty());
   this->m_editItem->setStepTime_mins  (this->lineEdit_stepTime    ->getOptCanonicalQty());
   this->m_editItem->setEndTemp_c      (this->lineEdit_endTemp     ->getOptCanonicalQty());
   this->m_editItem->setStartAcidity_pH(this->lineEdit_startAcidity->getOptCanonicalQty());
   this->m_editItem->setEndAcidity_pH  (this->lineEdit_endAcidity  ->getOptCanonicalQty());
   this->m_editItem->setStartGravity_sg(this->lineEdit_startGravity->getOptCanonicalQty());
   this->m_editItem->setEndGravity_sg  (this->lineEdit_endGravity  ->getOptCanonicalQty());
   this->m_editItem->setVessel         (this->lineEdit_vessel      ->text              ());
   this->m_editItem->setFreeRise       (this->boolCombo_freeRise   ->getOptBoolValue   ());

   return;
}

void FermentationStepEditor::writeLateFieldsToEditItem() {
   // Nothing to do here
   return;
}

// Insert the fermentationer-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_SLOT_DEFINITIONS(FermentationStepEditor)
