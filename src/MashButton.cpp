/*
 * MashButton.cpp is part of Brewtarget, and is Copyright the following
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

#include "MashButton.h"
#include "mash.h"
#include "recipe.h"
#include <QWidget>
#include <QDebug>

MashButton::MashButton(QWidget* parent)
   : QPushButton(parent),
     _rec(0),
     _mash(0)
{
}

void MashButton::setRecipe(Recipe* rec)
{

   if(_rec)
      disconnect( _rec, 0, this, 0 );

   _rec = rec;
   if( _rec )
   {
      connect( _rec, &BeerXMLElement::changed, this, &MashButton::recChanged );
      setMash( _rec->mash() );
   }
   else
      setMash(0);
}

void MashButton::setMash(Mash* mash)
{
   if( _mash )
      disconnect( _mash, 0, this, 0 );
   
   _mash = mash;
   if( _mash )
   {
      connect( _mash, &BeerXMLElement::changed, this, &MashButton::mashChanged );
      setText( _mash->name() );
   }
   else
      setText("");
}

// This is a bit different from the other buttons. I think we need this
// because the mash tab is the only tab where you can delete stuff directly.
Mash* MashButton::mash() { return _mash; }

void MashButton::mashChanged(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());
   if( propName == "name" )
      setText( val.toString() );
}

void MashButton::recChanged(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());
   
   if( propName == "mash" )
      setMash( qobject_cast<Mash*>(BeerXMLElement::extractPtr(val)) );
}
