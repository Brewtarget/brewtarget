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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "editors/BoilEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "model/Boil.h"
#include "model/Recipe.h"

BoilEditor::BoilEditor(QWidget* parent) :
   QDialog(parent),
   EditorWithRecipeBase<BoilEditor, Boil>() {
   this->setupUi(this);
   this->postSetupUiInit(
      {
       EDITOR_FIELD(Boil, label_name       , lineEdit_name       , PropertyNames::NamedEntity::name     ),
       EDITOR_FIELD(Boil, label_description, textEdit_description, PropertyNames::Boil::description     ),
       EDITOR_FIELD(Boil, label_preBoilSize, lineEdit_preBoilSize, PropertyNames::Boil::preBoilSize_l, 2),
       EDITOR_FIELD(Boil, label_notes      , textEdit_notes      , PropertyNames::Boil::notes           )
      }
   );

   // NB: label_description / textEdit_description don't need initialisation here as neither is a smart field
   // NB: label_notes / textEdit_notes don't need initialisation here as neither is a smart field
///   SMART_FIELD_INIT(BoilEditor, label_name       , lineEdit_name       , Boil, PropertyNames::NamedEntity::name  );
///   SMART_FIELD_INIT(BoilEditor, label_preBoilSize, lineEdit_preBoilSize, Boil, PropertyNames::Boil::preBoilSize_l, 2);

///   connect(this, &QDialog::accepted, this, &BoilEditor::saveAndClose);
///   connect(this, &QDialog::rejected, this, &BoilEditor::closeEditor );

///   this->connectSignalsAndSlots();
   return;
}

BoilEditor::~BoilEditor() = default;

void BoilEditor::writeFieldsToEditItem() {
   return;
}

void BoilEditor::writeLateFieldsToEditItem() {
   return;
}

void BoilEditor::readFieldsFromEditItem([[maybe_unused]] std::optional<QString> propName) {
   return;
}

// Insert the boilerplate stuff that we cannot do in EditorWithRecipeBase
EDITOR_WITH_RECIPE_COMMON_CODE(BoilEditor)
