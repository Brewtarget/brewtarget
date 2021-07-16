/*
 * PageBreak.h is part of Brewtarget, and is Copyright the following
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

#ifndef BTPAGE_PAGEBREAK_H
#define BTPAGE_PAGEBREAK_H
#include "PageChildObject.h"



namespace BtPage
{
   class Page;
   /**
    * @brief class to handle adding in extra pagebrakes in the generation of prinouts
    *
    */
   class PageBreak : public PageChildObject
   {
   public:
      /**
       * @brief Construct a new Page Break object
       *
       * @param parent
       */
      PageBreak(Page *parent) {
         this->parent = parent;
      }

      /**
       * @brief adds a new page to the printer.
       *
       * @param painter
       */
      void render(QPainter *painter);

      QSize getSize() { return QSize(); }
      void calculateBoundingBox( double scalex = 0.0, double scaley = 0.0 ) {}
   };
}

#endif /* BTPAGE_PAGEBREAK_H */