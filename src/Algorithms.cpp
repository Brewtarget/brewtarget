/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * Algorithms.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Eric Tamme <etamme@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "Algorithms.h"

#include <algorithm> // Of course we stand on the shoulders of the standard library, rather than reinvent the wheel
#include <cmath>

#include <QDebug>
#include <QVector>

#include "PhysicalConstants.h"
#include "measurement/SucroseConversion.h"
#include "measurement/Unit.h"

namespace {

   double constexpr ROOT_PRECISION = 0.0000001;

   double constexpr minPlausibleSpecificGravity = 0.900;
   double constexpr maxPlausibleSpecificGravity = 1.150;

   /**
    * \brief returns base^pow for the special case when pow is a positive integer
    *        (The more general case is already covered by pow() in the standard library.)
    */
   double intPow(double base, unsigned int pow) {
      double ret = 1;
      for(; pow > 0; pow--) {
         ret *= base;
      }
      return ret;
   }

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

   /**
    * \brief Convert specific gravity to excess gravity.
    *
    *        See comment in \c Algorithms::abvFromOgAndFg for the difference.
    */
   double specificGravityToExcessGravity(double sg) {
      return (sg - 1.0) * 1000;
   }

   /**
    * This struct and \c gravityDifferenceFactors are used for the ABV calculation in \c Algorithms::abvFromOgAndFg
    * It's a straight lift of the table at
    * https://www.gov.uk/government/publications/excise-notice-226-beer-duty/excise-notice-226-beer-duty--2#calculation-strength
    * except that we've multiplied the OG differences by 10 so we can represent them as integers
    */
   struct AbvFactorForGravityDifference {
      int excessGravityDiffx10_Min;
      int excessGravityDiffx10_Max;
      double pctAbv_Min;
      double pctAbv_Max;
      double factorToUse;
   };
   QVector<AbvFactorForGravityDifference> const gravityDifferenceFactors {
      { 00,   69,  0.0,  0.8, 0.125},
      { 70,  104,  0.8,  1.3, 0.126},
      {105,  172,  1.3,  2.1, 0.127},
      {173,  261,  2.2,  3.3, 0.128},
      {262,  360,  3.3,  4.6, 0.129},
      {361,  465,  4.6,  6.0, 0.130},
      {466,  571,  6.0,  7.5, 0.131},
      {572,  679,  7.5,  9.0, 0.132},
      {680,  788,  9.0, 10.5, 0.133},
      {789,  897, 10.5, 12.0, 0.134},
      {898, 1007, 12.0, 13.6, 0.135}
   };

