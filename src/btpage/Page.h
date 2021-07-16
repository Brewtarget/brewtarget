/*
 * Page.h is part of Brewtarget, and is Copyright the following
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
#ifndef BTPAGE_PAGE_H
#define BTPAGE_PAGE_H
#include <QFont>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPrinter>
#include <QRect>
#include <QMargins>
#include "PageTable.h"
#include "PageText.h"
#include "PageBreak.h"
#include "PageChildObject.h"
#include "BtEnumFlags.h"

namespace BtPage
{
   /**
    * @brief Page handles all object that goes on a page for printout to PDF or Paper.
    * @authors
    *    @mattiasmaahl
    *
    * It handles some placing functions to set a position for an object on the page.
    * My goal is to have a platform/model that will enable future development to allow for
    * using template/Specifications file that designs the output on screen.
    * I'm trying to keep everything simple for the developer using the object by keeping
    * to a simple interface where you create an instance of Page, add PageChildOjects to
    * it and then render it to preview it on screen.
    */
   class Page
   {
   public:
      QPrinter *printer;
      QPainter painter;

      ~Page() {};
      Page(QPrinter *printer);

      /**
       * @brief Add an object to the page
       * @authors @mattiasmaahl
       * @tparam T PageChildObject or derived.
       * @param obj Object to store on the page.
       * @param position Where to render it, value in pixels. depends on your printer resulution in the end.
       * @return the created object of supplied type.
       */
      template <class T>
      auto addChildObject(T *obj, QPoint position = QPoint()) -> decltype(obj)
      {
         if ( ! position.isNull() ) obj->setPosition(position);
         items.append(obj);
         return obj;
      }

      /**
       * @brief Renders page and all objects created using addChildOject(...)
       * @authors @mattiasmaahl
       */
      void renderPage();

      /**
       * @brief
       * Place 'target' PageChildObject on a page relational to another PageChildObject.
       * PlacingFlags can be stacked together for placement.
       * for example
       * myobject->placeRelationalTo(&other object, PlacingFlags::LEFTOF | PlacingFlags::ABOVE, 30, 30);
       * Valid flags for this is:
       *   - BtPage::BELOW
       *   - BtPage::ABOVE
       *   - BtPage::RIGHTOF
       *   - BtPage::LEFTOF
       *
       * @authors
       *    @mattiasmaahl

       * @param targetObj Object to move on the page
       * @param sourceObj Object to place target in relation to.
       * @param place Placingflag (BELOW, ABOVE, RIGHTOF, LEFTOF
       * @param xOffset if you want to offset the placing in x-axis in pixels, defaults 0 px
       * @param yOffset if you want to offset the placing in y-direction pixels, defaults 0 px
       */
      template <class T, class S>
      void placeRelationalTo(T *targetObj, S *sourceObj, RelationalPlacingFlags place, int xOffset = 0 /* pixels */, int yOffset = 0 /* pixels */)
      {
         PageChildObject *other = (PageChildObject*)sourceObj;
         //If the source object goes beyond the page and is doing a page brake, we need call our selves with that child to get to the last in the list.
         //this way we get the bottom coordinates from the last part of the object, thus placing ourselves to the right coordinates.
         if (other->needPageBreak && other->nextSection != nullptr) {
            placeRelationalTo( targetObj, other->nextSection, place, xOffset, yOffset );
            //after this we return so we done brake everuthing after.
            return;
         }

         PageChildObject *target = (PageChildObject*)targetObj;
         int x, y;
         x = other->position().x() + xOffset;
         y = other->position().y() + yOffset;
         other->calculateBoundingBox();
         target->calculateBoundingBox();
         y = (place & BtPage::ABOVE) ? y - target->getBoundingBox().height() : y;
         y = (place & BtPage::BELOW) ? y + other->getBoundingBox().height() : y;
         x = (place & BtPage::LEFTOF) ? x - target->getBoundingBox().width() : x;
         x = (place & BtPage::RIGHTOF) ? x + other->getBoundingBox().width() : x;

         target->setPosition(QPoint(x, y));
         target->calculateBoundingBox();
      }

      /**
       * @brief
       * This is a wrapper function to easier place object relational to another object giving the the Offsets in Millimeter.
       *
       * Place 'target' PageChildObject on a page relational to another PageChildObject.
       * PlacingFlags can be stacked together for placement.
       * for example
       * myobject->placeRelationalTo(&other object, PlacingFlags::LEFTOF | PlacingFlags::ABOVE, 30, 30);
       * Valid flags for this is:
       *   - BtPage::BELOW
       *   - BtPage::ABOVE
       *   - BtPage::RIGHTOF
       *   - BtPage::LEFTOF
       *
       * @author @mattiasmaahl
       *
       * @param targetObj Object to move on the page
       * @param sourceObj Object to place target in relation to.
       * @param place Placingflag (BELOW, ABOVE, RIGHTOF, LEFTOF
       * @param xOffset if you want to offset the placing in x-axis in Millimeter, defaults 0 mm
       * @param yOffset if you want to offset the placing in y-direction in Millimeter, defaults 0 mm
       */
      template <class T, class S>
      void placeRelationalToMM(T *targetObj, S *sourceObj, RelationalPlacingFlags place, int xOffset = 0 /* Millimeter */ , int yOffset = 0 /* Millimeter */)
      {
         //Converting the MM offsets to pixels on the page.
         yOffset *= (printer->logicalDpiY() / 25.4);
         xOffset *= (printer->logicalDpiX() / 25.4);

         placeRelationalTo(targetObj, sourceObj, place, xOffset, yOffset);
      }

      /**
       * @brief
       * Place a PageChildObject on a page relational to the page.
       * PlacingFlags can be stacked together for placement.
       * The object has to be placed on a page object before calling this as the page sizes are needed for the caclulations.
       * i.e.
       * Page * myPage = new Page(QPrinter);
       * PageImage * myObject = myPage.addChildObject( new PageImage(.....));
       * myobject->placeRelationalTo(&other object, PlacingFlags::TOP | PlacingFlags::RIGHT);
       *
       * Valid flags for this is:
       *    - BtPage::LEFT
       *    - BtPage::RIGHT
       *    - BtPage::TOP
       *    - BtPage::BOTTOM
       *    - BtPage::VCENTER
       *    - BtPage::HCENTER
       *    all other Flags will be ignored.
       *
       * @author @mattiasmaahl
       *
       * @param targetObj Object to move on the page.
       * @param place Placing flag for placment.
       * @param xOffset Offset from the placement in the x-axix
       * @param yOffset Offset from the placement in the y-axis.
       */
      template <class T>
      void placeOnPage(T *targetObj, FixedPlacingFlags place, int xOffset = 0, int yOffset = 0)
      {
         PageChildObject *tO = (PageChildObject*)targetObj;

         QRectF pagePaintRect = printer->pageLayout().paintRectPixels(printer->logicalDpiX());
         int x = tO->position().x() + xOffset;
         int y = tO->position().y() + yOffset;
         x = (place & BtPage::RIGHT) ? pagePaintRect.width() - tO->getBoundingBox().width() : x;
         x = (place & BtPage::LEFT) ? 0 : x;
         y = (place & BtPage::TOP) ? 0 : y;
         y = (place & BtPage::BOTTOM) ? pagePaintRect.height() - tO->getBoundingBox().height() : y;
         y = (place & BtPage::VCENTER) ? ((pagePaintRect.height() - tO->getBoundingBox().height()) / 2) : y;
         x = (place & BtPage::HCENTER) ? ((pagePaintRect.width() - tO->getBoundingBox().width()) / 2) : x;
         tO->setPosition(QPoint(x, y));
         tO->moveBoundingBox(QPoint(x, y));
      }

      template <class T>
      void placeOnPageMM(T *targetObj, FixedPlacingFlags place, int xOffset = 0, int yOffset = 0)
      {
         //Converting the MM offsets to pixels on the page.
         yOffset *= (printer->logicalDpiY() / 25.4);
         xOffset *= (printer->logicalDpiX() / 25.4);

         placeOnPage(targetObj, place, xOffset, yOffset);
      }

   private:
      QList<PageChildObject *> items;
   };
}
#endif /* BTPAGE_PAGE_H */
