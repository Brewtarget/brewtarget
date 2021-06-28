/*
 * PageTableStyle.h is part of Brewtarget, and is Copyright the following
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
#include <QColor>
#include <QBrush>

#ifndef BTPAGE_PAGETABLESTYLE_H
#define BTPAGE_PAGETABLESTYLE_H
namespace BtPage
{
   /**
    * @brief This is a Data class for suppyling the PageTable with styling options when rendering.
    * no real data is being processed by this class.
    *
    */
   class PageTableStyle
   {
   private:
      /* I don't know why I have this private section.... it's probobly never gonna be used! */
   public:
      enum FillStyle
      {
         ALL,
         EveryOtherRow,
         EveryOtherColumn
      };

      struct Border
      {
         QColor color;
         int Thickness;
      };

      struct FillRectOptions
      {
         QColor color;
         QBrush brush;
         FillStyle fillStyleFlags;
      };

      Border *borderOptions;
      FillRectOptions *fillRectOptions;
   };
}
#endif /* BTPAGE_PAGETABLESTYLE_H */