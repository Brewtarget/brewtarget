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
namespace nBtPage
{

   BtPage::BtPage(QPrinter *printer)
   {
      _printer = printer;
      _printer->setResolution(QPrinter::HighResolution);

      /*
      This is only until the Template file system is in place.
      My though here is to have the template govern the settings for the page including units and such.
      also need to add scaling between page sizes at that poing! but thats for later!
      */
      QPageLayout layout = _printer->pageLayout();
      layout.setUnits(QPageLayout::Point);
      QMarginsF margins = layout.margins();

      int lDpiX = _printer->logicalDpiX();
      int lDpiY = _printer->logicalDpiY();

      int margin = 20;
      margins.setBottom( margin );
      margins.setTop( margin );
      margins.setLeft( margin );
      margins.setRight( margin );
      layout.setMargins(margins);
      _printer->setPageLayout(layout);
   }

   void BtPage::renderPage()
   {
      QPainter *painter = new QPainter(_printer);
      QRectF r = _printer->pageLayout().fullRect();
      QMarginsF margins = _printer->pageLayout().margins();
      painter->setWindow(margins.left(), margins.right(), r.width(), r.height());
      painter->setViewport(0, 0, _printer->width(), _printer->height());
      painter->translate(margins.left(), margins.top());
      foreach (PageChildObject *child, _children)
      {
         child->calculateBoundingBox();
         child->render(painter);
      }
      painter->end();
   }
}