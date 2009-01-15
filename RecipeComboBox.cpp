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
#include <QString>
#include "RecipeComboBox.h"

RecipeComboBox::RecipeComboBox(QWidget* parent)
        : QComboBox(parent)
{

}

void RecipeComboBox::addRecipe(Recipe* recipe)
{
   recipeObs.push_back(recipe);
   addObserved(recipe);

   addItem( tr(recipe->getName().c_str()) );
}

void RecipeComboBox::notify(Observable *notifier)
{
   unsigned int i, size;

   size = recipeObs.size();

   for( i = 0; i < size; ++i )
      if( notifier == recipeObs[i] )
         repopulateList();
}

void RecipeComboBox::repopulateList()
{
   unsigned int i, size;
   clear();

   size = recipeObs.size();
   for( i = 0; i < size; ++i )
      addItem( tr(recipeObs[i]->getName().c_str()) );
}
