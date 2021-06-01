/*
 * PageChildObject.h is part of Brewtarget, and is Copyright the following
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
#ifndef _PAGEOBJECT_H
#define _PAGEOBJECT_H
#include <QPoint>
#include <QPainter>
#include <QObject>
#include <QFont>
#include <QRect>
#include "BtPage.h"


namespace BtPage
{
   enum struct PlacingFlags
   {
      BELOW = 1,
      ABOVE = 2,
      RIGHTOF = 4,
      LEFTOF = 8,
      LEFT = 16,
      RIGHT = 32,
      TOP = 64,
      BOTTOM = 128
   };

   inline PlacingFlags operator|(PlacingFlags a, PlacingFlags b)
   {
      return static_cast<PlacingFlags>(static_cast<int>(a) | static_cast<int>(b));
   }

   inline bool operator&(PlacingFlags a, PlacingFlags b)
   {
      return (static_cast<int>(a) & static_cast<int>(b));
   }

   class PageChildObject
   {
   public:
      QPoint position;
      QFont Font;
      QRect boundingBox;

      //All sub classes from PageChildObject should know how to render them selves.
      virtual void render(QPainter *painter) = 0;

      //Do I really need a template? or is it sufficient widht passing in the PageChildObject pointer?
      /* \!brief
      Place a PageChildObject on a page relational to another PageChildObject.
      PlacingFlags can be stacked together for placement.
      for example
      myobject->placeRelationalTo(&other object, PlacingFlags::LEFTOF | PlacingFlags::ABOVE, 30, 30);
      Valid flags for this is:
         - PlacingFlags::BELOW
         - PlacingFlags::ABOVE
         - PlacingFlags::RIGHTOF
         - PlacingFlags::LEFTOF
      */
      template <class T>
      void placeRelationalTo(T *obj, PlacingFlags place, int xPadding = 0, int yPadding = 0)
      {
         PageChildObject *other = (PageChildObject*)obj;
         int x, y;
         x = other->position.x();
         y = other->position.y();

         y = (place & PlacingFlags::ABOVE) ? y - boundingBox.height() - yPadding : y;
         y = (place & PlacingFlags::BELOW) ? y + other->boundingBox.height() + yPadding : y;
         x = (place & PlacingFlags::LEFTOF) ? x - boundingBox.width() - xPadding : x;
         x = (place & PlacingFlags::RIGHTOF) ? x + other->boundingBox.width() + xPadding : x;

         position = QPoint(x, y);
      }
      /* \!brief
      Place a PageChildObject on a page relational to the page.
      PlacingFlags can be stacked together for placement.
      The object has to be placed on a page object before calling this as the page sizes are needed for the caclulations.
      i.e.
      Page * myPage = new Page(QPrinter);
      PageImage * myObject = myPage.addChildObject( new PageImage(.....));
      myobject->placeRelationalTo(&other object, PlacingFlags::TOP | PlacingFlags::RIGHT);

      Valid flags for this is:
         - PlacingFlags::LEFT
         - PlacingFlags::RIGHT
         - PlacingFlags::TOP
         - PlacingFlags::BOTTOM
         all other Flags will be ignored.
      */
      void placeOnPage(PlacingFlags place, int xPadding = 0, int yPadding = 0);
      void setBoundingbox(QRect rect);
      void setBoundingBox(int x, int y, int width, int height);
      void setParent(Page * parent) { _parent = parent; }
   private:
      Page * _parent;
   };
}
#endif /* _PAGEOBJECT_H */