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
#ifndef _BTENUMFLAGS_H
#define _BTENUMFLAGS_H

namespace nBtPage
{
   /**
    * @brief These flags are used for placing objects on BtPage either relational to
    * other objects or to the page it self.
    * I use the bitshift method below as it more clearly shows the order of the flags and easier to see if one was missed. :)
    *
    * As these flags will be used with the | operator we can never have overlapping bits in their values
    * i.e. (1<<1) to give me an integer value of 2.
    * bitmasks and actual values are commented below on each value.
    */
   enum struct PlacingFlags
   {
      NOTUSED      = 0,
      /* These four are used with BtPage::placeRelationalto(..) and BtPage::placeRelationalToMM(..) functions */
      BELOW        = (1<<0), //00000000 00000000 00000000 00000001 (1)
      ABOVE        = (1<<1), //00000000 00000000 00000000 00000010 (2)
      RIGHTOF      = (1<<2), //00000000 00000000 00000000 00000100 (4)
      LEFTOF       = (1<<3), //00000000 00000000 00000000 00001000 (8)
      /* These six are used with BtPage::placeOnPage(...) function */
      LEFT         = (1<<4), //00000000 00000000 00000000 00010000 (16)
      RIGHT        = (1<<5), //00000000 00000000 00000000 00100000 (32)
      TOP          = (1<<6), //00000000 00000000 00000000 01000000 (64)
      BOTTOM       = (1<<7), //00000000 00000000 00000000 10000000 (128)
      VCENTER      = (1<<8), //00000000 00000000 00000001 00000000 (256)
      HCENTER      = (1<<9)  //00000000 00000000 00000010 00000000 (512)
   };

   inline PlacingFlags operator|(PlacingFlags a, PlacingFlags b)
   {
      return static_cast<PlacingFlags>(static_cast<int>(a) | static_cast<int>(b));
   }

   inline bool operator&(PlacingFlags a, PlacingFlags b)
   {
      return (static_cast<int>(a) & static_cast<int>(b));
   }
}


#endif /* _BTENUMFLAGS_H */