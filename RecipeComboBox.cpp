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

   addItem( tr(recipe->getName().c_str()) );
}

void RecipeComboBox::removeAllRecipes()
{
   /*
   removeAllObserved(); // Don't want to observe anything.
   recipeObs.clear(); // Delete internal list.
    */

   int i;
   for( i = 0; i < recipeObs.size(); ++i )
      removeObserved(recipeObs[i]);
   recipeObs.clear(); // Clear internal list.
   clear(); // Clear the combo box's visible list.
}

void RecipeComboBox::notify(Observable *notifier)
{
   unsigned int i, size;

   // Notifier could be the database. Only pay attention if the number of
   // recipes has changed.
   if( notifier == dbObs && dbObs->getNumRecipes() != recipeObs.size() )
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
            setItemText(i, tr(recipeObs[i]->getName().c_str()));
         }
   }
}

void RecipeComboBox::setIndexByRecipeName(std::string& name)
{
   int ndx;

   ndx = findText( tr(name.c_str()), Qt::MatchExactly );
   if( ndx == -1 )
      return;

   setCurrentIndex(ndx);
}

void RecipeComboBox::repopulateList()
{
   unsigned int i, size;
   clear();

   size = recipeObs.size();
   for( i = 0; i < size; ++i )
      addItem( tr(recipeObs[i]->getName().c_str()) );
}
