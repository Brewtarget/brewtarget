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

BoilStepEditor::BoilStepEditor(QWidget* parent, QString const editorName) :
   QDialog{parent},
   StepEditorBase<BoilStepEditor, BoilStep>{},
   EditorBase<BoilStepEditor, BoilStep, BoilStepEditorOptions>(editorName) {
   this->setupUi(this);
   this->postSetupUiInit({
      EDITOR_FIELD_NORM(BoilStep, label_name                , lineEdit_name                , NamedEntity::name               ),
      EDITOR_FIELD_NORM(BoilStep, label_description         , textEdit_description         , Step::description               ),
      EDITOR_FIELD_NORM(BoilStep, label_startTemp           , lineEdit_startTemp           , StepBase::startTemp_c        , 1),
      EDITOR_FIELD_NORM(BoilStep, label_stepTime            , lineEdit_stepTime            , StepBase::stepTime_mins      , 0),
      EDITOR_FIELD_NORM(BoilStep, label_rampTime            , lineEdit_rampTime            , StepBase::rampTime_mins      , 0),
      EDITOR_FIELD_NORM(BoilStep, label_endTemp             , lineEdit_endTemp             , Step::endTemp_c              , 1),
      EDITOR_FIELD_NORM(BoilStep, label_startAcidity        , lineEdit_startAcidity        , Step::startAcidity_pH        , 1),
      EDITOR_FIELD_NORM(BoilStep, label_endAcidity          , lineEdit_endAcidity          , Step::endAcidity_pH          , 1),
      EDITOR_FIELD_NORM(BoilStep, label_startGravity        , lineEdit_startGravity        , StepExtended::startGravity_sg, 3),
      EDITOR_FIELD_NORM(BoilStep, label_endGravity          , lineEdit_endGravity          , StepExtended::endGravity_sg  , 3),
      EDITOR_FIELD_ENUM(BoilStep, label_boilStepChillingType, comboBox_boilStepChillingType, BoilStep::chillingType          ),
   });

   return;
}

BoilStepEditor::~BoilStepEditor() = default;

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(BoilStep)
