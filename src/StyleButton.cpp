/*
 * StyleButton.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/Style.h"
#include "model/Recipe.h"
#include <QWidget>
#include <QDebug>

StyleButton::StyleButton(QWidget* parent)
   : QPushButton(parent),
     m_rec(nullptr),
     _style(nullptr)
{
}

void StyleButton::setRecipe(Recipe* rec)
{

   if(m_rec)
      disconnect( m_rec, nullptr, this, nullptr );

   m_rec = rec;
   if( m_rec )
   {
      connect( m_rec, &NamedEntity::changed, this, &StyleButton::recChanged );
      setStyle( m_rec->style() );
   }
   else
      setStyle(nullptr);
}

void StyleButton::setStyle(Style* style)
{
   if( _style )
      disconnect( _style, nullptr, this, nullptr );

   _style = style;
   if( _style )
   {
      connect( _style, &NamedEntity::changed, this, &StyleButton::styleChanged );
      setText( _style->name() );
   }
   else
      setText("");
}

void StyleButton::styleChanged(QMetaProperty prop, QVariant val) {
   if (prop.name() == PropertyNames::NamedEntity::name) {
      this->setText(val.toString());
   }
   return;
}

void StyleButton::recChanged(QMetaProperty prop, QVariant val) {
   if (prop.name() == PropertyNames::Recipe::style) {
      this->setStyle(val.value<Style*>());
   }
   return;
}
