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
#include "Page.h"

namespace BtPage
{
   void PageChildObject::setBoundingBox(QRect rect)
   {
      itemBoundingBox = rect;
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
      itemPosition = QPoint(point);
      moveBoundingBox(point);
   }

   /**
    * @brief Sets the position of the object on the page. messurements in millimeter.
    *
    * @param x
    * @param y
    */
   void PageChildObject::setPositionMM(int x, int y)
   {
      //Convert MM to pixel on the page.
      x *= (parent->printer->logicalDpiX() / 25.4);
      y *= (parent->printer->logicalDpiY() / 25.4);
      setPosition(QPoint(x, y));
   }

   /**
    * @brief Get the Font Horizontal Advance for a given string.
    * Thsi is QT version sensitive, using different methods depanding on QT version.
    * Since Qt-version 5.13 they introdued QFontMetrics::horizontalAdvance(const QString &text, int len = -1) to get the width of a given string.
    * Before that there was the QFontMetrics::width(const QString &text, int len = -1) function to do the same.
    *
    * @param fontMetrics
    * @param text
    * @return int
    */
   int PageChildObject::getFontHorizontalAdvance(QFontMetrics fontMetrics, QString text)
   {
      #if QT_VERSION < QT_VERSION_CHECK(5,13,0)
         return fontMetrics.width(text);
      #else
         return fontMetrics.horizontalAdvance(text);
      #endif
   }
}