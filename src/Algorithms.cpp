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
double* hydroCorrection15CPoly = 0;
unsigned int hydroCorrection15CPoly_order = 0;

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
      waterDensityPoly_C = new double[6];
      waterDensityPoly_C_order = 5;
      waterDensityPoly_C[0] = 0.9999776532;
      waterDensityPoly_C[1] = 6.557692037e-5;
      waterDensityPoly_C[2] = -1.007534371e-5;
      waterDensityPoly_C[3] = 1.372076106e-7;
      waterDensityPoly_C[4] = -1.414581892e-9;
      waterDensityPoly_C[5] = 5.6890971e-12;
   }

   if( hydroCorrection15CPoly == 0 )
   {
      hydroCorrection15CPoly = new double[4];
      hydroCorrection15CPoly_order = 3;
      hydroCorrection15CPoly[0] = -0.911045;
      hydroCorrection15CPoly[1] = -16.2853e-3;
      hydroCorrection15CPoly[2] = 5.84346e-3;
      hydroCorrection15CPoly[3] = -15.3243e-6;
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

// Returns the additive correction (in SG 15C units).
double hydrometer15CCorrection( double celsius )
{
   initVars();
   return polyEval( hydroCorrection15CPoly,hydroCorrection15CPoly_order, celsius ) * (double)1e-3;
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
