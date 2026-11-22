/*======================================================================================================================
 * widgets/RecipeNotesWidget.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *    • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Peter Buelow <goballstate@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "widgets/RecipeNotesWidget.h"

#include <QEvent>

#include "model/Recipe.h"
#include "undoRedo/Undoable.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeNotesWidget.cpp"
#endif

RecipeNotesWidget::RecipeNotesWidget(QWidget * parent) : QWidget(parent) {
   this->setupUi(this);

   connect(this->btTextEdit_notes     , &BtTextEdit::textModified, this, &RecipeNotesWidget::updateNotes     );
   connect(this->btTextEdit_tasteNotes, &BtTextEdit::textModified, this, &RecipeNotesWidget::updateTasteNotes);
   return;
}

RecipeNotesWidget::~RecipeNotesWidget() = default;

void RecipeNotesWidget::setRecipe(Recipe * recipe) {
   if (this->m_recipeObs) {
      disconnect(this->m_recipeObs, nullptr, this, nullptr);
   }

   this->m_recipeObs = recipe;
   if (this->m_recipeObs) {
      connect(this->m_recipeObs, &NamedEntity::changed, this, &RecipeNotesWidget::changed);
   }
   this->showChanges();
   return;
}

void RecipeNotesWidget::updateNotes() {
   if (!this->m_recipeObs) { return; }

   Undoable::doOrRedoUpdate(*this->m_recipeObs,
                            TYPE_INFO(Recipe, notes),
                            this->btTextEdit_notes->toPlainText(),
                            tr("Edit Notes"));
   return;
}

void RecipeNotesWidget::updateTasteNotes() {
   if (!this->m_recipeObs) { return; }

   Undoable::doOrRedoUpdate(*this->m_recipeObs,
                            TYPE_INFO(Recipe, tasteNotes),
                            this->btTextEdit_tasteNotes->toPlainText(),
                            tr("Edit Taste Notes"));
   return;
}

void RecipeNotesWidget::changed(QMetaProperty prop, [[maybe_unused]] QVariant val) {
   if (this->sender() != this->m_recipeObs) {
      return;
   }

   this->showChanges(&prop);
   return;
}

void RecipeNotesWidget::showChanges(QMetaProperty const * prop) {
   if (!this->m_recipeObs) {
      this->btTextEdit_notes     ->setPlainText(QString{});
      this->btTextEdit_tasteNotes->setPlainText(QString{});
      return;
   }

   QString const propName = prop ? prop->name() : QString{};
   if (!prop || propName == PropertyNames::Recipe::notes     ) { this->btTextEdit_notes     ->setPlainText(this->m_recipeObs->notes     ()); }
   if (!prop || propName == PropertyNames::Recipe::tasteNotes) { this->btTextEdit_tasteNotes->setPlainText(this->m_recipeObs->tasteNotes()); }
   return;
}

void RecipeNotesWidget::changeEvent(QEvent * event) {
   if (event->type() == QEvent::LanguageChange) {
      this->retranslateUi(this);
   }
   this->QWidget::changeEvent(event);
   return;
}
