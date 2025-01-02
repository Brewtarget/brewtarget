/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/FermentationEditor.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "editors/FermentationEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "model/Fermentation.h"
#include "model/Recipe.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_FermentationEditor.cpp"

FermentationEditor::FermentationEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<FermentationEditor, Fermentation, FermentationEditorOptions>(editorName) {
   this->setupUi(this);
   this->postSetupUiInit({
      EDITOR_FIELD_NORM(Fermentation, label_name       , lineEdit_name       , NamedEntity::name        ),
      EDITOR_FIELD_NORM(Fermentation, label_description, textEdit_description, Fermentation::description),
      EDITOR_FIELD_NORM(Fermentation, label_notes      , textEdit_notes      , Fermentation::notes      ),
   });
   return;
}

FermentationEditor::~FermentationEditor() = default;

// Insert the boilerplate stuff that we cannot do in EditorWithRecipeBase
EDITOR_COMMON_CODE(Fermentation)
