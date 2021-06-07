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
#include <QPagedPaintDevice>
#include <QRectF>
#include <QMargins>

namespace nBtPage
{

   void PageChildObject::placeOnPage(QPrinter *printer, PlacingFlags place, int xPadding, int yPadding)
   {
      QRectF pagePaintRect = printer->pageLayout().paintRect();
      QMarginsF margins = printer->pageLayout().margins();
      int x = _position.x() - margins.left() + xPadding;
      int y = _position.y() - margins.top() + yPadding;
      x = (place & PlacingFlags::RIGHT) ? pagePaintRect.width() - _boundingBox.width() : x;
      x = (place & PlacingFlags::LEFT) ? 0 : x;
      y = (place & PlacingFlags::TOP) ? 0 : y;
      y = (place & PlacingFlags::BOTTOM) ? pagePaintRect.height() - _boundingBox.height() - margins.bottom() : y;

      _position.setX(x);
      _position.setY(y);
      _boundingBox.moveTopLeft(_position);
   }

   void PageChildObject::setBoundingBox(QRect rect)
   {
      _boundingBox = rect;
   }

   void PageChildObject::setBoundingBox(int x, int y, int width, int height)
   {
      setBoundingBox(QRect(x, y, width, height));
   }

   void PageChildObject::setBoundingBox(QPoint p, int width, int height)
   {
      setBoundingBox(QRect(p.x(), p.y(), width, height));
   }

   void PageChildObject::setPosition(QPoint point)
   {
      _position = QPoint(point);
      if ( _boundingBox.isEmpty() )
         _boundingBox = QRect();
      setBoundingBox(_position.x(), _position.y(), _boundingBox.width(), _boundingBox.height());
   }
}