/*
 * utils/TypeLookup.h is part of Brewtarget, and is Copyright the following
 * authors 2023
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef UTILS_TYPELOOKUP_H
#define UTILS_TYPELOOKUP_H
#pragma once

#include <map>
#include <optional>
#include <typeindex>
#include <typeinfo>

#include "utils/BtStringConst.h"

/**
 * \brief Together with \c std::is_enum from \c <type_traits>, the \c is_optional and \c is_optional_enum templates
 *        defined here give us a generic way at compile time to determine whether a type T is:
 *           1. an enum
 *           2. an instance of std::optional< U > for some enum U
 *           3. an instance of std::optional< U > for some other type U
 *           4. neither an instance of std::optional nor an enum
 *        The four cases:
 *
 *           --------------------------------------------------------------------------------
 *           |     T               | std::is_enum<T> | is_optional<T> | is_optional_enum<T> |
 *           --------------------------------------------------------------------------------
 *           |                     |                 |                |                     |
 *           | an enum             |      true       |     false      |        false        |
 *           |                     |                 |                |                     |
 *           | other non optional  |      false      |     false      |        false        |
 *           |                     |                 |                |                     |
 *           | std::optional enum  |      false      |     true       |        true         |
 *           |                     |                 |                |                     |
 *           | other std::optional |      false      |     true       |        false        |
 *           |                     |                 |                |                     |
 *           --------------------------------------------------------------------------------
 *
 *        Template metaprogramming is sometimes very useful but can be a bit painful to follow.  What we've done here
 *        (at the simple end of things!) is somewhat inspired by the examples at:
 *        https://www.boost.org/doc/libs/1_81_0/libs/type_traits/doc/html/boost_typetraits/background.html
 *
 *        Normally we shouldn't need to use these templates directly outside of the \c TypeLookup class because the
 *        \c PROPERTY_TYPE_LOOKUP_ENTRY macro below takes care of everything for constructor calls where you would
 *        otherwise need them.
 */
template <typename T>
struct is_optional : public std::false_type{};
template <typename T>
struct is_optional< std::optional<T> > : public std::true_type{};

template <typename T>
struct is_optional_enum : public std::false_type{};
template <typename T>
struct is_optional_enum< std::optional<T> > : public std::is_enum<T>{};

/**
 * \brief Extends \c std::type_index with some other info we need about a type for serialisation, specifically whether
 *        it is an enum and/or whether it is \c std::optional.
 */
struct TypeInfo {
   /**
    * \brief \c std::type_index is essentially a wrapper around pointer to \c std::type_info.  It is guaranteed unique
    *        for each different type and guaranteed to compare equal for two properties of the same type.  (This is
    *        better than using raw pointers as they are not guaranteed to be identical for two properties of the same
    *        type.)
    *
    *        Note that we cannot use \c std::type_info::name() for this purpose as "the returned string can be identical
    *        for several types".
    */
   std::type_index typeIndex;

   /**
    * \brief This classification covers the main special cases we need to deal with, viz whether a property is optional
    *        (so we have to deal with std::optional wrapper around the underlying type) and whether it is an enum (where
    *        we treat it as an int for generic handling because it makes the serialisation code a lot simpler).
    */
   enum class Classification {
      RequiredEnum,
      RequiredOther,
      OptionalEnum,
      OptionalOther
   };
   Classification classification;

   bool isOptional() const;

   /**
    * \brief Factory function to construct a \c TypeInfo for a given type.
    *
    *        TODO: We could probably do this via a bunch of template specialisations...
    */
   template <typename T>
   const static TypeInfo construct() {
      if (std::is_enum<T>::value) {
         return TypeInfo{typeid(T), Classification::RequiredEnum};
      }
      if (!is_optional<T>::value) {
         return TypeInfo{typeid(T), Classification::RequiredOther};
      }
      if (is_optional_enum<T>::value) {
         return TypeInfo{typeid(T), Classification::OptionalEnum};
      }
      return TypeInfo{typeid(T), Classification::OptionalOther};
   }
};

/**
 * \class TypeLookup allows us to get \c TypeInfo for a property.
 *
 *        With the advent of BeerJSON, we have a lot more "optional" fields on objects.  We don't want to extend three
 *        different serialisation models (database, BeerXML and BeerJSON) with an extra flag, especially as the
 *        (subclass of) \c NamedEntity ought to know itself whether a field is optional/nullable.  This is enough for
 *        serialisation (where we just need to know eg whether we're reading/writing `double` or
 *        `std::optional<double>`).
 *
 *        In principle we might be able to avoid the need for this class and instead construct a look-up table at run
 *        time by making a bunch of calls to \c qRegisterMetaType(std::optional<T>) during start-up for all types \c T
 *        and storing the resulting IDs in a set or list that we then consult to discover whether a property is
 *        of type \c T or \c std::optional<T>.  But I _think_ the approach here is easier to debug.
 */
class TypeLookup {
public:

   /**
    * \brief If we want to change from using std::map in future, it's easier if we have a typedef alias for it
    */
   using LookupMap = std::map<BtStringConst const *, TypeInfo>;

   /**
    * \brief Construct a \c TypeLookup that optinoally extends an existing one (typically from the parent class)
    *
    * \param className Name of the class for which this is the property type lookup.  Eg for the \c TypeLookup for
    *                  \c Hop, this should be "Hop".  Used for error logging.
    * \param initializerList The mappings for this \c TypeLookup.  Eg for the \c TypeLookup for \c Hop, this would be
    *                        the type mappings for PropertyNames::Hop::... properties (but not the
    *                        PropertyNames::NamedEntity::... properties
    * \param parentClassLookup Pointer to the \c TypeLookup for the parent class, or \c nullptr if there is none.  Eg
    *                          for the \c TypeLookup for \c Hop,
    *                          this should point to the \c TypeLookup for \c NamedEntity because \c Hop inherits from
    *                          \c NamedEntity.
    */
   TypeLookup(char       const * const                     className,
              std::initializer_list<LookupMap::value_type> initializerList,
              TypeLookup const * const                     parentClassLookup = nullptr);

   /**
    * \brief Get the type ID (and whether it's an enum) for a given property name
    */
   TypeInfo const & getType(BtStringConst const & propertyName) const;

   /**
    * \brief Returns whether the attribute for a given property name is optional (ie std::optional<T> rather than T)
    */
   bool isOptional(BtStringConst const & propertyName) const;

private:
   char       const * const className;
   LookupMap          const lookupMap;
   TypeLookup const * const parentClassLookup;
};

/**
 * \brief This macro simplifies the entries in the \c initializerList parameter of a \c TypeLookup constructor call.  It
 *        also makes it easier for us to modify the structure of \c LookupMapEntry or \c LookupMap in future if we need
 *        to.
 *
 *        For the purposes of calling the \c TypeLookup constructor, the caller doesn't have to worry about what we
 *        are storing or how.  For each property, you just provide the name of the property and the member variable in
 *        which it is stored, eg:
 *           PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::alpha_pct, Hop::m_alpha_pct)
 *        The macro and the templates above etc then do the necessary.
 */
#define PROPERTY_TYPE_LOOKUP_ENTRY(propNameConstVar, memberVar) {&propNameConstVar, TypeInfo::construct<decltype(memberVar)>()}

#endif
