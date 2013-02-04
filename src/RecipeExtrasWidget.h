/*
* RecipeExtrasWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2011-2013.
*
* Brewtarget is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* Brewtarget is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RECIPEEXTRASWIDGET_H
#define RECIPEEXTRASWIDGET_H

class RecipeExtrasWidget;

#include <QDialog>
#include <QWidget>
#include <QMetaProperty>
#include <QVariant>
#include "ui_recipeExtrasWidget.h"

// Forward declarations
class Recipe;

/*!
 * \class RecipeExtrasWidget
 * \author Philip G. Lee
 *
 * \brief View/controller widget for modifying "extra" recipe fields.
 */
class RecipeExtrasWidget : public QWidget, public Ui::recipeExtrasWidget
{
   Q_OBJECT

public:
   RecipeExtrasWidget(QWidget* parent=0);
   virtual ~RecipeExtrasWidget() {}
   //! \brief Set the recipe to view/edit.
   void setRecipe(Recipe* rec);

private slots:
   void updateBrewer();
   void updateBrewerAsst();
   void updateTasteRating();
   void updatePrimaryAge();
   void updatePrimaryTemp();
   void updateSecondaryAge();
   void updateSecondaryTemp();
   void updateTertiaryAge();
   void updateTertiaryTemp();
   void updateAge();
   void updateAgeTemp();
   void updateDate(const QDate &date = QDate());
   void updateCarbonation();
   void updateTasteNotes();
   void updateNotes();

   void changeRatings(int rating);
   void saveAll();

   void changed(QMetaProperty,QVariant);
   
private:
   Recipe* recipe;
   bool ratingChanged;

   void showChanges(QMetaProperty* prop = 0);
};

#endif // RECIPEEXTRASWIDGET_H

