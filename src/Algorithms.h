/*
 * Algorithms.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Eric Tamme <etamme@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne (malavv) <duguigne@gmail.com>
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
#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#pragma once

#define ROOT_PRECISION 0.0000001

#include <cmath>
#include <limits> // For std::numeric_limits
#include <string.h>
#include <vector>

#include <QColor>
#include <QList>

/*!
 * \brief Class to encapsulate real polynomials in a single variable
 *
 * .:TBD:. At somme point consider replacing this with
 * https://www.boost.org/doc/libs/1_76_0/libs/math/doc/html/math_toolkit/polynomials.html
 */
class Polynomial
{
public:
   //! \brief Default constructor
   Polynomial();

   //! \brief Copy constructor
   Polynomial( Polynomial const& other );

   //! \brief Constructs the 0 polynomial with given \c order
   Polynomial( size_t order );

   //! \brief Constructor from an array of coefficients
   Polynomial(double const* coeffs, size_t order);

   //! \brief Add a coefficient for x^(\c order() + 1)
   Polynomial& operator<<(double coeff);

   //! \brief Get the polynomial's order (highest exponent)
   size_t order() const;

   //! \brief Get coefficient of x^n where \c n <= \c order()
   double operator[](size_t n) const;

   //! \brief Get coefficient of x^n where \c n <= \c order() (non-const)
   double & operator[](size_t n);


   //! \brief Evaluate the polynomial at point \c x
   double eval(double x) const;

   /*!
    * \brief Root-finding by the secant method.
    *
    * \param x0 - one of two initial \b distinct guesses at the root
    * \param x1 - one of two initial \b distinct guesses at the root
    * \returns \c HUGE_VAL on failure, otherwise a root of the polynomial
    */
   double rootFind( double x0, double x1 ) const;

private:
   std::vector<double> m_coeffs;
};

/*!
 * \namespace Algorithms
 *
 * \brief Beer-related math functions, arithmetic, and CS algorithms.
 */
namespace Algorithms {

   //===========================Generic stuff==================================

   //! \brief Cross-platform NaN checker.
   bool isNan(double d);

   //! \brief Cross-platform Inf checker.
   template<typename T> bool isInf(T var)
   {
      return
      (
         std::numeric_limits<T>::has_infinity &&
         var == std::numeric_limits<T>::infinity()
         //(var < std::numeric_limits<T>::min() || var > std::numeric_limits<T>::max())
      );
   }

   //! \brief Cross-platform rounding.
   double round(double d);

   //===================Beer-related stuff=====================

   //! \returns plato of \b sg
   double SG_20C20C_toPlato( double sg );
   //! \returns sg of \b plato
   double PlatoToSG_20C20C( double plato );
   //! \returns water density in kg/L at temperature \b celsius
   double getWaterDensity_kgL( double celsius );
   //! \returns additive correction to the 15C hydrometer reading if read at \b celsius
   double hydrometer15CCorrection( double celsius );

   /*!
    * \brief Return the approximate color for a given SRM value
    */
   QColor srmToColor(double srm);

   /*!
    * \brief Given dissolved sugar and wort volume, get SG in Plato
    *
    * Estimates Plato from kg of dissolved sucrose (\c sugar_kg) and
    * the total wort volume \c wort_l.
    *
    * \param sugar_kg kilograms of dissolved sucrose or equivalent
    * \param wort_l liters of wort
    */
   double getPlato( double sugar_kg, double wort_l );
   //! \brief Converts FG to plato, given the OG.
   double ogFgToPlato( double og, double fg );
   //! \brief Gets ABV by using current gravity reading and brix reading.
   double getABVBySGPlato( double sg, double plato );
   //! \brief Gets ABW from current gravity and plato.
   double getABWBySGPlato( double sg, double plato );
   //! \brief Gives you the SG from the starting plato and current plato.
   double sgByStartingPlato( double startingPlato, double currentPlato );
   //! \brief Returns the refractive index from plato.
   double refractiveIndex( double plato );
   //! \brief Corrects the apparent extract 'plato' to the real extract using current gravity 'sg'.
   double realExtract( double sg, double plato );
   //! \brief Calculate ABV from OG and FG
   double abvFromOgAndFg(double og, double fg);
   //! \brief Correct specific gravity reading for the temperature at which it was taken
   double correctSgForTemperature(double measuredSg, double readingTempInC, double calibrationTempInC);
}

#endif
