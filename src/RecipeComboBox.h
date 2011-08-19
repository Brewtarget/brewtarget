/*
 * RecipeComboBox.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#ifndef _RECIPECOMBOBOX_H
#define   _RECIPECOMBOBOX_H

#include <QComboBox>
#include <QWidget>
#include <QVariant>
#include <QVector>
#include <string>
#include "observable.h"
#include "recipe.h"
#include "database.h"

class RecipeComboBox;

class RecipeComboBox : public QComboBox, public MultipleObserver
{
   Q_OBJECT

public:
   RecipeComboBox(QWidget* parent=0);
   virtual ~RecipeComboBox() {}
   void startObservingDB();
   void addRecipe(Recipe* recipe);
   void setIndexByRecipeName(QString name);
   void setIndex(int ndx);
   void removeAllRecipes();
   void repopulateList();

   Recipe* getSelectedRecipe();

   virtual void notify(Observable *notifier, QVariant info = QVariant()); // This will get called by observed whenever it changes.

private:
   QVector<Recipe*> recipeObs;
   Database* dbObs;
};

#endif   /* _RECIPECOMBOBOX_H */

