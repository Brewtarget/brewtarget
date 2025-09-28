/*======================================================================================================================
 * editors/InventoryFermentableEditor.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "editors/InventoryFermentableEditor.h"

#include <QIcon>
#include <QInputDialog>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_InventoryFermentableEditor.cpp"
#endif

InventoryFermentableEditor::InventoryFermentableEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<InventoryFermentableEditor, InventoryFermentable, InventoryFermentableEditorOptions>(editorName) {
   setupUi(this);
   this->postSetupUiInit({
      EDITOR_FIELD_NORM(InventoryFermentable, label_dateOrdered, dateEdit_dateOrdered, Inventory::dateOrdered),
   });
   return;
}

InventoryFermentableEditor::~InventoryFermentableEditor() = default;

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(InventoryFermentable)
