/*
 * StyleComboBox.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "StyleComboBox.h"
#include <QList>

StyleComboBox::StyleComboBox(QWidget* parent)
        : QComboBox(parent), recipe(0)
{
   setCurrentIndex(-1);
   connect( &(Database::instance()), SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   repopulateList();
}

void StyleComboBox::addStyle(Style* style)
{
   if( !styles.contains(style) )
      styles.append(style);
   connect( style, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   
   addItem( style->name() );
}

void StyleComboBox::removeStyle(Style* style)
{
   disconnect( style, 0, this, 0 );
   int ndx = styles.indexOf(style);
   if( ndx > 0 )
   {
      styles.removeAt(ndx);
      removeItem(ndx);
   }
}

void StyleComboBox::changed(QMetaProperty prop, QVariant val)
{
   unsigned int i, size;
   
   // Notifier could be the database.
   if( sender() == &(Database::instance()) &&
      prop.propertyIndex() == Database::instance().metaObject().indexOfProperty("styles") )
   {
      Style* previousSelection = getSelected();
         
      repopulateList();
         
      // Need to reset the selected entry if we observe a recipe.
      if( recipe && recipe->style() )
         setIndexByStyle( recipe->style() );
      // Or, try to select the same thing we had selected last.
      else if( previousSelection )
         setIndexByStyle(previousSelection);
      else
         setCurrentIndex(-1); // Or just give up.
   }
   else if( sender() == recipe )
   {
      // Only respond if the style changed.
      if( prop.propertyIndex() != recipe->metaObject().indexOfProperty("style") )
         return;
         
      // All we care about is the style in the recipe.
      if( recipe->style() )
         setIndexByStyle( recipeObs->getStyle() );
      else
         setCurrentIndex(-1); // Or just give up.
   }
   else // Otherwise, we know that one of the styles changed.
   {
      Style* s = qobject_cast<Style*>(sender());
      i = equipments.indexOf(s);
      if( i > 0 )
         setItemText(i, styles[i]->name());
   }
}

void StyleComboBox::setIndexByStyle(Style* s)
{
   int ndx;

   ndx = styles.indexOf(s);
   setCurrentIndex(ndx);
}

void StyleComboBox::repopulateList()
{
   int i, size;
   clear(); // Remove all items in the visible list.

   // Disconnect all  current styles.
   size = styles.size();
   for( i = 0; i < size; ++i )
      removeStyle(styles[i]);
   
   // Get new list of styles.
   Database::instance().getStyle( styles );
   
   // Connect and add all new styles.
   size = styles.size();
   for( i = 0; i < size; ++i )
      addStyle(styles[i]);
}

Style* StyleComboBox::getSelected()
{
   if( currentIndex() >= 0 )
      return styles[currentIndex()];
   else
      return 0;
}

void StyleComboBox::observeRecipe(Recipe* rec)
{
   if( rec )
   {
      if( recipe )
         disconnect( recipe, 0, this, 0 );
      
      recipe = rec;
      connect( recipe, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );

      if( recipe->style() )
         setIndexByStyle( recipe->style() );
      else
         setCurrentIndex(-1);
   }
}
