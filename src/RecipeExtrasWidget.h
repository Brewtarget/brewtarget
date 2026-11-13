/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * RecipeExtrasWidget.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef RECIPEEXTRASWIDGET_H
#define RECIPEEXTRASWIDGET_H
#pragma once

#include <QMetaProperty>
#include <QVariant>
#include <QWidget>

#include "ui_recipeExtrasWidget.h"

// Forward declarations
class Recipe;
class QDate;

/*!
 * \class RecipeExtrasWidget
 *
 * \brief View/controller widget for modifying "extra" recipe fields.
 */
class RecipeExtrasWidget : public QWidget, public Ui::recipeExtrasWidget {
   Q_OBJECT

public:
   explicit RecipeExtrasWidget(QWidget * parent = nullptr);
   ~RecipeExtrasWidget() override;
   //! \brief Set the recipe to view/edit.
   void setRecipe(Recipe* rec);

private slots:
   void updateBrewer();
   void updateBrewerAsst();
   void updateRecipeWortClarity();
   void updateTasteRating();
   void updateAge();
   void updateAgeTemp();
   void updateDate(std::optional<QDate> const date);
   void updateCarbonation();
   void updateTasteNotes();
   void updateNotes();
   void updateAcidity();
   void updateAttenuation();

   void changeRatings(int rating);
   void saveAll();

   void changed(QMetaProperty, QVariant);

private:
   Recipe* m_recipeObs;
   bool ratingChanged;

   void showChanges(QMetaProperty * prop = nullptr);
};

#endif
