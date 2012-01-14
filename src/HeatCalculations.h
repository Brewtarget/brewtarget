/*
 * HeatCalculations.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _HEATCALCULATIONS_H
#define   _HEATCALCULATIONS_H

class HeatCalculations;

/*!
 * \class
 * \author Philip G. Lee
 *
 * \brief Algorithms and constants related to the thermodynamics of beer.
 */
class HeatCalculations
{
public:

   double equivalentMCProduct(double m1, double c1, double m2, double c2);
   // Water temp when mass 1 is initially at T1 and is to be brought to Tf by
   // water. MCw = (mass of water)*(water sp. heat). MC1 = (mass 1)*(sp. heat 1).
   double requiredWaterTemp( double MCw, double MC1, double Tf, double T1 );

   /***Specific heats***/
   // Water's specific heat.
   static double Cw_JKgK;
   static double Cw_calGC;
   static double Cgrain_calGC;

   /***Density***/
   static double rhoGrain_KgL;
   
   /***Absorption***/
   static double absorption_LKg;
};

#endif   /* _HEATCALCULATIONS_H */

