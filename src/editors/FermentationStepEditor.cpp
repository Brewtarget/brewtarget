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

FermentationStepEditor::FermentationStepEditor(QWidget* parent, QString const editorName) :
   QDialog{parent},
   StepEditorBase<FermentationStepEditor, FermentationStep>{},
   EditorBase<FermentationStepEditor, FermentationStep, FermentationStepEditorOptions>(editorName) {
   this->setupUi(this);
   this->postSetupUiInit({
      // NB: Although FermentationStep inherits (via StepExtended) from Step, the rampTime_mins field is not used and
      //     should not be stored in the DB or serialised.  See comment in model/Step.h.  There should therefore not be
      //     any label_rampTime or lineEdit_rampTime fields in the .ui file!
      EDITOR_FIELD_NORM(FermentationStep, label_name        , lineEdit_name           ,      NamedEntity::name              ),
      EDITOR_FIELD_NORM(FermentationStep, label_description , textEdit_description    ,             Step::description       ),
      EDITOR_FIELD_NORM(FermentationStep, label_startTemp   , lineEdit_startTemp      ,             Step::startTemp_c    , 1),
      EDITOR_FIELD_NORM(FermentationStep, label_stepTime    , lineEdit_stepTime       ,             Step::stepTime_mins  , 0),
      EDITOR_FIELD_NORM(FermentationStep, label_endTemp     , lineEdit_endTemp        ,             Step::endTemp_c      , 1),
      EDITOR_FIELD_NORM(FermentationStep, label_startAcidity, lineEdit_startAcidity   ,             Step::startAcidity_pH, 1),
      EDITOR_FIELD_NORM(FermentationStep, label_endAcidity  , lineEdit_endAcidity     ,             Step::endAcidity_pH  , 1),
      EDITOR_FIELD_NORM(FermentationStep, label_startGravity, lineEdit_startGravity   ,     StepExtended::startGravity_sg, 3),
      EDITOR_FIELD_NORM(FermentationStep, label_endGravity  , lineEdit_endGravity     ,     StepExtended::endGravity_sg  , 3),
      EDITOR_FIELD_NORM(FermentationStep, label_vessel      , lineEdit_vessel         , FermentationStep::vessel            ),
      EDITOR_FIELD_NORM(FermentationStep, label_freeRise    , boolCombo_freeRise      , FermentationStep::freeRise          ),
   });

   return;
}

FermentationStepEditor::~FermentationStepEditor() = default;

// Insert the boilerplate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(FermentationStep)
