/*======================================================================================================================
 * editors/SaltEditor.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "editors/SaltEditor.h"

#include <QtGui>
#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_SaltEditor.cpp"
#endif

SaltEditor::SaltEditor(QWidget * parent, QString const editorName) :
   QDialog(parent),
   EditorBase<SaltEditor, Salt, SaltEditorOptions>(editorName) {
   setupUi(this);
   this->postSetupUiInit({
      //
      // Write inventory late to make sure we've the row in the inventory table (because total inventory amount isn't
      // really an attribute of the Fermentable).
      //
      // Note that we do not need to store the value of comboBox_amountType.  It merely controls the available unit for
      // lineEdit_inventory
      //
      EDITOR_FIELD_NORM(Salt, label_name       , lineEdit_name       , NamedEntity::name         ),
      EDITOR_FIELD_NORM(Salt, label_totalInventory, display_totalInventory, Ingredient::totalInventory, 1, WhenToWriteField::Never),
      EDITOR_FIELD_ENUM(Salt, label_type       , comboBox_type       , Salt::type                ),
      EDITOR_FIELD_NORM(Salt, label_percentAcid, lineEdit_percentAcid, Salt::percentAcid         , 1),
   });
   return;
}

SaltEditor::~SaltEditor() = default;

void SaltEditor::postSetEditItem() {
   if (this->m_editItem) {
      bool const isAcid = this->m_editItem->isAcid();
      this->   label_percentAcid->setEnabled(isAcid);
      this->lineEdit_percentAcid->setEnabled(isAcid);
   } else {
      this->postInputFieldModified();
   }
   return;
}


void SaltEditor::postInputFieldModified() {
   // Strictly, we only need to do this if the combo box value changed.  But this way is trivial overhead, and keeps
   // other things simple, so we don't worry about it.
   bool const isAcid = Salt::typeIsAcid(this->comboBox_type->getNonOptValue<Salt::Type>());
   this->   label_percentAcid->setEnabled(isAcid);
   this->lineEdit_percentAcid->setEnabled(isAcid);
   if (!isAcid) {
      this->lineEdit_percentAcid->setRawText("");
   }
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(Salt)
