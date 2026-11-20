/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/EnumStringMapping.h is part of Brewtarget, and is copyright the following authors 2021-2026:
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
#ifndef UTILS_ENUMSTRINGMAPPING_H
#define UTILS_ENUMSTRINGMAPPING_H
#pragma once

#include <optional>
#include <stdexcept>

#include <QDebug>
#include <QString>
#include <QVector>

#include "Logging.h"
#include "utils/EnumMapping.h"
#include "utils/TranslatableText.h"
#include "utils/TypeLookup.h"

using EnumAndItsString = EnumKeyValue<QString>;

/**
 * \class EnumStringMapping
 *
 *        We don't actually bother creating hashmaps or similar between enum values and string representations
 *        because it's usually going to be a short list that we can search through pretty quickly (probably faster
 *        than calculating the hash of a key!)
 *
 *        In theory, it would suffice to use an array here rather than a vector, because the data for the mapping is
 *        always known at compile-time.  This would also allow us to make more things const or constexpr.  In practice,
 *        it's a bit clunky using arrays (eg length of std::array is a template parameter that cannot always be deduced
 *        by the compiler when it's an array of structs), and the extra "cost" of a vector here is negligible.
 *
 *        TBD: Could look at Boost Bimap
 *
 *        NOTE: We can get the number of values in the mapping by calling \c size (inherited from \c QVector).  By
 *              virtue of the fact that we always start our enums at 0 and don't skip any values, this also tells us
 *              the number of values in the enum (subject to the assumption that every value in the enum was given a
 *              mapping entry).  Use \c FlagEnumStringMapping for enums such as \c Ingredient::Measure where this is not
 *              the case.
 *
 *        NOTE: This class is for strings that are \b not translated -- typically serialisation strings for BeerJSON,
 *              the DB etc.  For localised display names, use \c EnumDisplayNameMapping (below).
 */
class EnumStringMapping : public EnumMapping<QString> {
public:
   using EnumMapping::EnumMapping;

   /**
    * \brief Convert data that \b might not be a valid string representation of an enum to the \c int value of that
    *        enum.  This is useful when we are deserialising a file for instance
    * \param stringValue
    * \param caseInsensitiveFallback If \c true (the default), this means we'll do a case-insensitive search if we didn't
    *                               find \c stringValue as a case-sensitive match.
    */
   std::optional<int> stringToEnumAsInt(QString const & stringValue, bool const caseInsensitiveFallback = true) const;

   /**
    * \brief Convenience function for using \c stringToEnum with strongly typed enums (ie those declared as
    *        "enum class")
    *
    * \throw std::out_of_range if no enum is found for the supplied string (because this is a coding error)
    */
   template<typename E>
   E stringToEnum(QString const & stringValue) const {
      std::optional<int> result = this->stringToEnumAsInt(stringValue);
      if (!result) {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Coding error: no enum mapping found for" << stringValue << Logging::getStackTrace();
         throw std::out_of_range("Missing enum value in EnumStringMapping");
      }
      return static_cast<E>(*result);
   }

   /**
    * \brief Convenience function for using \c stringToEnum with strongly typed enums (ie those declared as
    *        "enum class")
    */
   template<typename E>
   std::optional<E> stringToEnumOrNull(QString const & stringValue) const {
      std::optional<int> result = this->stringToEnumAsInt(stringValue);
      return result ? std::optional<E>{static_cast<E>(*result)} : std::optional<E>{std::nullopt};
   }
};

class FlagEnumStringMapping : public EnumStringMapping {
public:
   FlagEnumStringMapping(std::initializer_list<EnumAndItsString> args);
   std::optional<QString> enumAsIntToValue(int const enumValue) const;

};

