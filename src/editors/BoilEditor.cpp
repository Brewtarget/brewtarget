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
 =====================================================================================================================*/
#include "editors/BoilEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "model/Boil.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BoilEditor.cpp"
#endif

BoilEditor::BoilEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<BoilEditor, Boil, BoilEditorOptions>(editorName) {
   this->setupUi(this);
   this->postSetupUiInit(
      {
       EDITOR_FIELD_NORM(Boil, label_name       , lineEdit_name       , NamedEntity::name     ),
       EDITOR_FIELD_NORM(Boil, label_description, textEdit_description, Boil::description     ),
       EDITOR_FIELD_NORM(Boil, label_preBoilSize, lineEdit_preBoilSize, Boil::preBoilSize_l, 2),
       EDITOR_FIELD_NORM(Boil, label_notes      , textEdit_notes      , Boil::notes           ),
      }
   );

   return;
}

BoilEditor::~BoilEditor() = default;

// Insert the boilerplate stuff that we cannot do in EditorWithRecipeBase
EDITOR_COMMON_CODE(Boil)
