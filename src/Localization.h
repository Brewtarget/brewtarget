/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * Localization.h is part of Brewtarget, and is copyright the following authors 2011-2023:
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#ifndef LOCALIZATION_H
#define LOCALIZATION_H
#pragma once

#include <QDate>
#include <QLocale>
#include <QString>

class BtStringConst;
class NamedEntity;

namespace Localization {
   /**
    * \brief Returns the locale to use for formatting numbers etc.  Usually this is the same as \c QLocale::system(),
    *        but can be overridden for testing purposes.
    */
   QLocale const & getLocale();

   /**
    * \brief Coding for the three main numeric date formats
    *        See https://en.wikipedia.org/wiki/Date_format_by_country for which countries use which date formats
    */
   enum NumericDateFormat {
      //
      // The numbers here are for backwards compatibility.  We used to (ab)use Measurement::Unit::unitDisplay as the
      // code for date format with
      //              Measurement::Unit::displayUS  = mm-dd-YYYY,
      //              Measurement::Unit::displayImp = dd-mm-YYYY,
      //              Measurement::Unit::displaySI  = YYYY-mm-dd.
      // By keeping the same numeric values for the new date-specific flags, we should still correctly load config from
      // persistent settings (where the value of the enum is stored as an int).
      //
      YearMonthDay = 0x100,  // AKA ISO 8601, AKA the one true date format
      DayMonthYear = 0x102,  // What's mostly used outside the USA
      MonthDayYear = 0x101   // What's used in the USA
   };

   /**
    * \brief Set the current date format
    */
   void setDateFormat(NumericDateFormat newDateFormat);

   /**
    * \brief Get the current date format
    */
   NumericDateFormat getDateFormat();

   /**
    * \brief Convert an enum date format to the string equivalent suitable for Qt functions
    */
   QString numericToStringDateFormat(NumericDateFormat numericDateFormat);

   /**
    * \brief Display date formatted for the locale.
    */
   QString displayDate(QDate const & date);

   /**
    * \brief Display date formatted based on the user defined options.
    */
   QString displayDateUserFormated(QDate const & date);

   /**
    * \brief Loads the translator with two letter ISO 639-1 code.
    *
    *        For example, for Spanish, it would be 'es'. Currently, this does NO checking to make sure the locale code
    *        is acceptable.
    *
    * \param twoLetterLanguage two letter ISO 639-1 code
    */
   void setLanguage(QString twoLetterLanguage);

   /**
    * \brief Gets the 2-letter ISO 639-1 language code we are currently using.
    * \returns current 2-letter ISO 639-1 language code.
    */
   QString const & getCurrentLanguage();

   /**
    * \brief Gets the ISO 639-1 language code for the system.
    * \returns current 2-letter ISO 639-1 system language code
    */
   QString const & getSystemLanguage();

   /**
    * \brief Convert a \c QString to a \c double, if possible using the default locale and if not using the C locale.
    *
    *        Qt5 changed how QString::toDouble() works in that it will always convert in the C locale.  We are
    *        instructed to use QLocale::toDouble instead, except that will never fall back to the C locale.  This
    *        doesn't really work for us, so this function emulates the old behavior.
    * \param text
    * \param ok
    */
   double toDouble(QString text, bool* ok = nullptr);

   /**
    * \brief Convenience wrapper around \c toDouble()
    *
    * \param element
    * \param propertyName
    * \param caller Callers should use the \c Q_FUNC_INFO macro to supply this parameter
    */
   double toDouble(NamedEntity const & element, BtStringConst const & propertyName, char const * const caller);

   /**
    * \brief Convenience wrapper around \c toDouble()
    *
    * \param text
    * \param caller Callers should use the \c Q_FUNC_INFO macro to supply this parameter
    */
   double toDouble(QString text, char const * const caller);

   /**
    * \brief For a given string, determines whether it is just a number or a number plus units.
    *
    *        This is used as part of the mechanism to allow users to enter any relevant units in an input field.  Eg, if
    *        a field defaults to accepting grams, then entering a number will be interpreted as an amount in grams, but
    *        entering a number plus units, eg "0.5 kg" or "37mg" or "4 oz" "0.5 lb", etc, will convert the amount
    *        entered to the corresponding amount in grams.
    *
    *        Non-integer numbers can be entered using whatever decimal separator (ie '.' or ',') and digit grouping
    *        delimiter (usually ',', '.' or ' ') are configured on the user's system.  See
    *        https://en.wikipedia.org/wiki/Decimal_separator#Usage_worldwide and related links for which countries use
    *        which.
    *
    *        Thus this function accepts X,XXX.YZ (or X.XXX,YZ in locales where decimal comma is used instead of decimal
    *        point) as well as .YZ (or ,YZ) followed by some unit string
    *
    * \returns \c true if the supplied string has units, \c false otherwise
    */
   bool hasUnits(QString qstr);

///   /**
///    * \brief Load translation files.
///    */
///   void loadTranslations();

   /**
    * \brief Load localization settings from persistent storage
    */
   void loadSettings();

   /**
    * \brief Save localization settings to persistent storage
    */
   void saveSettings();
}

#endif