   /**
    * \brief Extension of std::lower_bound to find an interpolated conversion in a sorted range
    *
    * \param first As \c std::lower_bound, first \c T in the sorted range
    * \param last  Different from \c std::lower_bound, last \c T in the sorted range
    * \param value As \c std::lower_bound, a \c T struct containing the value to convert
    * \param getFrom Lambda to extract the "from" value from a struct of the type \c T
    * \param getTo Lambda to extract the "to" value from a struct of the type \c T
    * \param whatFrom Description for logging of what we're converting from
    * \param whatTo   Description for logging of what we're converting from
    */
   template<class ForwardIt, class T, class GetFrom, class GetTo>
   double interpolatedConversion(ForwardIt first,
                                    ForwardIt last,
                                    T const & value,
                                    GetFrom getFrom,
                                    GetTo getTo,
                                    char const * const whatFrom,
                                    char const * const whatTo) {
      auto const firstLarger = std::lower_bound(
         first,
         last + 1,
         value,
         [& getFrom, & getTo](T const & lhs, T const & rhs) {return getFrom(lhs) < getFrom(rhs);}
      );

      if (firstLarger == last + 1) {
         // We're off the end of the array
         qWarning() <<
            Q_FUNC_INFO << whatFrom << getFrom(value) << "too large to convert to " << whatTo <<
            " so using max value of" << getTo(*last);
         return getTo(*last);
      }

      // The lower bound is the first element that does not satisfy "element < value" (where value is what we're searching
      // for.
      Q_ASSERT(getFrom(*firstLarger) >= getFrom(value));

      // If we found an exact match, then return that
      if (getFrom(*firstLarger) == getFrom(value)) {
         return getTo(*firstLarger);
      }

      if (firstLarger == first) {
         qWarning() <<
            Q_FUNC_INFO << whatFrom << getFrom(value) << "too small to convert to " << whatTo <<
            " so using min value of" << getTo(*first);
         return getTo(*first);
      }

      // Since firstLarger is the first element not to satisfy element < value, its predecessor must, by definition,
      // satisfy this
      auto const lastSmaller = firstLarger - 1;
      Q_ASSERT(getFrom(*lastSmaller) < getFrom(value));

      // Now we just do a linear interpolation
      // positionInRange will be between 0 and 1 and tells us, in relative terms, where the supplied SG is in relation to
      // lastSmaller and firstLarger.  Eg 0.5 would mean it was exactly half-way between the two.
      Q_ASSERT(getFrom(*lastSmaller) < getFrom(*firstLarger));

      double const positionInRange =
         (getFrom(value) - getFrom(*lastSmaller)) / (getFrom(*firstLarger) - getFrom(*lastSmaller));
      qDebug() <<
         Q_FUNC_INFO << "Supplied value" << getFrom(value) << whatFrom << " lies" << (100 * positionInRange) << "% "
         "between" << getFrom(*lastSmaller) << whatFrom << "(=" << getTo(*lastSmaller) << whatTo << ") and" <<
         getFrom(*firstLarger) << whatFrom << "(=" << getTo(*firstLarger) << whatTo << ")";

      Q_ASSERT(positionInRange >= 0.0);
      Q_ASSERT(positionInRange <= 1.0);

      return positionInRange * (getTo(*firstLarger) - getTo(*lastSmaller)) + getTo(*lastSmaller);
   }

}

Polynomial::Polynomial() :
   m_coeffs() {
   return;
}

Polynomial::Polynomial(Polynomial const & other) :
   m_coeffs(other.m_coeffs) {
   return;
}

Polynomial::Polynomial(size_t order) :
   m_coeffs(order + 1, 0.0) {
   return;
}

Polynomial::Polynomial(double const * coeffs, size_t order) :
   m_coeffs(coeffs, coeffs + order + 1) {
   return;
}

Polynomial & Polynomial::operator<<(double coeff) {
   m_coeffs.push_back(coeff);
   return *this;
}

size_t Polynomial::order() const {
   return m_coeffs.size()-1;
}

double Polynomial::operator[](size_t n) const {
   Q_ASSERT( n <= m_coeffs.size() );
   return m_coeffs[n];
}

double & Polynomial::operator[] (size_t n) {
   Q_ASSERT( n < m_coeffs.size() );
   return m_coeffs[n];
}

double Polynomial::eval(double x) const {
   double ret = 0.0;

   for(size_t i = order(); i > 0; --i) {
      ret += m_coeffs[i] * intPow( x, i );
   }
   ret += m_coeffs[0];

   return ret;
}

double Polynomial::rootFind( double x0, double x1 ) const {
   double guesses[] = { x0, x1 };
   double newGuess = x0;
   double maxAllowableSeparation = qAbs( x0 - x1 ) * 1e3;

   while( qAbs( guesses[0] - guesses[1] ) > ROOT_PRECISION ) {
      newGuess = guesses[1] - (guesses[1] - guesses[0]) * eval(guesses[1]) / ( eval(guesses[1]) - eval(guesses[0]) );

      guesses[0] = guesses[1];
      guesses[1] = newGuess;

      if( qAbs( guesses[0] - guesses[1] ) > maxAllowableSeparation ) {
         return HUGE_VAL;
      }
   }

   return newGuess;
}

//======================================================================================================================

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

double Algorithms::SG_20C20C_toPlato(double sg) {
   return platoFromSG_20C20C.eval(sg);
}

double Algorithms::PlatoToSG_20C20C(double plato) {
   // Copy the polynomial, cuz we need to alter it.
   Polynomial poly(platoFromSG_20C20C);

   // After this, finding the root of the polynomial will be finding the SG.
   poly[0] -= plato;

   return poly.rootFind(minPlausibleSpecificGravity, maxPlausibleSpecificGravity);
}

