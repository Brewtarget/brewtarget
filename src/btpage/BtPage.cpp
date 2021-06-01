/*
 * page.cpp is part of Brewtarget, and is Copyright the following
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

#include "BtPage.h"
namespace BtPage
{

   Page::Page(QPrinter *printer)
   {
      _printer = printer;
   }

   void Page::renderPage()
   {
      QPainter *painter = new QPainter(_printer);
      foreach (PageChildObject *child, _children)
      {
         child->render(painter);
      }
      painter->end();
   }
   QRect Page::getPageSize()
   {
      return _printer->paperRect();
   }

   QMargins Page::getPageMargins()
   {
      QMargins margins;
      qreal left, right, top, bottom;

      _printer->getPageMargins(&left, &top, &right, &bottom, QPrinter::Unit::DevicePixel);
      margins.setLeft(left);
      margins.setRight(right);
      margins.setTop(top);
      margins.setBottom(bottom);

      return margins;
   }
}