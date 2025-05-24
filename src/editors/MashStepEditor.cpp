/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/MashStepEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MashStepEditor.cpp"
#endif

MashStepEditor::MashStepEditor(QWidget* parent, QString const editorName) :
   QDialog{parent},
   StepEditorBase<MashStepEditor, MashStep>{},
   EditorBase<MashStepEditor, MashStep, MashStepEditorOptions>{editorName} {
   this->setupUi(this);
   this->postSetupUiInit({
      //
      // As explained in model/MashStep.h, there is only one amount for a MashStep, and it is accessed via
      // MashStep::amount_l.  The decoctionAmount_l and infuseAmount_l properties are only used for reading and writing
      // BeerXML.
      //
      // We retain infuseTemp_c for now, even though it is not part of BeerJSON.  TBD whether it is needed longer-term.
      //
      EDITOR_FIELD_NORM(MashStep, label_name        , lineEdit_name        , NamedEntity::name                  ),
      EDITOR_FIELD_NORM(MashStep, label_stepNum     , label_stepNum_value  , EnumeratedBase::stepNumber         ),
      EDITOR_FIELD_NORM(MashStep, label_description , textEdit_description , Step::description                  ),
      EDITOR_FIELD_NORM(MashStep, label_amount      , lineEdit_amount      , MashStep::amount_l                 ),
      EDITOR_FIELD_NORM(MashStep, label_stepTemp    , lineEdit_stepTemp    , StepBase::startTemp_c           , 1),
      EDITOR_FIELD_NORM(MashStep, label_stepTime    , lineEdit_stepTime    , StepBase::stepTime_mins         , 0),
      EDITOR_FIELD_NORM(MashStep, label_rampTime    , lineEdit_rampTime    , StepBase::rampTime_mins         , 0),
      EDITOR_FIELD_NORM(MashStep, label_endTemp     , lineEdit_endTemp     , Step::endTemp_c                 , 1),
      EDITOR_FIELD_NORM(MashStep, label_infuseTemp  , lineEdit_infuseTemp  , MashStep::infuseTemp_c          , 1),
      EDITOR_FIELD_NORM(MashStep, label_startAcidity, lineEdit_startAcidity, Step::startAcidity_pH           , 1),
      EDITOR_FIELD_NORM(MashStep, label_endAcidity  , lineEdit_endAcidity  , Step::endAcidity_pH             , 1),
      EDITOR_FIELD_NORM(MashStep, label_thickness   , lineEdit_thickness   , MashStep::liquorToGristRatio_lKg, 1),
      EDITOR_FIELD_ENUM(MashStep, label_mashStepType, comboBox_mashStepType, MashStep::type                     ),
   });

   // This is extra for this editor
   this->connect(this->comboBox_mashStepType, &QComboBox::currentTextChanged, this, &MashStepEditor::grayOutStuff);
   return;
}

MashStepEditor::~MashStepEditor() = default;

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
EDITOR_COMMON_CODE(MashStep)