double Algorithms::SgAt20CToBrix(double sg) {
   // Since Brix is "the sugar content of an aqueous solution", there isn't really a meaningful conversion for SG below
   // 1.000, so we just always return 0 brix in this case.
   if (sg <= 1.0) {
      qWarning() << Q_FUNC_INFO << "Specific gravity" << sg << "does not have a meaningful conversion to Brix";
      return 0.0;
   }

   //
   // A lot of people use the Wikipedia conversion formula from specific gravity to Brix (well, the more accurate of the
   // two offered on https://en.wikipedia.org/wiki/Brix.  See eg
   // https://beermaverick.com/brix-plato-specific-gravity-converter/).
   //
   // Note that Wikipedia says this formula "should not be used above S = 1.17874 (40 °Bx)".
   //
   // You can simplify the calculation by using Horner's method (https://en.wikipedia.org/wiki/Horner%27s_method) to
   // evaluate the formula (as offered at
   // https://www.vcalc.com/wiki/MichaelBartmess/Degrees+Brix%2C+Bx%2C+to+SG and in various other places) to give code
   // as follows:
   //
   //    if (sg <= 1.17874) {
   //       return ((182.4601*sg - 775.6821)*sg + 1262.7794)*sg - 669.5622;
   //    }
   //
   // There is a different formula (brix = 143.254 * sg^3 - 648.670 * sg^2 + 1125.805 * sg - 620.389) at
   // https://www.vinolab.hr/calculator/gravity-density-sugar-conversions-en19, that is "based on an expression from a
   // polynomial fit to a large data set", but it doesn't say what data set was used.  I don't know whether that's more
   // or less accurate than the Wikipedia formula.
   //
   // In either case, such formulae are a "best fit curve" to observed data.  Since we have 800 points of observed data,
   // we can do something more accurate.  We search that data and either find an exact match or we find the two nearest
   // values above and below the one we are looking for, and we then do a linear interpolation on those.  Effectively,
   // we're drawing straight lines between all the observed data points.  For the number of points we have, I think it's
   // a good approximation.
   //


   //
   // The zeros in searchingFor are dummy values.  We need the struct for std::lower_bound below
   //
   // The advantage of using std::lower_bound over std::find_if is that, provided you give it random-access iterators,
   // the former does O(log N) binary search rather than O(N) linear search.
   //
   Measurement::SucroseConversion const searchingFor{0, 0, sg};
   return interpolatedConversion(
      &Measurement::sucroseConversions[0],
      &Measurement::sucroseConversions[Measurement::sucroseConversions_size - 1],
      searchingFor,
      [](Measurement::SucroseConversion const & value) {return value.apparentSgAt2020C; },
      [](Measurement::SucroseConversion const & value) {return value.degreesBrix; },
      "Specific gravity",
      "Brix"
   );

}

