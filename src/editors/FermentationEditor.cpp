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

FermentationEditor::FermentationEditor(QWidget* parent) :
   QDialog(parent),
   EditorWithRecipeBase<FermentationEditor, Fermentation>() {
   this->setupUi(this);
   this->postSetupUiInit(
      {
       EDITOR_FIELD(Fermentation, label_name       , lineEdit_name       , PropertyNames::NamedEntity::name     ),
       EDITOR_FIELD(Fermentation, label_description, textEdit_description, PropertyNames::Fermentation::description     ),
       EDITOR_FIELD(Fermentation, label_notes      , textEdit_notes      , PropertyNames::Fermentation::notes           )
      }
   );

///   // NB: label_description / textEdit_description don't need initialisation here as neither is a smart field
///   // NB: label_notes / textEdit_notes don't need initialisation here as neither is a smart field
///   SMART_FIELD_INIT(FermentationEditor, label_name       , lineEdit_name       , Fermentation, PropertyNames::NamedEntity::name  );
///
///   connect(this, &QDialog::accepted, this, &FermentationEditor::saveAndClose);
///   connect(this, &QDialog::rejected, this, &FermentationEditor::closeEditor );
   return;
}

FermentationEditor::~FermentationEditor() = default;

void FermentationEditor::writeFieldsToEditItem() {
   return;
}

void FermentationEditor::writeLateFieldsToEditItem() {
   return;
}

void FermentationEditor::readFieldsFromEditItem([[maybe_unused]] std::optional<QString> propName) {
   return;
}

// Insert the boilerplate stuff that we cannot do in EditorWithRecipeBase
EDITOR_WITH_RECIPE_COMMON_CODE(FermentationEditor)

///void FermentationEditor::showEditor() {
///   showChanges();
///   setVisible(true);
///   return;
///}
///
///void FermentationEditor::closeEditor() {
///   setVisible(false);
///   return;
///}
///
///void FermentationEditor::saveAndClose() {
///   bool isNew = false;
///
///   if (!this->m_fermentationObs) {
///      this->m_fermentationObs = std::make_shared<Fermentation>(lineEdit_name->text());
///      isNew = true;
///   }
///   qDebug() << Q_FUNC_INFO << "Saving" << (isNew ? "new" : "existing") << "fermentation (#" << this->m_fermentationObs->key() << ")";
///
///   this->m_fermentationObs->setName         (this->lineEdit_name       ->text                 ());
///   this->m_fermentationObs->setDescription  (this->textEdit_description->toPlainText          ());
///   this->m_fermentationObs->setNotes        (this->textEdit_notes      ->toPlainText          ());
///
///   if (isNew) {
///      ObjectStoreWrapper::insert(this->m_fermentationObs);
///      this->m_rec->setFermentation(this->m_fermentationObs);
///   }
///
///   return;
///}
///
///void FermentationEditor::setFermentation(std::shared_ptr<Fermentation> fermentation) {
///   if (this->m_fermentationObs) {
///      disconnect(this->m_fermentationObs.get(), nullptr, this, nullptr);
///   }
///
///   this->m_fermentationObs = fermentation;
///   if (this->m_fermentationObs) {
///      connect(this->m_fermentationObs.get(), &NamedEntity::changed, this, &FermentationEditor::changed);
///      showChanges();
///   }
///   return;
///}
///
///void FermentationEditor::setRecipe(Recipe * recipe) {
///   if (!recipe) {
///      return;
///   }
///
///   this->m_rec = recipe;
///
///   return;
///}
///
///void FermentationEditor::changed(QMetaProperty prop, QVariant /*val*/) {
///   if (!this->m_fermentationObs) {
///      return;
///   }
///
///
///   if (sender() == this->m_fermentationObs.get()) {
///      this->showChanges(&prop);
///   }
///
///   if (sender() == this->m_rec) {
///      this->showChanges();
///   }
///   return;
///}
///
///void FermentationEditor::showChanges(QMetaProperty* prop) {
///   if (!this->m_fermentationObs) {
///      this->clear();
///      return;
///   }
///
///   QString propName;
///   bool updateAll = false;
///   if (prop == nullptr) {
///      updateAll = true;
///   } else {
///      propName = prop->name();
///   }
///   qDebug() << Q_FUNC_INFO << "Updating" << (updateAll ? "all" : "property") << propName;
///
///   if (updateAll || propName == PropertyNames::NamedEntity::name        ) {this->lineEdit_name       ->setText     (m_fermentationObs->name       ()); if (!updateAll) { return; } }
///   if (updateAll || propName == PropertyNames::Fermentation::description) {this->textEdit_description->setPlainText(m_fermentationObs->description()); if (!updateAll) { return; } }
///   if (updateAll || propName == PropertyNames::Fermentation::notes      ) {this->textEdit_notes      ->setPlainText(m_fermentationObs->notes      ()); if (!updateAll) { return; } }
///   return;
///}
///
///void FermentationEditor::clear() {
///   this->lineEdit_name               ->setText     ("");
///   this->textEdit_description        ->setText     ("");
///   this->lineEdit_preFermentationSize->setText     ("");
///   this->lineEdit_fermentationTime   ->setText     ("");
///   this->textEdit_notes              ->setPlainText("");
///   return;
///}
