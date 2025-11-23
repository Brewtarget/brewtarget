/*======================================================================================================================
 * editors/StockUseIngredientEditor.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "editors/StockUseIngredientEditor.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockUseIngredientEditor.cpp"
#endif

StockUseFermentableEditor::StockUseFermentableEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EnumeratedItemEditorBase<StockUseFermentableEditor, StockUseFermentable>{},
   EditorBase<StockUseFermentableEditor, StockUseFermentable, StockUseFermentableEditorOptions>(editorName) {
   setupUi(this);
   // Do the ingredient-specific bits of the UI
   this->retranslateUi();

   this->postSetupUiInit({
      EDITOR_FIELD_NORM(StockUseFermentable, label_date      , dateEdit_date      , StockUse::date  ),
      EDITOR_FIELD_ENUM(StockUseFermentable, label_reason    , comboBox_reason    , StockUse::reason),
      EDITOR_FIELD_NORM(StockUseFermentable, label_amountUsed, lineEdit_amountUsed, StockUseBase::amountUsed, 1),
      EDITOR_FIELD_NORM(StockUseFermentable, label_comment   , lineEdit_comment   , StockUse::comment),
      EDITOR_FIELD_PATH(StockUseFermentable, label_recipe    , label_recipe_value , {PropertyNames::StockUse::brewNote,
                                                                                     PropertyNames::OwnedByRecipe::recipe,
                                                                                     PropertyNames::NamedEntity::name}, 1),
   });
   return;
}

void StockUseFermentableEditor::retranslateUi() {
   this->setWindowTitle(tr("%1 Stock Use Editor").arg(Fermentable::localisedName()));
   this->Ui::stockUseIngredientEditor::retranslateUi(this);
   return;
}

StockUseHopEditor::StockUseHopEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EnumeratedItemEditorBase<StockUseHopEditor, StockUseHop>{},
   EditorBase<StockUseHopEditor, StockUseHop, StockUseHopEditorOptions>(editorName) {
   setupUi(this);
   // Do the ingredient-specific bits of the UI
   this->retranslateUi();

   this->postSetupUiInit({
      EDITOR_FIELD_NORM(StockUseHop, label_date      , dateEdit_date      , StockUse::date  ),
      EDITOR_FIELD_ENUM(StockUseHop, label_reason    , comboBox_reason    , StockUse::reason),
      EDITOR_FIELD_NORM(StockUseHop, label_amountUsed, lineEdit_amountUsed, StockUseBase::amountUsed, 1),
      EDITOR_FIELD_NORM(StockUseHop, label_comment   , lineEdit_comment   , StockUse::comment),
      EDITOR_FIELD_PATH(StockUseHop, label_recipe    , label_recipe_value , {PropertyNames::StockUse::brewNote,
                                                                             PropertyNames::OwnedByRecipe::recipe,
                                                                             PropertyNames::NamedEntity::name}, 1),
   });
   return;
}

void StockUseHopEditor::retranslateUi() {
   this->setWindowTitle(tr("%1 Stock Use Editor").arg(Hop::localisedName()));
   this->Ui::stockUseIngredientEditor::retranslateUi(this);
   return;
}

StockUseMiscEditor::StockUseMiscEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EnumeratedItemEditorBase<StockUseMiscEditor, StockUseMisc>{},
   EditorBase<StockUseMiscEditor, StockUseMisc, StockUseMiscEditorOptions>(editorName) {
   setupUi(this);
   // Do the ingredient-specific bits of the UI
   this->retranslateUi();

   this->postSetupUiInit({
      EDITOR_FIELD_NORM(StockUseMisc, label_date      , dateEdit_date      , StockUse::date  ),
      EDITOR_FIELD_ENUM(StockUseMisc, label_reason    , comboBox_reason    , StockUse::reason),
      EDITOR_FIELD_NORM(StockUseMisc, label_amountUsed, lineEdit_amountUsed, StockUseBase::amountUsed, 1),
      EDITOR_FIELD_NORM(StockUseMisc, label_comment   , lineEdit_comment   , StockUse::comment),
      EDITOR_FIELD_PATH(StockUseMisc, label_recipe    , label_recipe_value , {PropertyNames::StockUse::brewNote,
                                                                              PropertyNames::OwnedByRecipe::recipe,
                                                                              PropertyNames::NamedEntity::name}, 1),
   });
   return;
}

void StockUseMiscEditor::retranslateUi() {
   this->setWindowTitle(tr("%1 Stock Use Editor").arg(Misc::localisedName()));
   this->Ui::stockUseIngredientEditor::retranslateUi(this);
   return;
}

StockUseSaltEditor::StockUseSaltEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EnumeratedItemEditorBase<StockUseSaltEditor, StockUseSalt>{},
   EditorBase<StockUseSaltEditor, StockUseSalt, StockUseSaltEditorOptions>(editorName) {
   setupUi(this);
   // Do the ingredient-specific bits of the UI
   this->retranslateUi();

   this->postSetupUiInit({
      EDITOR_FIELD_NORM(StockUseSalt, label_date      , dateEdit_date      , StockUse::date  ),
      EDITOR_FIELD_ENUM(StockUseSalt, label_reason    , comboBox_reason    , StockUse::reason),
      EDITOR_FIELD_NORM(StockUseSalt, label_amountUsed, lineEdit_amountUsed, StockUseBase::amountUsed, 1),
      EDITOR_FIELD_NORM(StockUseSalt, label_comment   , lineEdit_comment   , StockUse::comment),
      EDITOR_FIELD_PATH(StockUseSalt, label_recipe    , label_recipe_value , {PropertyNames::StockUse::brewNote,
                                                                              PropertyNames::OwnedByRecipe::recipe,
                                                                              PropertyNames::NamedEntity::name}, 1),
   });
   return;
}

void StockUseSaltEditor::retranslateUi() {
   this->setWindowTitle(tr("%1 Stock Use Editor").arg(Salt::localisedName()));
   this->Ui::stockUseIngredientEditor::retranslateUi(this);
   return;
}

StockUseYeastEditor::StockUseYeastEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EnumeratedItemEditorBase<StockUseYeastEditor, StockUseYeast>{},
   EditorBase<StockUseYeastEditor, StockUseYeast, StockUseYeastEditorOptions>(editorName) {
   setupUi(this);
   // Do the ingredient-specific bits of the UI
   this->retranslateUi();

   this->postSetupUiInit({
      EDITOR_FIELD_NORM(StockUseYeast, label_date      , dateEdit_date      , StockUse::date  ),
      EDITOR_FIELD_ENUM(StockUseYeast, label_reason    , comboBox_reason    , StockUse::reason),
      EDITOR_FIELD_NORM(StockUseYeast, label_amountUsed, lineEdit_amountUsed, StockUseBase::amountUsed, 1),
      EDITOR_FIELD_NORM(StockUseYeast, label_comment   , lineEdit_comment   , StockUse::comment),
      EDITOR_FIELD_PATH(StockUseYeast, label_recipe    , label_recipe_value , {PropertyNames::StockUse::brewNote,
                                                                               PropertyNames::OwnedByRecipe::recipe,
                                                                               PropertyNames::NamedEntity::name}, 1),
   });
   return;
}

void StockUseYeastEditor::retranslateUi() {
   this->setWindowTitle(tr("%1 Stock Use Editor").arg(Yeast::localisedName()));
   this->Ui::stockUseIngredientEditor::retranslateUi(this);
   return;
}

StockUseFermentableEditor::~StockUseFermentableEditor() = default;
StockUseHopEditor        ::~StockUseHopEditor        () = default;
StockUseMiscEditor       ::~StockUseMiscEditor       () = default;
StockUseSaltEditor       ::~StockUseSaltEditor       () = default;
StockUseYeastEditor      ::~StockUseYeastEditor      () = default;

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(StockUseFermentable)
EDITOR_COMMON_CODE(StockUseHop        )
EDITOR_COMMON_CODE(StockUseMisc       )
EDITOR_COMMON_CODE(StockUseSalt       )
EDITOR_COMMON_CODE(StockUseYeast      )
