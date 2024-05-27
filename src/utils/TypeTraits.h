/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/TypeTraits.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef UTILS_TYPETRAITS_H
#define UTILS_TYPETRAITS_H
#pragma once

#include <memory>
#include <optional>
#include <type_traits>

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
 *        Mostly we shouldn't need to use these templates directly outside of the \c TypeLookup class because the
 *        \c PROPERTY_TYPE_LOOKUP_ENTRY macro in \c utils/TypeLookup.h takes care of everything for constructor calls
 *        where you would otherwise need them.  However, they are sometimes useful for, eg, declaring templated
 *        functions where we need different versions for optional and non-optional, such as \c SmartField::setAmount.
 */
template <typename T> struct is_optional : public std::false_type{};
template <typename T> struct is_optional<std::optional<T>> : public std::true_type{};

template <typename T> struct is_non_optional : public std::true_type{};
template <typename T> struct is_non_optional<std::optional<T>> : public std::false_type{};

template <typename T> struct is_optional_enum : public std::false_type{};
template <typename T> struct is_optional_enum<std::optional<T>> : public std::is_enum<T>{};

// This is not strictly needed, as it duplicates std::is_enum, but it helps document intent
template <typename T> struct is_non_optional_enum : public std::is_enum<T>{};

//
// This bit requires C++20 or later.  It makes the specialisations of TypeInfo::construct() below a bit less clunky
// Older versions of GCC (eg as shipped with Ubuntu 20.04 LTS) have a sort of pre-release support for concepts so we
// have to use non-standard syntax there.
//
// Rather than define every concept twice - once for old compilers and once for new ones, we use the preprocessor to
// add in the extra word ("bool") if and only if we're using an old compiler.
//
#if defined(__linux__ ) && defined(__GNUC__) && (__GNUC__ < 10)
#define CONCEPT_FIX_UP bool
#else
#define CONCEPT_FIX_UP
#endif

template <typename T> concept CONCEPT_FIX_UP IsOptional      = is_optional<T>::value;
template <typename T> concept CONCEPT_FIX_UP IsRequiredEnum  = std::is_enum<T>::value;
template <typename T> concept CONCEPT_FIX_UP IsRequiredOther = !std::is_enum<T>::value && !is_optional<T>::value;
template <typename T> concept CONCEPT_FIX_UP IsOptionalEnum  = is_optional_enum<T>::value;
template <typename T> concept CONCEPT_FIX_UP IsOptionalOther = !is_optional_enum<T>::value && is_optional<T>::value;

//
// In a similar vein it's useful at compile time to be able to know if something is a raw pointer or a shared pointer.
//
// Already std::is_pointer tells us whether something is a raw pointer, so we just need the other case.
//
template<typename T> struct is_shared_ptr : public std::false_type{};
template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : public std::true_type{};

template <typename T> concept CONCEPT_FIX_UP IsRawPointer    = std::is_pointer<T>::value;
template <typename T> concept CONCEPT_FIX_UP IsSharedPointer = is_shared_ptr  <T>::value;

#endif
