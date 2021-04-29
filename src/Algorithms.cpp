/*
 * Algorithms.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Eric Tamme <etamme@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include <cmath>
#include "Algorithms.h"
#include "PhysicalConstants.h"

namespace {
   // This is the cubic fit to get Plato from specific gravity, measured at 20C
   // relative to density of water at 20C.
   // P = -616.868 + 1111.14(SG) - 630.272(SG)^2 + 135.997(SG)^3
   Polynomial const platoFromSG_20C20C {
      Polynomial() << -616.868 << 1111.14 << -630.272 << 135.997
   };

   // Water density polynomial, given in kg/L as a function of degrees C.
   // 1.80544064e-8*x^3 - 6.268385468e-6*x^2 + 3.113930471e-5*x + 0.999924134
   Polynomial const waterDensityPoly_C {
      Polynomial() << 0.9999776532 << 6.557692037e-5 << -1.007534371e-5
         << 1.372076106e-7 << -1.414581892e-9 << 5.6890971e-12
   };

   // Polynomial in degrees Celsius that gives the additive hydrometer
   // correction for a 15C hydrometer when read at a temperature other
   // than 15C.
   Polynomial const hydroCorrection15CPoly {
      Polynomial() << -0.911045 << -16.2853e-3 << 5.84346e-3 << -15.3243e-6
   };


}

bool Algorithms::isNan(double d) {
   // If using IEEE floating points, all comparisons with a NaN
   // are false, so the following should be true only if we have
   // a NaN.
   return (d != d);
}

double Algorithms::round(double d)
{
   return floor(d+0.5);
}

double Algorithms::hydrometer15CCorrection( double celsius )
{
   return hydroCorrection15CPoly.eval(celsius) * 1e-3;
}

QColor Algorithms::srmToColor(double srm) {
   QColor ret;

   //==========My approximation from a photo and spreadsheet===========
   //double red = 232.9 * pow( (double)0.93, srm );
   //double green = (double)-106.25 * log(srm) + 280.9;
   //
   //int r = (int)Algorithms::round(red);
   //int g = (int)Algorithms::round(green);
   //int b = 0;

   // Philip Lee's approximation from a color swatch and curve fitting.
   int r = 0.5 + (272.098 - 5.80255*srm); if( r > 253.0 ) r = 253.0;
   int g = (srm > 35)? 0 : 0.5 + (2.41975e2 - 1.3314e1*srm + 1.881895e-1*srm*srm);
   int b = 0.5 + (179.3 - 28.7*srm);

   r = (r < 0) ? 0 : ((r > 255)? 255 : r);
   g = (g < 0) ? 0 : ((g > 255)? 255 : g);
   b = (b < 0) ? 0 : ((b > 255)? 255 : b);
   ret.setRgb( r, g, b );

   return ret;
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

double Algorithms::abvFromOgAndFg(double og, double fg) {
   // Assert the parameters were supplied in the right order by checking that FG cannot by higher than OG
   Q_ASSERT(og >= fg);

   //
   // From http://www.brewersfriend.com/2011/06/16/alcohol-by-volume-calculator-updated/:
   //    "[This] formula, and variations on it, comes from Ritchie Products Ltd, (Zymurgy, Summer 1995, vol. 18, no. 2)
   //    Michael L. Hall’s article Brew by the Numbers: Add Up What’s in Your Beer, and Designing Great Beers by
   //    Daniels.
   //    ...
   //    The relationship between the change in gravity, and the change in ABV is not linear. All these equations are
   //    approximations."
   //
   return (76.08 * (og - fg) / (1.775 - og)) * (fg / 0.794);

}
