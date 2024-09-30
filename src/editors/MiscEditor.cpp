/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/MiscEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#include "editors/MiscEditor.h"

#include <QtGui>
#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

MiscEditor::MiscEditor(QWidget * parent, QString const editorName) :
   QDialog(parent),
   EditorBase<MiscEditor, Misc, MiscEditorOptions>(editorName) {
   setupUi(this);
   this->postSetupUiInit({
      //
      // Write inventory late to make sure we've the row in the inventory table (because total inventory amount isn't
      // really an attribute of the Fermentable).
      //
      // Note that we do not need to store the value of comboBox_amountType.  It merely controls the available unit for
      // lineEdit_inventory
      //
      EDITOR_FIELD_NORM(Misc, label_name      , lineEdit_name      , NamedEntity::name         ),
      EDITOR_FIELD_NORM(Misc, label_inventory , lineEdit_inventory , Ingredient::totalInventory, 1, WhenToWriteField::Late),
      EDITOR_FIELD_COPQ(Misc, label_amountType, comboBox_amountType, Ingredient::totalInventory, lineEdit_inventory, WhenToWriteField::Never),
      EDITOR_FIELD_ENUM(Misc, label_type      , comboBox_type      , Misc::type                ),
      EDITOR_FIELD_NORM(Misc, tab_useFor      , textEdit_useFor    , Misc::useFor              ),
      EDITOR_FIELD_NORM(Misc, tab_notes       , textEdit_notes     , Misc::notes               ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      EDITOR_FIELD_NORM(Misc, label_producer  , lineEdit_producer  , Misc::producer            ),
      EDITOR_FIELD_NORM(Misc, label_productId , lineEdit_productId , Misc::productId           ),
   });
   return;
}

MiscEditor::~MiscEditor() = default;

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(Misc)
