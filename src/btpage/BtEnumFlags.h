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
   enum struct PlacingFlags
   {
      BELOW = 1,
      ABOVE = 2,
      RIGHTOF = 4,
      LEFTOF = 8,
      LEFT = 16,
      RIGHT = 32,
      TOP = 64,
      BOTTOM = 128,
      VCENTER = 256,
      HCENTER = 512
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