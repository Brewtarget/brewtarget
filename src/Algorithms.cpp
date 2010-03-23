/*
 * Algorithms.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com) and Eric Tamme,  2009-2010.
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

// Called when Instance() is called, should only initialize once.
Algorithms::Algorithms()
{
	PlatoFromSG_20C20C_order = 3;
	PlatoFromSG_20C20C[0] = -616.868;
	PlatoFromSG_20C20C[1] = 1111.14;
	PlatoFromSG_20C20C[2] = -630.272;
	PlatoFromSG_20C20C[3] = 135.997;

	waterDensityPoly_C_order = 5;
	waterDensityPoly_C[0] = 0.9999776532;
	waterDensityPoly_C[1] = 6.557692037e-5;
	waterDensityPoly_C[2] = -1.007534371e-5;
	waterDensityPoly_C[3] = 1.372076106e-7;
	waterDensityPoly_C[4] = -1.414581892e-9;
	waterDensityPoly_C[5] = 5.6890971e-12;

	hydroCorrection15CPoly_order = 3;
	hydroCorrection15CPoly[0] = -0.911045;
	hydroCorrection15CPoly[1] = -16.2853e-3;
	hydroCorrection15CPoly[2] = 5.84346e-3;
	hydroCorrection15CPoly[3] = -15.3243e-6;
}


inline double Algorithms::intPow( double base, unsigned int pow )
{
   double ret = 1;
   for(; pow > 0; pow--)
      ret *= base;

   return ret;
}

// NOTE: order is the ORDER of the polynomial, NOT THE SIZE of the poly array.
double Algorithms::polyEval( double* poly, unsigned int order, double x )
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
double Algorithms::rootFind( double* poly, unsigned int order, double x0, double x1 )
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
double Algorithms::hydrometer15CCorrection( double celsius )
{
   return polyEval( hydroCorrection15CPoly,hydroCorrection15CPoly_order, celsius ) * (double)1e-3;
}

double Algorithms::SG_20C20C_toPlato( double sg )
{
   return polyEval(PlatoFromSG_20C20C, PlatoFromSG_20C20C_order, sg );
}

double Algorithms::PlatoToSG_20C20C( double plato )
{
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

double Algorithms::getWaterDensity_kgL( double celsius )
{
   return polyEval(waterDensityPoly_C, waterDensityPoly_C_order, celsius);
}

double Algorithms::getABVBySGPlato( double sg, double plato )
{
   // Implements the method found at:
   // http://www.byo.com/stories/projects-and-equipment/article/indices/29-equipment/1343-refractometers
   // ABV = [277.8851 - 277.4(SG) + 0.9956(Brix) + 0.00523(Brix2) + 0.000013(Brix3)] x (SG/0.79)

   return (277.8851 - 277.4*sg + 0.9956*plato + 0.00523*plato*plato + 0.000013*plato*plato*plato) * (sg/0.79);
}

double Algorithms::getABWBySGPlato( double sg, double plato )
{
   // Implements the method found at:
   // http://primetab.com/formulas.html

   double ri = refractiveIndex(plato);
   return 1017.5596 - 277.4*sg + ri*(937.8135*ri - 1805.1228);
}

double Algorithms::sgByStartingPlato( double startingPlato, double currentPlato )
{
   // Implements the method found at:
   // http://primetab.com/formulas.html

   double sp2 = startingPlato*startingPlato;
   double sp3 = sp2*startingPlato;

   double cp2 = currentPlato*currentPlato;
   double cp3 = cp2*currentPlato;

   return 1.001843 - 0.002318474*startingPlato - 0.000007775*sp2 - 0.000000034*sp3
          + 0.00574*currentPlato + 0.00003344*cp2 + 0.000000086*cp3;

}

double Algorithms::refractiveIndex( double plato )
{
   // Implements the method found at:
   // http://primetab.com/formulas.html
   return 1.33302 + 0.001427193*plato + 0.000005791157*plato*plato;
}

double Algorithms::realExtract( double sg, double plato )
{
   double ri = refractiveIndex(plato);
   return 194.5935 + 129.8*sg + ri*(410.8815*ri - 790.8732);
}
