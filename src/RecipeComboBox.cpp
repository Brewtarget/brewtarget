/*
 * RecipeComboBox.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include <QComboBox>
#include <QWidget>
#include <Qt>
#include <list>
#include <string>

#include "database.h"
#include <QString>
#include "RecipeComboBox.h"

RecipeComboBox::RecipeComboBox(QWidget* parent)
        : QComboBox(parent)
{
}

void RecipeComboBox::startObservingDB()
{
   if( Database::isInitialized() )
   {
      dbObs = Database::getDatabase();
      addObserved(dbObs);

      std::list<Recipe*>::iterator it, end;

      end = dbObs->getRecipeEnd();

      for( it = dbObs->getRecipeBegin(); it != end; ++it )
         addRecipe(*it);
      repopulateList();
   }
}

void RecipeComboBox::addRecipe(Recipe* recipe)
{
   recipeObs.push_back(recipe);
   addObserved(recipe);

   addItem( recipe->getName() );
}

void RecipeComboBox::removeAllRecipes()
{
   int i;
   for( i = 0; i < recipeObs.size(); ++i )
      removeObserved(recipeObs[i]);
   recipeObs.clear(); // Clear internal list.
   clear(); // Clear the combo box's visible list.
}

void RecipeComboBox::notify(Observable *notifier, QVariant info)
{
   unsigned int i, size;

   // Notifier could be the database. Only pay attention if the number of
   // recipes has changed.
   if( notifier == dbObs && (info.toInt() == DBRECIPE || info.toInt() == DBALL) )
   {
      removeAllRecipes();
      std::list<Recipe*>::iterator it, end;

      end = dbObs->getRecipeEnd();

      for( it = dbObs->getRecipeBegin(); it != end; ++it )
         addRecipe(*it);
      repopulateList();
   }
   else // Otherwise, we know that one of the recipes changed.
   {
      size = recipeObs.size();
      for( i = 0; i < size; ++i )
         if( notifier == recipeObs[i] )
         {
            // Notice we assume 'i' is an index into both 'recipeObs' and also
            // to the text list in this combo box...
            setItemText(i, recipeObs[i]->getName() );
         }
   }
}

void RecipeComboBox::setIndexByRecipeName(QString name)
{
   int ndx;

   ndx = findText( name, Qt::MatchExactly );

   setCurrentIndex(ndx);
}

void RecipeComboBox::setIndex(int ndx)
{
   setCurrentIndex(ndx);
}

void RecipeComboBox::repopulateList()
{
   unsigned int i, size;
   clear();

   size = recipeObs.size();
   for( i = 0; i < size; ++i )
      addItem( recipeObs[i]->getName() );
}

Recipe* RecipeComboBox::getSelectedRecipe()
{
   if( currentIndex() >= 0 )
      return recipeObs[currentIndex()];
   else
      return 0;
}
