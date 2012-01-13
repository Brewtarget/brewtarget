/*
 * RecipeComboBox.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
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
#include <QMetaProperty>
#include <QVariant>
#include <QList>

// Forward declarations.
class Recipe;

/*!
 * \class RecipeComboBox
 * \author Philip G. Lee
 *
 * View/model combobox for viewing the recipes in the database. Really need
 * to separate the view from the model.
 */
class RecipeComboBox : public QComboBox
{
   Q_OBJECT

public:
   RecipeComboBox(QWidget* parent=0);
   virtual ~RecipeComboBox() {}
   //! Get populated with all the database recipes.
   void observeDatabase(bool val);
   //! Add a recipe to the internal model.
   void addRecipe(Recipe* recipe);
   //! Remove a recipe from the internal model.
   void removeRecipe(Recipe* recipe);
   //! Set the current index to that of the given recipe.
   void setIndexByRecipe(Recipe* recipe);
   //! Remove all recipes from the internal model.
   void removeAllRecipes();
   //! Repopulate our list from the database.
   void repopulateList();

   Recipe* getSelectedRecipe();

public slots:
   void changed(QMetaProperty, QVariant);

private:
   QList<Recipe*> recipeObs;
};

#endif   /* _RECIPECOMBOBOX_H */

