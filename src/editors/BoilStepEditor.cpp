/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/BoilStepEditor.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "editors/BoilStepEditor.h"

#include "MainWindow.h"
#include "measurement/Unit.h"

BoilStepEditor::BoilStepEditor(QWidget* parent) :
   QDialog{parent},
   EditorBase<BoilStepEditor, BoilStep>() {
   this->setupUi(this);

   // NB: label_description / textEdit_description don't need initialisation here as neither is a smart field
   SMART_FIELD_INIT(BoilStepEditor, label_name           , lineEdit_name           , BoilStep, PropertyNames:: NamedEntity::name           );
   SMART_FIELD_INIT(BoilStepEditor, label_startTemp      , lineEdit_startTemp      , BoilStep, PropertyNames::        Step::startTemp_c    , 1);
   SMART_FIELD_INIT(BoilStepEditor, label_stepTime       , lineEdit_stepTime       , BoilStep, PropertyNames::        Step::stepTime_mins  , 0);
   SMART_FIELD_INIT(BoilStepEditor, label_rampTime       , lineEdit_rampTime       , BoilStep, PropertyNames::        Step::rampTime_mins  , 0);
   SMART_FIELD_INIT(BoilStepEditor, label_endTemp        , lineEdit_endTemp        , BoilStep, PropertyNames::        Step::endTemp_c      , 1);
   SMART_FIELD_INIT(BoilStepEditor, label_startAcidity   , lineEdit_startAcidity   , BoilStep, PropertyNames::        Step::startAcidity_pH, 1);
   SMART_FIELD_INIT(BoilStepEditor, label_endAcidity     , lineEdit_endAcidity     , BoilStep, PropertyNames::        Step::endAcidity_pH  , 1);
   SMART_FIELD_INIT(BoilStepEditor, label_startGravity   , lineEdit_startGravity   , BoilStep, PropertyNames::StepExtended::startGravity_sg, 3);
   SMART_FIELD_INIT(BoilStepEditor, label_endGravity     , lineEdit_endGravity     , BoilStep, PropertyNames::StepExtended::endGravity_sg  , 3);

   BT_COMBO_BOX_INIT(BoilStepEditor, comboBox_boilStepChillingType, BoilStep, chillingType);

   this->connectSignalsAndSlots();

   return;
}

BoilStepEditor::~BoilStepEditor() = default;

void BoilStepEditor::readFieldsFromEditItem(std::optional<QString> propName) {
   if (!propName || *propName == PropertyNames:: NamedEntity::name           ) { this->lineEdit_name        ->setTextCursor(m_editItem->name           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::description    ) { this->textEdit_description ->setPlainText (m_editItem->description    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::startTemp_c    ) { this->lineEdit_startTemp   ->setQuantity  (m_editItem->startTemp_c    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::stepTime_mins  ) { this->lineEdit_stepTime    ->setQuantity  (m_editItem->stepTime_mins  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::rampTime_mins  ) { this->lineEdit_rampTime    ->setQuantity  (m_editItem->rampTime_mins  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::endTemp_c      ) { this->lineEdit_endTemp     ->setQuantity  (m_editItem->endTemp_c      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::startAcidity_pH) { this->lineEdit_startAcidity->setQuantity  (m_editItem->startAcidity_pH()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::        Step::endAcidity_pH  ) { this->lineEdit_endAcidity  ->setQuantity  (m_editItem->endAcidity_pH  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::StepExtended::startGravity_sg) { this->lineEdit_startGravity->setQuantity  (m_editItem->startGravity_sg()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::StepExtended::endGravity_sg  ) { this->lineEdit_endGravity  ->setQuantity  (m_editItem->endGravity_sg  ()); if (propName) { return; } }

   if (!propName || *propName == PropertyNames::BoilStep::chillingType       ) { this->comboBox_boilStepChillingType->setValue(m_editItem->chillingType()); if (propName) { return; } }
   return;
}

void BoilStepEditor::writeFieldsToEditItem() {
   this->m_editItem->setName                  (this->lineEdit_name->text());
   this->m_editItem->setDescription           (this->textEdit_description ->toPlainText          ());
   this->m_editItem->setStartTemp_c           (this->lineEdit_startTemp   ->getNonOptCanonicalQty());
   this->m_editItem->setStepTime_mins         (this->lineEdit_stepTime    ->getNonOptCanonicalQty());
   this->m_editItem->setRampTime_mins         (this->lineEdit_rampTime    ->getOptCanonicalQty   ());
   this->m_editItem->setEndTemp_c             (this->lineEdit_endTemp     ->getOptCanonicalQty   ());
   this->m_editItem->setStartAcidity_pH       (this->lineEdit_startAcidity->getOptCanonicalQty   ());
   this->m_editItem->setEndAcidity_pH         (this->lineEdit_endAcidity  ->getOptCanonicalQty   ());
   this->m_editItem->setStartGravity_sg       (this->lineEdit_startGravity->getOptCanonicalQty   ());
   this->m_editItem->setEndGravity_sg         (this->lineEdit_endGravity  ->getOptCanonicalQty   ());

   this->m_editItem->setChillingType(this->comboBox_boilStepChillingType->getOptValue<BoilStep::ChillingType>());
   return;
}

void BoilStepEditor::writeLateFieldsToEditItem() {
   // Nothing to do here
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_SLOT_DEFINITIONS(BoilStepEditor)
