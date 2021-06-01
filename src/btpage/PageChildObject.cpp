/*
 * PageChildObject.cpp is part of Brewtarget, and is Copyright the following
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

#include "PageChildObject.h"
namespace BtPage
{

   void PageChildObject::placeOnPage(PlacingFlags place, int xPadding, int yPadding)
   {
      if (_parent == nullptr) return;

      QMargins margins = _parent->getPageMargins();
      QRect pageSize = _parent->getPageSize();

      int x = position.x();
      int y = position.y();

      x = (place & PlacingFlags::RIGHT) ? pageSize.right() - margins.right() - boundingBox.right() - xPadding : x;
      x = (place & PlacingFlags::LEFT) ? margins.left() + xPadding : x;
      y = (place & PlacingFlags::TOP) ? margins.top() + yPadding : y;
      y = (place & PlacingFlags::BOTTOM) ? pageSize.height() - margins.bottom() - boundingBox.height() - yPadding : y;

      position.setX(x);
      position.setY(y);
   }

   void PageChildObject::setBoundingbox(QRect rect)
   {
      boundingBox = rect;
   }

   void PageChildObject::setBoundingBox(int x, int y, int width, int height)
   {
      setBoundingbox(QRect(x, y, width, height));
   }
}