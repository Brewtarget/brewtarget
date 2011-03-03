/*
 * IbuMethods.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#ifndef _IBUMETHODS_H
#define _IBUMETHODS_H

class IbuMethods
{
public:
   IbuMethods();
   ~IbuMethods();

   // AArating in [0,1], wort_grav in specific gravity at around 60F I guess.
   static double getIbus(double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes);
private:
   static double tinseth(double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes);
   static double rager(double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes);
};

#endif
