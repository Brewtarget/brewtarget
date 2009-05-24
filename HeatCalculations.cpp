/*
 * HeatCalculations.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include "HeatCalculations.h"

double HeatCalculations::Cw_JKgK = 4184.0;
double HeatCalculations::Cw_calGC = 1.0;
double HeatCalculations::Cgrain_calGC = 0.4;
double HeatCalculations::rhoGrain_KgL = 0.432;

double HeatCalculations::equivalentMCProduct(double m1, double c1, double m2, double c2)
{
   return m1*c1*(1.+(m2*c2)/(m1*c1));
}