/*
 * Algorithms.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com) and Eric Tamme,  2009-2013.
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
#include "PhysicalConstants.h"

Polynomial Algorithms::platoFromSG_20C20C(
   Polynomial() << -616.868 << 1111.14 << -630.272 << 135.997
);
Polynomial Algorithms::waterDensityPoly_C(
   Polynomial() << 0.9999776532 << 6.557692037e-5 << -1.007534371e-5
      << 1.372076106e-7 << -1.414581892e-9 << 5.6890971e-12
);
Polynomial Algorithms::hydroCorrection15CPoly(
   Polynomial() << -0.911045 << -16.2853e-3 << 5.84346e-3 << -15.3243e-6
);

double Algorithms::round(double d)
{
   return floor(d+0.5);
}

double Algorithms::hydrometer15CCorrection( double celsius )
{
   return hydroCorrection15CPoly.eval(celsius) * 1e-3;
}

double Algorithms::SG_20C20C_toPlato( double sg )
{
   return platoFromSG_20C20C.eval(sg);
}

double Algorithms::PlatoToSG_20C20C( double plato )
{
   // Copy the polynomial, cuz we need to alter it.
   Polynomial poly(platoFromSG_20C20C);

   // After this, finding the root of the polynomial will be finding the SG.
   poly[0] -= plato;

   return poly.rootFind( 1.000, 1.050 );
}

double Algorithms::getPlato( double sugar_kg, double wort_l )
{
   double water_kg = wort_l - sugar_kg/PhysicalConstants::sucroseDensity_kgL; // Assumes sucrose vol and water vol add to wort vol.

   return sugar_kg/(sugar_kg+water_kg) * 100.0;
}

double Algorithms::getWaterDensity_kgL( double celsius )
{
   return waterDensityPoly_C.eval(celsius);
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

double Algorithms::ogFgToPlato( double og, double fg )
{
   double sp = SG_20C20C_toPlato( og );

   Polynomial poly(
      Polynomial()
         << 1.001843 - 0.002318474*sp - 0.000007775*sp*sp - 0.000000034*sp*sp*sp - fg
         << 0.00574 << 0.00003344 << 0.000000086
   );

   return poly.rootFind(3, 5);
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
