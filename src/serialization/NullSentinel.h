/*======================================================================================================================
 * serialization/NullSentinel.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef SERIALIZATION_NULLSENTINEL_H
#define SERIALIZATION_NULLSENTINEL_H
#pragma once

#include <optional>

#include <QMetaType>
#include <QVariant>

#include "utils/OptionalHelpers.h"

namespace Serialization {
   /**
    * \brief Used in \c XmlRecordDefinition::FieldDefinition and \c JsonRecordDefinition::FieldDefinition for fields
    *        that are required in the file format but optional (ie \c std::optional) in our internal data model.  Eg
    *        FERMENTABLE/YIELD is required in BeerXML, but \c Fermentable::fineGrindYield_pct is optional (because a
    *        fermentable might instead have \c coarseGrindYield_pct or \c potentialYield_sg).
    *
    *        \c value is what we write out if the property is \c std::nullopt, so that the output is valid.
    *
    *        If \c mode is \c Mode::ReadAndWrite (the default), then, when reading a file, we also treat \c value as
    *        meaning "not set" -- ie we set the property to \c std::nullopt.  This makes round-tripping consistent.
    *
    *        However, there are some fields where \c value is also a perfectly normal value, so we cannot treat it as
    *        "not set" when reading, and \c mode should be \c Mode::WriteOnly.  Eg in BeerXML, HOP/TIME of 0 for a boil
    *        addition means "at the end of the boil", whereas if \c RecipeAddition::addAtTime_mins is not set, the
    *        addition is at the start of the step.
    *
    *        Currently only supported for numeric fields (and, in JSON, for measurements of a single number).
    */
   struct NullSentinel {
      enum class Mode {
         ReadAndWrite,
         WriteOnly
      };
      QVariant m_value;
      Mode     m_mode = Mode::ReadAndWrite;

      /**
       * \brief Substitute the sentinel value for the supplied one if the latter is null
       *
       * @tparam T
       * @param value
       * @return \c true if a substitution was made, \c false otherwise
       */
      template<typename T>
      bool replaceNullWithSentinel(QVariant & value) const {
         if (Optional::isNotNull<T>(value)) {
            return false;
         }
         value = this->m_value;
         return true;
      }
   };

   /**
    * \brief When reading a file, if \c parsedValue (which should hold a \c std::optional of a numeric type) is equal to
    *        \c nullSentinel.value (and \c nullSentinel.mode is \c NullSentinel::Mode::ReadAndWrite), then replace it
    *        with \c std::nullopt.  Does nothing if \c nullSentinel is \c std::nullopt or \c propertyIsOptional is
    *        \c false.
    *
    * \return \c true if \c parsedValue was replaced with \c std::nullopt, \c false otherwise
    */
   inline bool applyNullSentinelOnRead(std::optional<NullSentinel> const & nullSentinel,
                                    bool const propertyIsOptional,
                                    QVariant & parsedValue) {
      if (!nullSentinel || nullSentinel->m_mode != NullSentinel::Mode::ReadAndWrite || !propertyIsOptional) {
         return false;
      }
      auto replaceIfNull = [&]<typename T>() {
         if (parsedValue.metaType() != QMetaType::fromType<std::optional<T>>()) {
            return false;
         }
         auto const value = parsedValue.value<std::optional<T>>();
         if (value && *value == nullSentinel->m_value.value<T>()) {
            parsedValue = QVariant::fromValue(std::optional<T>{});
            return true;
         }
         return false;
      };
      return replaceIfNull.template operator()<double      >() ||
             replaceIfNull.template operator()<int         >() ||
             replaceIfNull.template operator()<unsigned int>();
   }
}

#endif
