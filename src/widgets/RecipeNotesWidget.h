/*======================================================================================================================
 * widgets/RecipeNotesWidget.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#ifndef WIDGETS_RECIPENOTESWIDGET_H
#define WIDGETS_RECIPENOTESWIDGET_H
#pragma once

#include <QMetaProperty>
#include <QVariant>
#include <QWidget>

#include "ui_recipeNotesWidget.h"

// Forward declarations
class Recipe;

/*!
 * \class RecipeNotesWidget
 *
 * \brief View/controller widget for a recipe's notes and taste notes.  (These used to be squeezed into the side of the
 *        "Extras" tab, but they get a tab of their own so there is room to write in them.)
 */
class RecipeNotesWidget : public QWidget, public Ui::recipeNotesWidget {
   Q_OBJECT

public:
   explicit RecipeNotesWidget(QWidget * parent = nullptr);
   ~RecipeNotesWidget() override;

   //! \brief Set the recipe to view/edit.
   void setRecipe(Recipe * recipe);

private slots:
   void updateNotes();
   void updateTasteNotes();

   void changed(QMetaProperty, QVariant);

protected:
   /**
    * \brief Overrides \c QWidget::changeEvent so that, on \c QEvent::LanguageChange, we update our text to the new
    *        language.
    */
   void changeEvent(QEvent * event) override;

private:
   void showChanges(QMetaProperty const * prop = nullptr);

   Recipe * m_recipeObs = nullptr;
};

#endif
