/*
 * StyleButton.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "StyleButton.h"
#include "style.h"
#include "recipe.h"
#include <QWidget>
#include <QDebug>

StyleButton::StyleButton(QWidget* parent)
   : QPushButton(parent),
     _rec(0),
     _style(0)
{
}

void StyleButton::setRecipe(Recipe* rec)
{

   if(_rec)
      disconnect( _rec, 0, this, 0 );

   _rec = rec;
   if( _rec )
   {
      connect( _rec, &BeerXMLElement::changed, this, &StyleButton::recChanged );
      setStyle( _rec->style() );
   }
   else
      setStyle(0);
}

void StyleButton::setStyle(Style* style)
{
   if( _style )
      disconnect( _style, 0, this, 0 );
   
   _style = style;
   if( _style )
   {
      connect( _style, &BeerXMLElement::changed, this, &StyleButton::styleChanged );
      setText( _style->name() );
   }
   else
      setText("");
}

void StyleButton::styleChanged(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());
   if( propName == "name" )
      setText( val.toString() );
}

void StyleButton::recChanged(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());
   
   if( propName == "style" )
      setStyle( qobject_cast<Style*>(BeerXMLElement::extractPtr(val)) );
}
