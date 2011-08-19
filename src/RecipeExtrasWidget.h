/*
* RecipeExtrasWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2011.
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
#include "ui_recipeExtrasWidget.h"
#include "observable.h"
#include "recipe.h"

class RecipeExtrasWidget : public QWidget, public Ui::recipeExtrasWidget, public Observer
{
   Q_OBJECT

public:
   RecipeExtrasWidget(QWidget* parent=0);
   virtual ~RecipeExtrasWidget() {}
   void setRecipe(Recipe* rec);

public slots:
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
   void updateDate();
   void updateCarbonation();
   void updateTasteNotes();
   void updateNotes();

   void saveAll();

private:
   Recipe* recObs;

   void showChanges();

   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer
};

#endif // RECIPEEXTRASWIDGET_H