double Algorithms::BrixToSgAt20C(double brix) {
   //
   // Converting Brix to Specific Gravity is "just" the inverse of SgAt20CToBrix()
   //
   // If we were taking the formulaic approach, we might think of algebraically finding the inverse of the "best fit"
   // cubic function.  However, doing this directly would give something horrifically unwieldy.  Eg, if you ask
   // Wolfram Alpha (https://www.wolframalpha.com/) for the inverse function of
   //    y = ((182.4601*x - 775.6821)*x + 1262.7794)*x - 669.5622
   // it gives you:
   //    y = 1.41708 - 2.63482×10^-7 (
   //           4.46934×10^6 sqrt(1.12359×10^21 x^2 - 1.8305×10^23 x + 1.14483×10^25)
   //           - 1.49813×10^17 x + 1.22034×10^19
   //        )^(1/3) + (1.13417×10^6) / (
   //          4.46934×10^6 sqrt(1.12359×10^21 x^2 - 1.8305×10^23 x + 1.14483×10^25) - 1.49813×10^17 x + 1.22034×10^19
   //        )^(1/3)
   // as the simpler of two answers!
   //
   // Apparently, according to BYO Magazine, there is also SG = (Brix / (258.6-((Brix / 258.2)*227.1))) + 1, but I've
   // only found indirect reference to that at https://www.brewersfriend.com/brix-converter/
   // The same formula is offered at https://brucrafter.com/convert-brix-to-sg/
   //
   // We could use an approximate method to find the roots of the "best fit" cubic function, as is done in
   // Algorithms::ogFgToPlato.  Code would be:
   //
   //    Polynomial sgToBrixFormula {
   //       Polynomial() << -669.5622 << 1262.7794 << -775.6821 << 182.4601
   //    };
   //    sgToBrixFormula[0] -= brix;
   //    return sgToBrixFormula.rootFind(minPlausibleSpecificGravity, maxPlausibleSpecificGravity);
   //
   // However, instead, we use the same approach as in SgAt20CToBrix of interpolating the USDA observed data.
   //
   Measurement::SucroseConversion const searchingFor{0, brix, 0};

   return interpolatedConversion(
      &Measurement::sucroseConversions[0],
      &Measurement::sucroseConversions[Measurement::sucroseConversions_size - 1],
      searchingFor,
      [](Measurement::SucroseConversion const & value) {return value.degreesBrix; },
      [](Measurement::SucroseConversion const & value) {return value.apparentSgAt2020C; },
      "Brix",
      "Specific gravity"
   );


}

double Algorithms::getPlato(double sugar_kg, double wort_l) {
   double const water_kg = wort_l - sugar_kg/PhysicalConstants::sucroseDensity_kgL; // Assumes sucrose vol and water vol add to wort vol.
   double const totalMass_kg = sugar_kg + water_kg;

   // It's not hugely meaningful to call this function with zero values for sugar and wort.  In those circumstances,
   // rather than return not-a-number (because of dividing by zero), we return the °Plato value of water: 0.0.
   if (0.0 == totalMass_kg) {
      return 0.0;
   }

   return sugar_kg/totalMass_kg * 100.0;
}

double Algorithms::getWaterDensity_kgL(double celsius) {
   return waterDensityPoly_C.eval(celsius);
}

double Algorithms::getABVBySGPlato(double sg, double plato) {
   // Implements the method found at:
   // http://www.byo.com/stories/projects-and-equipment/article/indices/29-equipment/1343-refractometers
   // ABV = [277.8851 - 277.4(SG) + 0.9956(Brix) + 0.00523(Brix2) + 0.000013(Brix3)] x (SG/0.79)

   return (277.8851 - 277.4*sg + 0.9956*plato + 0.00523*plato*plato + 0.000013*plato*plato*plato) * (sg/0.79);
}

double Algorithms::getABWBySGPlato(double sg, double plato) {
   // Implements the method found at:
   // http://primetab.com/formulas.html

   double ri = refractiveIndex(plato);
   return 1017.5596 - 277.4*sg + ri*(937.8135*ri - 1805.1228);
}

double Algorithms::sgByStartingPlato(double startingPlato, double currentPlato) {
   // Implements the method found at:
   // http://primetab.com/formulas.html

   double sp2 = startingPlato*startingPlato;
   double sp3 = sp2*startingPlato;

   double cp2 = currentPlato*currentPlato;
   double cp3 = cp2*currentPlato;

   return 1.001843 - 0.002318474*startingPlato - 0.000007775*sp2 - 0.000000034*sp3
          + 0.00574*currentPlato + 0.00003344*cp2 + 0.000000086*cp3;

}

double Algorithms::ogFgToPlato(double og, double fg) {
   double sp = SG_20C20C_toPlato( og );

   Polynomial poly(
      Polynomial()
         << 1.001843 - 0.002318474*sp - 0.000007775*sp*sp - 0.000000034*sp*sp*sp - fg
         << 0.00574 << 0.00003344 << 0.000000086
   );

   return poly.rootFind(3, 5);
}

double Algorithms::refractiveIndex(double plato) {
   // Implements the method found at:
   // http://primetab.com/formulas.html
   return 1.33302 + 0.001427193*plato + 0.000005791157*plato*plato;
}