/**
 * \class EnumDisplayNameMapping
 *
 * \brief Maps enum values to localised display names, which are translated at time of use rather than time of
 *        construction.
 *
 *        Our display name mappings (\c Hop::formDisplayNames, \c Measurement::physicalQuantityDisplayNames, etc) are
 *        all constructed during static initialisation.  If we called \c tr() there, we would always get the
 *        untranslated text, because that happens before \c QApplication exists and before any \c QTranslator has
 *        been installed.  (And, even if that were not the case, we would not pick up a change of language while the
 *        program is running.)  So instead we store the untranslated text, marked with \c QT_TRANSLATE_NOOP so that
 *        \c lupdate still finds it, and translate it each time it is looked up.  Eg:
 *
 *           EnumDisplayNameMapping const Hop::formDisplayNames {"Hop", {
 *              {Hop::Form::Leaf   , QT_TRANSLATE_NOOP("Hop", "Leaf"   )},
 *              {Hop::Form::Pellet , QT_TRANSLATE_NOOP("Hop", "Pellet" )},
 *              ...
 *           }};
 *
 *        The context passed to the constructor \b must match the one in each \c QT_TRANSLATE_NOOP, otherwise the
 *        lookup will not find the translations.  By convention, it is the name of the class that owns the mapping (or
 *        "QObject" where that is what we used historically).
 *
 *        This is deliberately a separate class from \c EnumStringMapping (rather than, eg, a template parameter on it)
 *        so that the compiler stops us accidentally passing a serialisation mapping where we need display names or
 *        vice versa.  For the same reason, we do not expose the underlying container: everything you can get out of
 *        this class is already translated.
 *
 *        As with \c EnumStringMapping, the entries must be supplied in enum order, starting from 0 with no gaps.
 */
class EnumDisplayNameMapping {
public:
   EnumDisplayNameMapping(char const * const context, std::initializer_list<EnumKeyValue<char const *>> args);

   /**
    * \brief Number of entries, which is also the number of values in the enum.  See comment in \c EnumStringMapping.
    */
   [[nodiscard]] qsizetype size() const;

   /**
    * \brief The translation context we were constructed with.  Mostly useful for logging and debugging.
    */
   [[nodiscard]] char const * context() const;

   /**
    * \brief Returns the untranslated text for the supplied enum value, as a \c TranslatableText that can be stored and
    *        translated later (eg by a widget that needs to update its text when the language changes).
    */
   [[nodiscard]] std::optional<TranslatableText> enumAsIntToTranslatableText(int const enumValue) const;

   /**
    * \brief Returns the translated display name for the supplied enum value, or \c std::nullopt if the value is not
    *        valid for this enum.
    */
   [[nodiscard]] std::optional<QString> enumAsIntToValue(int const enumValue) const;

   /**
    * \brief Convenience function for use with strongly typed enums (ie those declared as "enum class")
    *
    * \throw std::out_of_range if no string is found for the supplied enum (because this is a coding error)
    */
   template<typename E>
   [[nodiscard]] QString enumToValue(E const enumValue) const {
      std::optional<QString> result = this->enumAsIntToValue(static_cast<int>(enumValue));
      if (!result) {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Coding error: no display name found in context" << this->m_context << "for" <<
            static_cast<int>(enumValue) << Logging::getStackTrace();
         throw std::out_of_range("Missing value in EnumDisplayNameMapping");
      }
      return *result;
   }

   /**
    * \brief A more concise version of \c enumToValue.  See comments in \c EnumMapping.
    */
   template<typename E, typename = std::enable_if_t<is_non_optional<E>::value> >
   [[nodiscard]] QString operator[](E const enumValue) const {
      return this->enumToValue(enumValue);
   }

   template<typename E, typename = std::enable_if_t<is_non_optional<E>::value> >
   [[nodiscard]] QString operator[](std::optional<E> const optionalEnumValue) const {
      return optionalEnumValue ? this->enumToValue(*optionalEnumValue) : QString{};
   }

   /**
    * \brief As \c enumToValue, but returns the untranslated text, for callers that need to store it and translate it
    *        later (eg so that a widget can redraw itself in a new language).
    *
    * \throw std::out_of_range if no string is found for the supplied enum (because this is a coding error)
    */
   template<typename E>
   [[nodiscard]] TranslatableText enumToTranslatableText(E const enumValue) const {
      std::optional<TranslatableText> result = this->enumAsIntToTranslatableText(static_cast<int>(enumValue));
      if (!result) {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Coding error: no display name found in context" << this->m_context << "for" <<
            static_cast<int>(enumValue) << Logging::getStackTrace();
         throw std::out_of_range("Missing value in EnumDisplayNameMapping");
      }
      return *result;
   }

private:
   char const * const        m_context;
   EnumMapping<char const *> m_sourceTexts;
};

#endif