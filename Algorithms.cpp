/*
 * Algorithms.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <cmath>
#include <math.h>
#include "Algorithms.h"

double* PlatoFromSG_20C20C = 0;
unsigned int PlatoFromSG_20C20C_order;
double* waterDensityPoly_C = 0;
unsigned int waterDensityPoly_C_order = 0;

void initVars()
{
   if( PlatoFromSG_20C20C == 0 )
   {
      PlatoFromSG_20C20C = new double[4];
      PlatoFromSG_20C20C_order = 3;
      PlatoFromSG_20C20C[0] = -616.868;
      PlatoFromSG_20C20C[1] = 1111.14;
      PlatoFromSG_20C20C[2] = -630.272;
      PlatoFromSG_20C20C[3] = 135.997;
   }

   if( waterDensityPoly_C == 0 )
   {
      waterDensityPoly_C = new double[4];
      waterDensityPoly_C_order = 3;
      waterDensityPoly_C[0] = 0.999924134;
      waterDensityPoly_C[1] = 3.113930471e-5;
      waterDensityPoly_C[2] = 6.268385468e-6;
      waterDensityPoly_C[3] = 1.80544064e-8;
   }
}

inline double intPow( double base, unsigned int pow )
{
   double ret = 1;
   for(; pow > 0; pow--)
      ret *= base;

   return ret;
}

// NOTE: order is the ORDER of the polynomial, NOT THE SIZE of the poly array.
double polyEval( double* poly, unsigned int order, double x )
{
   double ret = 0.0;

   for( ; order > 0; --order )
   {
      ret += poly[order] * intPow( x, order );
   }
   ret += poly[0];

   return ret;
}

// Root finding of a polynomial via the secant method. Returns HUGE_VAL on failure.
double rootFind( double* poly, unsigned int order, double x0, double x1 )
{
   double guesses[] = { x0, x1 };
   double newGuess;
   double maxAllowableSeparation = fabs( x0 - x1 ) * 1e3;

   while( fabs( guesses[0] - guesses[1] ) > ROOT_PRECISION )
   {
      newGuess = guesses[1] - (guesses[1] - guesses[0]) * polyEval( poly, order, guesses[1]) / ( polyEval( poly, order, guesses[1]) - polyEval( poly, order, guesses[0]) );

      guesses[0] = guesses[1];
      guesses[1] = newGuess;

      if( fabs( guesses[0] - guesses[1] ) > maxAllowableSeparation )
         return HUGE_VAL;
   }

   return newGuess;
}

double SG_20C20C_toPlato( double sg )
{
   initVars();
   return polyEval(PlatoFromSG_20C20C, PlatoFromSG_20C20C_order, sg );
}

double PlatoToSG_20C20C( double plato )
{
   initVars();
   double poly[PlatoFromSG_20C20C_order+1];

   // Copy the polynomial, cuz we need to alter it.
   for( int i = 0; i <= PlatoFromSG_20C20C_order; ++i )
   {
      poly[i] = PlatoFromSG_20C20C[i];
   }

   // After this, finding the root of the polynomial will be finding the SG.
   poly[0] -= plato;

   return rootFind( poly, PlatoFromSG_20C20C_order, 1.000, 1.050 );
}

double getWaterDensity_kgL( double celsius )
{
   initVars();
   return polyEval(waterDensityPoly_C, waterDensityPoly_C_order, celsius);
}