/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/ObjectAddressStringMapping.h is part of Brewtarget, and is copyright the following authors 2023:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef UTILS_OBJECTADDRESSSTRINGMAPPING_H
#define UTILS_OBJECTADDRESSSTRINGMAPPING_H
#pragma once

#include <vector>

#include <QDebug>
#include <QString>

#include "Logging.h"
#include "utils/MetaTypes.h"

/**
 * \brief For global const objects, such as \c Unit, \c UnitSystem and so on, this provides a mechanism to map them
 *        to or from a string ID suitable for storing in the DB.  Eg for a recipe addition amount or inventory amount
 *        that can be measured by mass or by volume, we would use a pointer or reference to
 *        \c Measurement::Units::kilograms or \c Measurement::Units::liters in memory, we might store "kilograms" or
 *        "liters" in the database.
 *
 *        Although, in cases where we are using this mapping, there would often be an alternative of "just" storing eg
 *        \c Measurement::PhysicalQuantity, it's better that data in the database be as unambiguous as possible - both
 *        for debugging and for any users who want to look at or use the database data directly.  Thus rather than
 *        "quantity : 3.2 / measure : 'mass'", we prefer "quantity : 3.2 / unit : 'kilograms'".  However, we still stick
 *        to the principle of only storing things in canonical metric values, so the code will assert if it finds an
 *        amount stored in the DB in grams or ounces.
 *
 *        NOTE unlike, eg, \c Measurement::Unit::name, the serialisation strings (a) are not localised and (b) should be
 *        globally unique for the type we are dealing with (hence "liters" and "lintner" rather than "L" and "L").  The
 *        latter constraint means, among other things, that we only need one lookup table for all global Unit objects
 *        (\c Measurement::Units::unitStringMapping) rather than multiple individual ones.
 */
template<class T>
class ObjectAddressStringMapping{
public:
   struct ObjectAddressAndItsString{
      T const * address;
      QString   string ;

      //! Standard constructor saves us prefixing everything with & at the call site.
      ObjectAddressAndItsString(T const & address, QString string) :
         address{&address},
         string{string} {
         return;
      }
   };

   ObjectAddressStringMapping(std::initializer_list<ObjectAddressAndItsString> args) : m_map{args} {
      return;
   }

   /**
    * \param stringValue
    * \param caseInensitiveFallback If \c true (the default), this means we'll do a case-insensitive search if we didn't
    *                               find \c stringValue as a case-sensitive match.
    * \return \c nullptr if the supplied string could not be found in the mapping
    */
   T const * stringToObjectAddress(QString const & stringValue, bool const caseInensitiveFallback = true) const {
      auto match = std::find_if(this->m_map.begin(),
                                this->m_map.end(),
                                [stringValue](ObjectAddressAndItsString const & ii){return stringValue == ii.string;});
      //
      // If we didn't find an exact match, we'll try a case-insensitive one if so-configured.  (We don't do this by
      // default as the assumption is that it's rare we'll need the case insensitivity.)
      //
      if (match == this->m_map.end() && caseInensitiveFallback) {
         match = std::find_if(
            this->m_map.begin(),
            this->m_map.end(),
            [stringValue](ObjectAddressAndItsString const & ii){return stringValue.toLower() == ii.string.toLower();}
         );
      }

      if (match == this->m_map.end()) {
         return nullptr;
      }

      return match->address;
   }

   /**
    * \brief Convert data that \b might not be a valid \c int value of an enum to the string representation of that
    *        enum.  This is possibly useful in some circumstances when we are reading a Qt property of an object and we
    *        can't be 100% certain that the object is of the class we expect
    */
   QString objectAddressToString(T const * objectAddress) const {
      auto match = std::find_if(
         this->m_map.begin(),
         this->m_map.end(),
         [objectAddress](ObjectAddressAndItsString const & ii){return objectAddress == ii.address;}
      );

      if (match == this->m_map.end()) {
         // This is very probably a coding error
         qCritical() <<
            Q_FUNC_INFO << "Could not find string for " << objectAddress << "(in " << this->m_map.size() << "entries)";
         qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
         Q_ASSERT(false);
         return "";
      }

      return match->string;
   }

private:
   // I'm not sure the exact container we use makes that much difference given the typically small number of entries
   // we're storing.  There's an advantage to using std::vector over QVector in that the former does not require
   // default-constructable types.
   std::vector<ObjectAddressAndItsString> m_map;

};

#endif
