/*
 * PageText.h is part of Brewtarget, and is Copyright the following
 * authors 2021
 * - Mattias Måhl <mattias@kejsarsten.com>
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

#include "PageText.h"
namespace nBtPage
{

   PageText::PageText(QString value, QFont font)
   {
      Value = value;
      Font = font;
      QFontMetrics fm(Font);
      setBoundingBox( 0 , 0, fm.horizontalAdvance(Value), fm.height());
   }

   void PageText::render(QPainter *painter)
   {
      QFont old = painter->font();
      painter->setFont(Font);
      QFontMetrics fm(Font);
      setBoundingBox(position().x(), position().y(), fm.horizontalAdvance(Value), fm.height());
      painter->drawText(*boundingBox, Value, Options);
      painter->setFont(old);
   }

   int PageText::count()
   {
      return Value.count();
   }

   QSize PageText::getSize()
   {
      QFontMetrics fm(Font);
      return QSize(fm.horizontalAdvance(Value), fm.height());
   }

   void PageText::calculateBoundingBox(QPainter *painter)
   {
      QFontMetrics fm(Font);
      setBoundingBox(fm.boundingRect(Value));
      boundingBox->moveTopLeft(position());
   }
}