double Algorithms::realExtract(double sg, double plato) {
   double ri = refractiveIndex(plato);
   return 194.5935 + 129.8*sg + ri*(410.8815*ri - 790.8732);
}

double Algorithms::abvFromOgAndFg(double og, double fg) {
   // Assert the parameters were supplied in the right order by checking that FG cannot by higher than OG
   Q_ASSERT(og >= fg);

   //
   // Previously, in different places in the code, we either used a very rough rule of thumb:
   //
   //    double calculatedABV_pct = (og - fg) * 130
   //
   // or we used the FALLBACK METHOD described below.
   //
   // The current calculation method we use comes from the UK Laboratory of the Government Chemist.  It is what HM
   // Revenue and Customs (HMRC) encourage UK microbreweries to use to calculate ABV if they have "no or minimal
   // laboratory facilities" and is described here:
   // https://www.gov.uk/government/publications/excise-notice-226-beer-duty/excise-notice-226-beer-duty--2#calculation-strength.
   // (Larger breweries in the UK are expected to use distillation analysis per
   // https://www.gov.uk/government/publications/excise-notice-226-beer-duty/excise-notice-226-beer-duty--2#distillation-analysis
   // or any method producing the same results.)
   //
   // AIUI this method is more accurate than the simpler formulas more traditionally proposed to homebrewers.  That
   // said, it is not intended to give results accurate to more than one decimal place.  HMRC say "For duty purposes ...
   // the percentage of alcohol by volume (ABV) in the beer ... should be expressed to one decimal place, for example,
   // 4.19% ABV becomes 4.1% ABV. Ignore figures after the first decimal place."  (See
   // https://www.gov.uk/government/publications/excise-notice-226-beer-duty/excise-notice-226-beer-duty--2#alcohol-strength)
   //

   //
   // It's worth reiterating some definitions here.  Although OG and FG are often expressed in terms of SPECIFIC GRAVITY
   // (see https://en.wikipedia.org/wiki/Relative_density), the definition HMRC will almost certainly be using is in
   // terms of EXCESS GRAVITY.  Per https://beerandbrewing.com/dictionary/c9EBwhgZpA/: "Original gravity is expressed as
   // the density above that of distilled water and in the UK is called the excess gravity. Water is deemed to have a
   // density at STP of 1.000.  If the wort density is 1.048, it will have 48° of excess gravity and an OG of 48.
   //    "Internationally, different units are used to express OG that are unique to the brewing industry and include
   // degrees Plato, degrees Balling, or percent dry matter of the wort, Brix % (for sucrose only). ... The numerical
   // figure for these units approximates one-quarter of the excess gravity. In the example above 48/4 = 12% dry matter
   // by weight or 12° Balling or 12° Plato."
   //
   // First convert our OG and FG from specific gravity to excess gravity, then take the the difference and round it to
   // one decimal place.  Except do everything ×10 because it makes the subsequent look-up easier.
   //
   int excessGravityDiffx10 = round(10.0 * (specificGravityToExcessGravity(og) - specificGravityToExcessGravity(fg)));
   double excessGravityDiff = excessGravityDiffx10 / 10.0;
   qDebug() <<
      Q_FUNC_INFO << "OG (as SG) =" << og << ", FG (as SG) =" << fg << ", excess gravity diff =" << excessGravityDiff <<
      "(×10 =" << excessGravityDiffx10 << ")";

   //
   // Working to one decimal place and multiplying by 10 means we're working with integers for the excess gravity
   // difference, which makes everything simple for this lookup, and means we don't have to think about floating point
   // rounding errors.
   //
   auto matchingGravityDifferenceRec = std::find_if(
      gravityDifferenceFactors.cbegin(),
      gravityDifferenceFactors.cend(),
      [excessGravityDiffx10](AbvFactorForGravityDifference const & rec) {
         return (rec.excessGravityDiffx10_Min <= excessGravityDiffx10 &&
                 excessGravityDiffx10 <= rec.excessGravityDiffx10_Max);
      }
   );

   //
   // FALLBACK METHOD
   //
   // From http://www.brewersfriend.com/2011/06/16/alcohol-by-volume-calculator-updated/:
   //    "[This] formula, and variations on it, comes from Ritchie Products Ltd, (Zymurgy, Summer 1995, vol. 18, no. 2)
   //    Michael L. Hall’s article Brew by the Numbers: Add Up What’s in Your Beer, and Designing Great Beers by
   //    Daniels.
   //    ...
   //    The relationship between the change in gravity, and the change in ABV is not linear. All these equations are
   //    approximations."
   //
   double const abvByFallbackMethod = (76.08 * (og - fg) / (1.775 - og)) * (fg / 0.794);

   if (matchingGravityDifferenceRec == gravityDifferenceFactors.cend()) {
      qCritical() <<
         Q_FUNC_INFO << "Could not find gravity difference record for difference of " <<
         (excessGravityDiffx10 / 10.0) << "so using fallback method";
      return abvByFallbackMethod;
   }

   double const abvByHmrcMethod = excessGravityDiff * matchingGravityDifferenceRec->factorToUse;

   qDebug() <<
      Q_FUNC_INFO << "ABV old method:" << abvByFallbackMethod << "% , new method:" << abvByHmrcMethod << "% (used factor" <<
      matchingGravityDifferenceRec->factorToUse << "and should be in range" <<
      matchingGravityDifferenceRec->pctAbv_Min << "% -" << matchingGravityDifferenceRec->pctAbv_Max << "%)";

   // The tables from UK HMRC have some sanity-check data, so let's use it!
   if (abvByHmrcMethod < matchingGravityDifferenceRec->pctAbv_Min ||
       abvByHmrcMethod > matchingGravityDifferenceRec->pctAbv_Max) {
      qWarning() <<
         Q_FUNC_INFO << "Calculated ABV of" << abvByHmrcMethod << "% is outside expected range (" <<
         matchingGravityDifferenceRec->pctAbv_Min << "% -" << matchingGravityDifferenceRec->pctAbv_Max << "%)";
   }

   return abvByHmrcMethod;
}

