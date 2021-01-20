/*
 * WaterButton.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2025
 * - Mik Firestone (mikfire@fastmail.com)
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

#include "WaterButton.h"
#include "water.h"
#include "recipe.h"
#include <QWidget>

WaterButton::WaterButton(QWidget* parent)
   : QPushButton(parent),
     m_rec(nullptr),
     m_water(nullptr)
{
}

void WaterButton::setRecipe(Recipe* rec)
{
   if (m_rec) {
      disconnect( m_rec, nullptr, this, nullptr );
   }

   m_rec = rec;
   if ( m_rec && m_rec->waters().size() > 0 ) {
      connect( m_rec, &Ingredient::changed, this, &WaterButton::recChanged );
      setWater( m_rec->waters().at(0) );
   }
   else {
      setWater(nullptr);
   }
}

void WaterButton::setWater(Water* water)
{
   if ( m_water )
      disconnect( m_water, nullptr, this, nullptr );

   m_water = water;
   if ( m_water ) {
      connect( m_water, &Ingredient::changed, this, &WaterButton::waterChanged );
      setText( m_water->name() );
   }
   else {
      setText("");
   }
}

void WaterButton::waterChanged(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());
   if ( propName == "name" ) {
      setText( val.toString() );
   }
}

void WaterButton::recChanged(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());

   if ( propName == "water" ) {
      setWater( qobject_cast<Water*>(Ingredient::extractPtr(val)) );
   }
}
