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

#include <vector>

#include <QCoreApplication>
#include <QObject>
#include <QString>


/**
 * \brief Struct to represent info about an individual currency.
 */
struct CurrencyInfo {
   //
   // This gives us CurrencyInfo::tr, but means we have to declare a constructor.
   //
   Q_DECLARE_TR_FUNCTIONS(CurrencyInfo)

public:
   /**
    * \brief The three-letter ISO-4217 alphabetic code for this currency -- as per \c QLocale::CurrencyIsoCode.
    */
   QString m_isoAlphabeticCode;
   /**
    * \brief The numeric code is 3 digits and can have leading zeros, so it's easiest to store it as a string.  In any
    *        case, it's not meaningful to do any sort of arithmetic or numerical operation on it, so there would be no
    *        benefit to storing it as numeric type.
    */
   QString m_isoNumericCode;

   /**
    * \brief The official name of the currency - eg "United States Dollar" or "Pound Sterling"
    */
   QString m_fullName;

   /**
    * \brief The day-to-day name of the currency - eg "Dollar" or "Pound".  For current locale's currency -- as per
    *        \c QLocale::CurrencyDisplayName
    */
   QString m_shortName;

   /**
    * \brief Symbol for major units of this currency -- as per \c QLocale::CurrencySymbol
    *
    *        There isn't always a canonical symbol or abbreviation for a currency.  Eg Sri Lankan Rupee is abbreviated
    *        as "₨" in English, "රු" in Sinhala, "௹" in Tamil.  Where there isn't a canonical one, we'll put the
    *        English one here, and deal with other variants via translations.
    *
    *        This is the symbol for what we call the "units" - dollars, euros, pounds, etc.  We do not hold or use a
    *        symbol for the "cents" (aka "minor units") such as cents, pennies, etc.  This is because it's largely
    *        sufficient to write "$1.23" or "4,56 €" etc.
    *
    */
   QString m_unitSymbol;

   /**
    * \brief ISO 4217 doesn't include the name of the minor units of currency (eg cents, pence, etc).  We call it
    */
   QString m_centsName;

   /**
    * \brief In ISO 4217 this is the "minor unit fraction".  But our name is more intuitive.  "0" means that there is no
    *        minor unit for the currency, whereas "1", "2" and "3" signify a ratio of 10:1, 100:1 and 1000:1
    *        respectively.
    */
   int m_numDigitsAfterDecimal;

   /**
    * \brief In theory this can be derived from numDigitsAfterDecimal, but, in a few cases, it cannot.  Eg one
    *        Mauritanian Ouguiya is worth five Khoums.
    *
    *        0 means there are no "cents" (aka minor units).
    */
   int m_centsPerUnit;

   CurrencyInfo(QString const & isoAlphabeticCode    ,
                QString const & isoNumericCode       ,
                QString const & fullName             ,
                QString const & shortName            ,
                QString const & unitSymbol           ,
                QString const & centsName            ,
                int     const   numDigitsAfterDecimal,
                int     const   centsPerUnit         );

   static std::vector<CurrencyInfo> const & getAll();

   static CurrencyInfo const * getFromIsoAlphabeticCode(QString const & isoAlphabeticCode);
};

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
 *        Qt has limited support for currencies in the \c QLocale class (see \c QLocale::toCurrencyString,
 *        \c QLocale::currencySymbol) but it is relatively limited.  Eg, although it can give you a locale's "currency
 *        symbol" in one of the following three formats, it does not provide a more general API for converting between
 *        them:
 *              \c QLocale::CurrencyIsoCode     = ISO-4217 code (USD, GBP, EUR, etc)
 *              \c QLocale::CurrencySymbol      = Symbol ($, £, €, etc)
 *              \c QLocale::CurrencyDisplayName = User readable name (Dollars, Pounds, Euros, etc)
 *
 *        So this class gives us "just enough" functionality to handle any currency whose principal subunit (if any) is
 *        1/100 of the main unit -- which is most (but not all) currencies.
 *
 *        NOTE: This lives with the "measurement" code for want of a better home but, for now at least, I didn't put it
 *              in the \c Measurement namespace as it's not really a measurement.
 */
struct CurrencyAmount {
   /**
    * \brief The ISO-4217 alphabetic code of the currency.  This is the canonical way for unambiguously referring to
    *        different currencies.
    */
   CurrencyInfo const * m_currencyInfo;

   /**
    * \brief This is the actual stored value: the total amount converted to "cents", ie the minor units of the main
    *        currency unit, which are typically hundredths of it.  (Eg this is cents for dollars; cents/centimes for
    *        euros, pence/piastre/etc for pounds.)
    *
    *        Despite the name, we also handle other cases:
    *          - For a small number of currencies, the minor units are not hundredths but, instead are thousandths or
    *            twentieths.  We just use a different multiplier for these.
    *          - A few currencies have no minor units.  So we just store major units in this value.
    *          - In some cases, minor units exist but are not in common usage.  Where the minor units have been
    *            officially removed from circulation (eg the "sen" and "rin" subdivisions of Japanese yen were removed
    *            from circulation in 1953), we'll treat the currency as having no minor units.
    */
   int m_totalAsCents;

   /**
    * \brief Construct from display value.
    *
    *        Where no currency identifier is supplied the one from the current locale is assumed.
    *
    *        Where currency identifier is ambiguous (eg "$" rather than "USD", "CAD", "AUD", "NZD") we'll make a best
    *        guess at what currency is meant.
    */
   explicit CurrencyAmount(QString const & inputString);

   //! Construct from currency symbol (or code) and amount (in units).
   explicit CurrencyAmount(QString const symbolOrCode, double const amount);

   //! Construct from currency symbol (or code) and amount (in cents).
   explicit CurrencyAmount(QString const symbolOrCode, int const amountAsCents);

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

   void setCurrencyFromSymbolOrCode(QString const & symbolOrCode);

   bool                  operator== (CurrencyAmount const & other) const;
   bool                  operator!= (CurrencyAmount const & other) const;
   std::partial_ordering operator<=>(CurrencyAmount const & other) const;

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(QString currencyCode      READ currencyCode      WRITE setCurrencyCode)

   /**
    * \brief Returns the units and cents as a \c double (with possible rounding errors which can usually be ignored
    *        provided we are not trying to do arithmetic or comparisons with the results).
    *
    *        This is, eg, fine for storing in a database (if we store the currency symbol separately).
    */
   double asUnits() const;

   /**
    * \brief Returns a string (including currency symbol) suitable for display or serialisation.
    *
    *        Display format depends on locale (eg "€1.23" vs "1,23 €") and user preference for symbol vs code (eg
    *        "$4.56" vs "CAD 4.56").
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
