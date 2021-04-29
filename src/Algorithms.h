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
#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_

#define ROOT_PRECISION 0.0000001

#include <cmath>
#include <limits> // For std::numeric_limits
#include <string.h>
#include <vector>

#include <QColor>
#include <QList>

/*!
 * \brief Class to encapsulate real polynomials in a single variable
 */
class Polynomial {
public:
   //! \brief Default constructor
   Polynomial() :
      _coeffs()
   {
   }

   //! \brief Copy constructor
   Polynomial( Polynomial const& other ) :
      _coeffs( other._coeffs )
   {
   }

   //! \brief Constructs the 0 polynomial with given \c order
   Polynomial( size_t order ) :
      _coeffs(order+1, 0.0)
   {
   }

   //! \brief Constructor from an array of coefficients
   Polynomial( double const* coeffs, size_t order ) :
      _coeffs(coeffs, coeffs+order+1)
   {
   }

   //! \brief Add a coefficient for x^(\c order() + 1)
   Polynomial& operator<< ( double coeff )
   {
      _coeffs.push_back(coeff);
      return *this;
   }

   //! \brief Get the polynomial's order (highest exponent)
   size_t order() const { return _coeffs.size()-1; }

   //! \brief Get coefficient of x^n where \c n <= \c order()
   double operator[] (size_t n) const
   {
      Q_ASSERT( n <= _coeffs.size() );
      return _coeffs[n];
   }

   //! \brief Get coefficient of x^n where \c n <= \c order() (non-const)
   double& operator[] (size_t n)
   {
      Q_ASSERT( n < _coeffs.size() );
      return _coeffs[n];
   }

   //! \brief Evaluate the polynomial at point \c x
   double eval(double x) const
   {
      double ret = 0.0;
      size_t i;

      for( i = order(); i > 0; --i )
         ret += _coeffs[i] * intPow( x, i );
      ret += _coeffs[0];

      return ret;
   }

   /*!
    * \brief Root-finding by the secant method.
    *
    * \param x0 - one of two initial \b distinct guesses at the root
    * \param x1 - one of two initial \b distinct guesses at the root
    * \returns \c HUGE_VAL on failure, otherwise a root of the polynomial
    */
   double rootFind( double x0, double x1 ) const
   {
      double guesses[] = { x0, x1 };
      double newGuess = x0;
      double maxAllowableSeparation = qAbs( x0 - x1 ) * 1e3;

      while( qAbs( guesses[0] - guesses[1] ) > ROOT_PRECISION )
      {
         newGuess = guesses[1] - (guesses[1] - guesses[0]) * eval(guesses[1]) / ( eval(guesses[1]) - eval(guesses[0]) );

         guesses[0] = guesses[1];
         guesses[1] = newGuess;

         if( qAbs( guesses[0] - guesses[1] ) > maxAllowableSeparation )
            return HUGE_VAL;
      }

      return newGuess;
   }

private:
   std::vector<double> _coeffs;

   //! \brief returns base^pow
   static double intPow( double base, unsigned int pow )
   {
      double ret = 1;
      for(; pow > 0; pow--)
         ret *= base;

      return ret;
   }
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
}

#endif
