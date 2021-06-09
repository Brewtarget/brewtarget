/*
 * BtPage.h is part of Brewtarget, and is Copyright the following
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
#ifndef _BTPAGE_H
#define _BTPAGE_H
#include <QFont>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPrinter>
#include <QRect>
#include <QMargins>
#include "PageTable.h"
#include "PageText.h"
#include "PageChildObject.h"
#include "BtEnumFlags.h"

namespace nBtPage
{
   class BtPage
   {
   public:
      QPrinter *printer;
      QPainter painter;

      ~BtPage() {};
      BtPage(QPrinter *printer);

      template <class T>
      auto addChildObject(T *obj, QPoint position = QPoint()) -> decltype(obj)
      {
         if ( ! position.isNull() ) obj->setPosition(position);
         obj->parent = this;
         _children.append(obj);
         return obj;
      }

      void renderPage();

      /**
       * @brief
       * Place 'target' PageChildObject on a page relational to another PageChildObject.
       * PlacingFlags can be stacked together for placement.
       * for example
       * myobject->placeRelationalTo(&other object, PlacingFlags::LEFTOF | PlacingFlags::ABOVE, 30, 30);
       * Valid flags for this is:
       *   - PlacingFlags::BELOW
       *   - PlacingFlags::ABOVE
       *   - PlacingFlags::RIGHTOF
       *   - PlacingFlags::LEFTOF
       *
       * @param targetObj Object to move on the page
       * @param sourceObj Object to place target in relation to.
       * @param place Placingflag (BELOW, ABOVE, RIGHTOF, LEFTOF
       * @param xOffset if you want to offset the placing in x-axis in pixels, defaults 0 px
       * @param yOffset if you want to offset the placing in y-direction pixels, defaults 0 px
       */
      template <class T, class S>
      void placeRelationalTo(T *targetObj, S *sourceObj, PlacingFlags place, int xOffset = 0 /* pixels */, int yOffset = 0 /* pixels */)
      {
         PageChildObject *other = (PageChildObject*)sourceObj;
         PageChildObject *target = (PageChildObject*)targetObj;
         int x, y;
         x = other->position().x();
         y = other->position().y();

         y = (place & PlacingFlags::ABOVE) ? y - target->getBoundingBox().height() - yOffset : y;
         y = (place & PlacingFlags::BELOW) ? y + other->getBoundingBox().height() + yOffset : y;
         x = (place & PlacingFlags::LEFTOF) ? x - target->getBoundingBox().width() - xOffset : x;
         x = (place & PlacingFlags::RIGHTOF) ? x + other->getBoundingBox().width() + xOffset : x;

         target->setPosition(QPoint(x, y));
      }

      /**
       * @brief
       * Place 'target' PageChildObject on a page relational to another PageChildObject.
       * PlacingFlags can be stacked together for placement.
       * for example
       * myobject->placeRelationalTo(&other object, PlacingFlags::LEFTOF | PlacingFlags::ABOVE, 30, 30);
       * Valid flags for this is:
       *   - PlacingFlags::BELOW
       *   - PlacingFlags::ABOVE
       *   - PlacingFlags::RIGHTOF
       *   - PlacingFlags::LEFTOF
       *
       * @param targetObj Object to move on the page
       * @param sourceObj Object to place target in relation to.
       * @param place Placingflag (BELOW, ABOVE, RIGHTOF, LEFTOF
       * @param xOffset if you want to offset the placing in x-axis in Millimeter, defaults 0 mm
       * @param yOffset if you want to offset the placing in y-direction in Millimeter, defaults 0 mm
       */
      template <class T, class S>
      void placeRelationalToMM(T *targetObj, S *sourceObj, PlacingFlags place, int xOffset = 0 /* Millimeter */ , int yOffset = 0 /* Millimeter */)
      {
         //Converting the MM offsets to pixels on the page.
         yOffset *= (printer->logicalDpiY() / 25.4);
         xOffset *= (printer->logicalDpiX() / 25.4);

         PageChildObject *other = (PageChildObject*)sourceObj;
         PageChildObject *target = (PageChildObject*)targetObj;
         int x, y;
         x = other->position().x();
         y = other->position().y();

         y = (place & PlacingFlags::ABOVE) ? y - target->getBoundingBox().height() - yOffset : y;
         y = (place & PlacingFlags::BELOW) ? y + other->getBoundingBox().height() + yOffset : y;
         x = (place & PlacingFlags::LEFTOF) ? x - target->getBoundingBox().width() - xOffset : x;
         x = (place & PlacingFlags::RIGHTOF) ? x + other->getBoundingBox().width() + xOffset : x;

         target->setPosition(QPoint(x, y));
      }

      /**
       * @brief
       * Place a PageChildObject on a page relational to the page.
       * PlacingFlags can be stacked together for placement.
       * The object has to be placed on a page object before calling this as the page sizes are needed for the caclulations.
       * i.e.
       * BtPage * myPage = new BtPage(QPrinter);
       * PageImage * myObject = myPage.addChildObject( new PageImage(.....));
       * myobject->placeRelationalTo(&other object, PlacingFlags::TOP | PlacingFlags::RIGHT);
       *
       * Valid flags for this is:
       *    - PlacingFlags::LEFT
       *    - PlacingFlags::RIGHT
       *    - PlacingFlags::TOP
       *    - PlacingFlags::BOTTOM
       *    - PlacingFlags::VCENTER
       *    - PlacingFlags::HCENTER
       *    all other Flags will be ignored.
       *
       * @param targetObj Object to move on the page.
       * @param place Placing flag for placment.
       * @param xOffset Offset from the placement in the x-axix
       * @param yOffset Offset from the placement in the y-axis.
       */
      template <class T>
      void placeOnPage(T *targetObj, PlacingFlags place, int xOffset = 0, int yOffset = 0)
      {
         PageChildObject *tO = (PageChildObject*)targetObj;

         QRectF pagePaintRect = printer->pageLayout().paintRectPixels(printer->logicalDpiX());
         QMarginsF margins = printer->pageLayout().marginsPixels(printer->logicalDpiX());
         int x = tO->position().x() + xOffset;
         int y = tO->position().y() + yOffset;
         x = (place & PlacingFlags::RIGHT) ? pagePaintRect.width() - tO->getBoundingBox().width() : x;
         x = (place & PlacingFlags::LEFT) ? 0 : x;
         y = (place & PlacingFlags::TOP) ? 0 : y;
         y = (place & PlacingFlags::BOTTOM) ? pagePaintRect.height() - tO->getBoundingBox().height() : y;
         y = (place & PlacingFlags::VCENTER) ? ((pagePaintRect.height() - tO->getBoundingBox().height()) / 2) : y;
         x = (place & PlacingFlags::HCENTER) ? ((pagePaintRect.width() - tO->getBoundingBox().width()) / 2) : x;
         tO->setPosition(QPoint(x, y));
         tO->moveBoundingBox(QPoint(x, y));
      }

   private:
      QList<PageChildObject *> _children;
   };
}
#endif /* _BTPAGE_H */
