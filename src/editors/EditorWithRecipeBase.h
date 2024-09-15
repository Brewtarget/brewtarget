/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/EditorWithRecipeBase.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef EDITORS_EDITORWITHRECIPEBASE_H
#define EDITORS_EDITORWITHRECIPEBASE_H
#pragma once

#include "editors/EditorBase.h"
#include "model/Recipe.h"

/**
 * \brief Extends \c EditorBase for editors where it makes sense for us to be able to watch a \c Recipe, because a
 *        \c Recipe can have at most one of the thing we are editing (\c Boil, \c Equipment, \c Fermentation, \c Mash,
 *        \c Style).
 *
 *        Classes deriving from this one have the same requirements as those deriving directly from \c EditorBase,
 *        except that:
 *           EDITOR_WITH_RECIPE_COMMON_DECL should be placed in the header file instead of EDITOR_COMMON_DECL
 *           EDITOR_WITH_RECIPE_COMMON_CODE should be placed in the .cpp   file instead of EDITOR_COMMON_CODE
 */
template<class Derived, class NE>
class EditorWithRecipeBase : public EditorBase<Derived, NE> {
public:
   EditorWithRecipeBase() :
      EditorBase<Derived, NE>{},
      m_recipeObs{nullptr} {
      return;
   }
   virtual ~EditorWithRecipeBase() = default;

   void setRecipe(Recipe * recipe) {
      this->m_recipeObs = recipe;
      // TBD: We could automatically set the edit item as follows:
//   if (this->m_recipeObs) {
//      this->m_editItem = this->m_recipeObs->get<NE>()
//   }
      return;
   }

   /**
    * \brief Override \c EditorBase::doChanged
    */
   virtual void doChanged(QObject * sender, QMetaProperty prop, QVariant val) {
      // Extra handling if sender is Recipe we are observing...
      if (this->m_recipeObs && sender == this->m_recipeObs) {
         this->readAllFields();
         return;
      }
      // ...otherwise we fall back to the base class handling
      this->EditorBase<Derived, NE>::doChanged(sender, prop, val);
      return;
   }

   /**
    * \brief
    */
   void doShowEditor() {
      this->readAllFields();
      this->derived().setVisible(true);
      return;
   }

   /**
    * \brief
    */
   void doCloseEditor() {
      this->derived().setVisible(true);
      return;
   }

protected:

   /**
    * \brief The \c Recipe, if any, that we are "observing".
    */
   Recipe * m_recipeObs;
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT, instead of EDITOR_COMMON_DECL
 *        (which this macro also pulls in).
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define EDITOR_WITH_RECIPE_COMMON_DECL(NeName)                                                 \
   EDITOR_COMMON_DECL(NeName)                                                                  \
                                                                                               \
   /* This allows EditorWithRecipeBase to call protected and private members of Derived */     \
   friend class EditorWithRecipeBase<NeName##Editor, NeName>;                                  \
                                                                                               \
   public slots:                                                                               \
      /* Additional standard slots for editors with recipe */                                  \
      void showEditor();                                                                       \
      void closeEditor();                                                                      \

/**
 * \brief Derived classes should include this in their implementation file, usually at the end, instead of
 *        EDITOR_COMMON_CODE (which this macro also pulls in).
 */
#define EDITOR_WITH_RECIPE_COMMON_CODE(EditorName) \
   EDITOR_COMMON_CODE(EditorName) \
   void EditorName::showEditor() { this->doShowEditor(); return; } \
   void EditorName::closeEditor() { this->doCloseEditor(); return; } \



#endif
