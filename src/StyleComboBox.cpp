/*
 * StyleComboBox.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "StyleComboBox.h"
#include <list>

StyleComboBox::StyleComboBox(QWidget* parent)
        : QComboBox(parent)
{
   recipeObs = 0;
}

void StyleComboBox::startObservingDB()
{
   if( ! Database::isInitialized() )
      Database::initialize();

   dbObs = Database::getDatabase();
   addObserved(dbObs);

   std::list<Style*>::iterator it, end;

   end = dbObs->getStyleEnd();

   for( it = dbObs->getStyleBegin(); it != end; ++it )
      addStyle(*it);
   repopulateList();

   setCurrentIndex(-1);
}

void StyleComboBox::addStyle(Style* style)
{
   styleObs.push_back(style);
   addObserved(style);

   addItem( style->getName() );
}

void StyleComboBox::removeAllStyles()
{
   int i;
   for( i = 0; i < styleObs.size(); ++i )
      removeObserved(styleObs[i]);
   styleObs.clear(); // Clear the internal list.
   clear(); // Clear the combo box's visible list.
}

void StyleComboBox::notify(Observable *notifier, QVariant info)
{
   unsigned int i, size;

   // Notifier could be the database.
   if( notifier == dbObs && (info.toInt() == DBSTYLE || info.toInt() == DBALL) )
   {
      Style* previousSelection = getSelected(); // Remember what we had.
      
      removeAllStyles();
      std::list<Style*>::iterator it, end;

      end = dbObs->getStyleEnd();

      for( it = dbObs->getStyleBegin(); it != end; ++it )
         addStyle(*it);
      repopulateList();

      // Need to reset the selected entry if we observe a recipe.
      if( recipeObs && recipeObs->getStyle() )
         setIndexByStyleName( (recipeObs->getStyle())->getName() );
      // Or, try to select the same thing we had selected last.
      else if( previousSelection )
         setIndexByStyleName(previousSelection->getName());
      else
         setCurrentIndex(-1); // Or just give up.
   }
   else if( notifier == recipeObs )
   {
      // All we care about is the style in the recipe.
      if( recipeObs->getStyle() )
         setIndexByStyleName( (recipeObs->getStyle())->getName() );
      else
         setCurrentIndex(-1); // Or just give up.
   }
   else // Otherwise, we know that one of the styles changed.
   {
      size = styleObs.size();
      for( i = 0; i < size; ++i )
         if( notifier == styleObs[i] )
         {
            // Notice we assume 'i' is an index into both 'styleObs' and also
            // to the text list in this combo box...
            setItemText(i, styleObs[i]->getName());
         }
   }
}

void StyleComboBox::setIndexByStyleName(QString name)
{
   int ndx;

   ndx = findText( name, Qt::MatchExactly );
   /*
   if( ndx == -1 )
      return;
   */
   
   setCurrentIndex(ndx);
}

void StyleComboBox::repopulateList()
{
   unsigned int i, size;
   clear(); // Remove all items in the visible list.

   size = styleObs.size();
   for( i = 0; i < size; ++i )
      addItem( styleObs[i]->getName() );
}

Style* StyleComboBox::getSelected()
{
   if( currentIndex() >= 0 )
      return styleObs[currentIndex()];
   else
      return 0;
}

void StyleComboBox::observeRecipe(Recipe* rec)
{
   // Make sure caller isn't stupid.
   if( rec )
   {
      // Remove any previous association.
      if( recipeObs )
         removeObserved(recipeObs);
      
      recipeObs = rec;
      addObserved(recipeObs);

      if( recipeObs->getStyle() )
         setIndexByStyleName( (recipeObs->getStyle())->getName() );
      else
         setCurrentIndex(-1);
   }
}
