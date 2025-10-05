/*======================================================================================================================
 * measurement/CurrencyAmount.h is part of Brewtarget, and is copyright the following authors 2025:
 *   • Matt Young <mfsy@yahoo.com>
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
 =====================================================================================================================*/
#ifndef MEASUREMENT_CURRENCYAMOUNT_H
#define MEASUREMENT_CURRENCYAMOUNT_H
#pragma once

#include <QString>

/**
 * \brief Small class to represent an amount of money
 *
 *        We don't want to store currency amounts as floating point numbers because of rounding errors (see
 *        https://0.30000000000000004.com/).  In theory, since C++ incorporates C, we can use the C23 IEEE-754 decimal
 *        types such as _Decimal32.  In practice, there is little to no built-in support for manipulating such numbers.
 *        We could also use boost::multiprecision::cpp_dec_float, but this is overkill for our needs. (See
 *        https://stackoverflow.com/questions/149033/best-way-to-store-currency-values-in-c/73551215#73551215 for
 *        discussion and examples.)
 *
 *        So this class gives us "just enough" functionality to handle any currency whose principal subunit (if any) is
 *        1/100 of the main unit.
 *
 *        NOTE: This lives with the "measurement" code for want of a better home but, for now at least, I didn't put it
 *              in the \c Measurement namespace as it's not really a measurement.
 */
struct CurrencyAmount {
   /**
    * \brief Normally this will be the same as \c QLocale::currencySymbol, but we allow the user to override it, eg for
    *        cross-border purchases.
    */
   QString currencySymbol;

   /**
    * \brief This is the actual stored value: the total amount converted to "cents", ie hundredths of the main currency
    *        unit, which works for a lot of currencies.  (Eg this is cents for dollars; cents/centimes for euros,
    *        pence/piastre/etc for pounds.)
    */
   int asCents;

   //! Construct from display value
   explicit CurrencyAmount(QString const & inputString);

   //! Construct from currency symbol and amount.
   explicit CurrencyAmount(double const amount, QString const symbol);

   /**
    * Default constructor is required if we are passing things through the Qt Property system.
    * NOTE that this will construct a zero amount in default currency
    */
   CurrencyAmount();

   ~CurrencyAmount();

   //! Copy constructor
   CurrencyAmount(CurrencyAmount const & other);

   //! Copy assignment operator
   CurrencyAmount & operator=(CurrencyAmount const & other);

   //! Move constructor
   CurrencyAmount(CurrencyAmount && other) noexcept;

   //! Move assignment operator
   CurrencyAmount & operator=(CurrencyAmount && other) noexcept;

   bool                  operator== (CurrencyAmount const & other) const;
   bool                  operator!= (CurrencyAmount const & other) const;
   std::partial_ordering operator<=>(CurrencyAmount const & other) const;

   /**
    * \brief Returns the units and cents as a \c double (with possible rounding errors which can usually be ignored
    *        provided we are not trying to do arithmetic or comparisons with the results).
    *
    *        This is, eg, fine for storing in a database (if we store the currency symbol separately).
    */
   double asUnits() const;

   /**
    * \brief Returns a string (including currency symbol) suitable for display or serialisation.
    */
   QString asDisplayable() const;

   /**
    * \brief This returns the "cents" part of the amount, ie an integer between 0 and 99.
    */
   int centsPart() const;

   /**
    * \brief This returns the "units" part of the amount, ie whole dollars/euros/pounds/etc.
    */
   int unitsPart() const;


};

/**
 * \brief Convenience function to allow output of \c CurrencyAmount to \c QDebug or \c QTextStream stream etc
 */
template<class S> S & operator<<(S & stream, CurrencyAmount const & amount) {
   stream << amount.asDisplayable();
   return stream;
}

#endif
