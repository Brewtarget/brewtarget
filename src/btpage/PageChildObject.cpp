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
      QRectF pageFullRect = printer->pageLayout().fullRect();
      QMarginsF margins = printer->pageLayout().margins();
      int x = _position.x();
      int y = _position.y();
      x = (place & PlacingFlags::RIGHT) ? pageFullRect.width() - boundingBox->width() - margins.right() - xPadding : x;
      x = (place & PlacingFlags::LEFT) ? margins.left() + xPadding : x;
      y = (place & PlacingFlags::TOP) ? margins.top() + yPadding : y;
      y = (place & PlacingFlags::BOTTOM) ? pageFullRect.height() - boundingBox->height() - margins.bottom() - yPadding : y;

      _position.setX(x);
      _position.setY(y);
      boundingBox->moveTopLeft(_position);
   }

   void PageChildObject::setBoundingBox(QRect rect)
   {
      boundingBox = new QRect(rect);
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
      if ( boundingBox != nullptr )
         boundingBox = new QRect();
      if ( ! boundingBox->isNull() &&  ! point.isNull())
         setBoundingBox(_position.x(), _position.y(), boundingBox->width(), boundingBox->height());
   }
}