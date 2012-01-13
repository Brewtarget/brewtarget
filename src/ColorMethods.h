/*
 * ColorMethods.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _COLORMETHODS_H
#define _COLORMETHODS_H

class ColorMethods;

/*!
 * \class ColorMethods
 * \author Philip G. Lee
 *
 * Converts malt color units to SRM.
 */
class ColorMethods
{
public:
   ColorMethods();
   ~ColorMethods();

   //! Depending on selected algorithm, convert malt color units to SRM.
   static double mcuToSrm(double mcu);
private:
   static double morey(double mcu);
   static double daniel(double mcu);
   static double mosher(double mcu);
};

#endif
