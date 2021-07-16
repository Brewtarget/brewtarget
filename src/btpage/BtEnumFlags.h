/*
 * BtEnumFlags.h is part of Brewtarget, and is Copyright the following
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
#ifndef BTENUMFLAGS_H
#define BTENUMFLAGS_H

namespace BtPage
{
   /**
    * @brief These flags are used for placing objects on Page relational to
    * other objects.
    * I use the bitshift method below as it more clearly shows the order of the flags and easier to see if one was missed. :)
    *
    * As these flags will be used with the | operator we can never have overlapping bits in their values
    * i.e. (1<<1) to give me an integer value of 2.
    * bitmasks and actual values are commented below on each value.
    */
   enum RelationalPlacingFlags
   {
      /* These four are used with Page::placeRelationalto(..) and Page::placeRelationalToMM(..) functions */
      BELOW        = 1, // (1<<0) = 1! 00000000 00000000 00000000 00000001 (1)
      ABOVE        = (1<<1),         //00000000 00000000 00000000 00000010 (2)
      RIGHTOF      = (1<<2),         //00000000 00000000 00000000 00000100 (4)
      LEFTOF       = (1<<3),         //00000000 00000000 00000000 00001000 (8)
   };

   inline RelationalPlacingFlags operator|(RelationalPlacingFlags a, RelationalPlacingFlags b)
   {
      return static_cast<RelationalPlacingFlags>(static_cast<int>(a) | static_cast<int>(b));
   }

   inline bool operator&(RelationalPlacingFlags a, RelationalPlacingFlags b)
   {
      return (static_cast<int>(a) & static_cast<int>(b));
   }

   /**
    * @brief These flags are used for placing objects on Page, these are the fixed positions to the page it self.
    * I use the bitshift method below as it more clearly shows the order of the flags and easier to see if one was missed. :)
    *
    * As these flags will be used with the | operator we can never have overlapping bits in their values
    * i.e. (1<<1) to give me an integer value of 2.
    * bitmasks and actual values are commented below on each value.
    */
   enum FixedPlacingFlags
   {
      CUSTOM       = 0,
      /* These six are used with Page::placeOnPage(...) function */
      LEFT         = 1,      // (1<<0) = 1, 00000000 00000000 00000000 00010000 (16)
      RIGHT        = (1<<1),              //00000000 00000000 00000000 00100000 (32)
      TOP          = (1<<2),              //00000000 00000000 00000000 01000000 (64)
      BOTTOM       = (1<<3),              //00000000 00000000 00000000 10000000 (128)
      VCENTER      = (1<<4),              //00000000 00000000 00000001 00000000 (256)
      HCENTER      = (1<<5)               //00000000 00000000 00000010 00000000 (512)
   };

   inline FixedPlacingFlags operator|(FixedPlacingFlags a, FixedPlacingFlags b)
   {
      return static_cast<FixedPlacingFlags>(static_cast<int>(a) | static_cast<int>(b));
   }

   inline bool operator&(FixedPlacingFlags a, FixedPlacingFlags b)
   {
      return (static_cast<int>(a) & static_cast<int>(b));
   }
}


#endif /* BTENUMFLAGS_H */