/*
 * RecipeComboBox.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QComboBox>
#include <QWidget>
#include <QList>
#include <QString>
#include "database.h"
#include "recipe.h"
#include "RecipeComboBox.h"

RecipeComboBox::RecipeComboBox(QWidget* parent)
        : QComboBox(parent)
{
}

void RecipeComboBox::observeDatabase(bool val)
{
   if( val )
      connect( &(Database::instance()), SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   else
      disconnect( &(Database::instance()), 0, this, 0 );
}

void RecipeComboBox::addRecipe(Recipe* recipe)
{
   if( !recipeObs.contains(recipe) )
   {
      recipeObs.push_back(recipe);
      connect( recipe, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      addItem( recipe->name() );
   }
}

void RecipeComboBox::removeRecipe(Recipe* recipe)
{
   int i = recipeObs.indexOf(recipe);
   if( i >= 0 )
   {
      recipeObs.removeAt(i);
      disconnect( recipe, 0, this, 0 );
      removeItem( i );
   }
}

void RecipeComboBox::removeAllRecipes()
{
   int i;
   for( i = 0; i < recipeObs.size(); ++i )
      removeRecipe(recipeObs[i]);
}

void RecipeComboBox::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;

   QString propName(prop.name());
   
   // Notifier could be the database.
   if( sender() == &(Database::instance()) && propName == "recipes" )
   {
      removeAllRecipes();
      repopulateList();
   }
   else // Otherwise, we know that one of the recipes changed.
   {
      Recipe* recSender = qobject_cast<Recipe*>(sender());
      i = recipeObs.indexOf(recSender);
      if( i >= 0 )
      {
         // Notice we assume 'i' is an index into both 'recipeObs' and also
         // to the text list in this combo box...
         setItemText(i, recipeObs[i]->name() );
      }
   }
}

void RecipeComboBox::setIndexByRecipe(Recipe* rec)
{
   int ndx = recipeObs.indexOf(rec);
   setCurrentIndex(ndx);
}

/*
void RecipeComboBox::setIndex(int ndx)
{
   setCurrentIndex(ndx);
}
*/

void RecipeComboBox::repopulateList()
{
   int i, size;
   clear();

   QList<Recipe*> tmp = Database::instance().recipes();
   size = tmp.size();
   for( i = 0; i < size; ++i )
      addRecipe(tmp[i]);
}

Recipe* RecipeComboBox::getSelectedRecipe()
{
   if( currentIndex() >= 0 )
      return recipeObs[currentIndex()];
   else
      return 0;
}
