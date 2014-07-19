/*
 * Algorithms.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_

#define ROOT_PRECISION 0.0000001

#include <QList>
#include <QColor>
#include <limits> // For std::numeric_limits
#include <vector>
#include <cassert>
#include <cmath>
#include <string.h>

/*!
 * \brief Class to encapsulate real polynomials in a single variable
 * \author Philip G. Lee
 */
class Polynomial
{
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
      assert( n <= _coeffs.size() );
      return _coeffs[n];
   }
   
   //! \brief Get coefficient of x^n where \c n <= \c order() (non-const)
   double& operator[] (size_t n)
   {
      assert( n < _coeffs.size() );
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
 * \class Algorithms
 * \author Philip G. Lee
 * \author Eric Tamme
 *
 * \brief Beer-related math functions, arithmetic, and CS algorithms.
 */
class Algorithms
{
public:

   //===========================Generic stuff==================================
   
   //! \brief Cross-platform NaN checker.
   static bool isNan(double d)
   {
      // If using IEEE floating points, all comparisons with a NaN
      // are false, so the following should be true only if we have
      // a NaN.
      return (d != d);
   }
   
   //! \brief Cross-platform Inf checker.
   template<typename T> static bool isInf(T var)
   {
      return
      (
         std::numeric_limits<T>::has_infinity &&
         var == std::numeric_limits<T>::infinity()
         //(var < std::numeric_limits<T>::min() || var > std::numeric_limits<T>::max())
      );
   }
   
   //! \brief Cross-platform rounding.
   static double round(double d);

   //===================Beer-related stuff=====================
   
   //! \returns plato of \b sg
   static double SG_20C20C_toPlato( double sg );
   //! \returns sg of \b plato
   static double PlatoToSG_20C20C( double plato );
   //! \returns water density in kg/L at temperature \b celsius
   static double getWaterDensity_kgL( double celsius );
   //! \returns additive correction to the 15C hydrometer reading if read at \b celsius
   static double hydrometer15CCorrection( double celsius );

   /*!
    * \brief Return the approximate color for a given SRM value
    */
   static QColor srmToColor(double srm)
   {
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
   
   /*!
    * \brief Given dissolved sugar and wort volume, get SG in Plato
    * 
    * Estimates Plato from kg of dissolved sucrose (\c sugar_kg) and
    * the total wort volume \c wort_l.
    * 
    * \param sugar_kg kilograms of dissolved sucrose or equivalent
    * \param wort_l liters of wort
    */
   static double getPlato( double sugar_kg, double wort_l );
   //! \brief Converts FG to plato, given the OG.
   static double ogFgToPlato( double og, double fg );
   //! \brief Gets ABV by using current gravity reading and brix reading.
   static double getABVBySGPlato( double sg, double plato );
   //! \brief Gets ABW from current gravity and plato.
   static double getABWBySGPlato( double sg, double plato );
   //! \brief Gives you the SG from the starting plato and current plato.
   static double sgByStartingPlato( double startingPlato, double currentPlato );
   //! \brief Returns the refractive index from plato.
   static double refractiveIndex( double plato );
   //! \brief Corrects the apparent extract 'plato' to the real extract using current gravity 'sg'.
   static double realExtract( double sg, double plato );

private:
   // This is the cubic fit to get Plato from specific gravity, measured at 20C
   // relative to density of water at 20C.
   // P = -616.868 + 1111.14(SG) - 630.272(SG)^2 + 135.997(SG)^3
   static Polynomial platoFromSG_20C20C;
   
   // Water density polynomial, given in kg/L as a function of degrees C.
   // 1.80544064e-8*x^3 - 6.268385468e-6*x^2 + 3.113930471e-5*x + 0.999924134
   static Polynomial waterDensityPoly_C;
   
   // Polynomial in degrees Celsius that gives the additive hydrometer
   // correction for a 15C hydrometer when read at a temperature other
   // than 15C.
   static Polynomial hydroCorrection15CPoly;

   // Hide constructors and assignment op.
   Algorithms(){}
   Algorithms(Algorithms const&){}
   Algorithms& operator=(Algorithms const& other){ return *this; }
   ~Algorithms(){}
};

#endif /* ALGORITHMS_H_ */