double Algorithms::correctSgForTemperature(double measuredSg, double readingTempInC, double calibrationTempInC) {
   //
   // Typically older hydrometers are calibrated to 15°C and newer ones to 20°C
   //
   // From https://www.vinolab.hr/calculator/hydrometer-temperature-correction-en31,
   // http://www.straighttothepint.com/hydrometer-temperature-correction/ and
   // https://homebrew.stackexchange.com/questions/4137/temperature-correction-for-specific-gravity we have the
   // following formula for temperatures in Fahrenheit:
   //
   //   corrected-reading = measured-reading * (
   //     (1.00130346 - (0.000134722124 * tr) + (0.00000204052596 * tr^2) - (0.00000000232820948 * tr^3)) /
   //     (1.00130346 - (0.000134722124 * tc) + (0.00000204052596 * tc^2) - (0.00000000232820948 * tc^3))
   //   )
   // Where:
   //    tr = temperature at time of reading
   //    tc = calibration temperature of hydrometer
   //
   // All these sorts of formulae are derived from fitting a polynomial to observed results.  (See
   // https://onlinelibrary.wiley.com/doi/pdf/10.1002/j.2050-0416.1970.tb03327.x for a rather old example.)  Hence the
   // use of non-SI units -- because the people in question were working in Fahrenheit.
   //
   double tr = Measurement::Units::fahrenheit.fromCanonical(readingTempInC);
   double tc = Measurement::Units::fahrenheit.fromCanonical(calibrationTempInC);

   double correctedSg = measuredSg * (
      (1.00130346 - 0.000134722124 * tr + 0.00000204052596 * intPow(tr,2) - 0.00000000232820948 * intPow(tr,3)) /
      (1.00130346 - 0.000134722124 * tc + 0.00000204052596 * intPow(tc,2) - 0.00000000232820948 * intPow(tc,3))
   );

   qDebug() <<
     Q_FUNC_INFO << measuredSg << "SG measured @" << readingTempInC << "°C (" << tr << "°F) "
     "on hydrometer calibrated at" << calibrationTempInC << "°C (" << tc << "°F) is corrected to" << correctedSg <<
     "SG";

   return correctedSg;

}
