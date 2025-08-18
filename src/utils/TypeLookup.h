/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/TypeLookup.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef UTILS_TYPELOOKUP_H
#define UTILS_TYPELOOKUP_H
#pragma once

#include <concepts>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <vector>

#include "measurement/QuantityFieldType.h"
#include "utils/BtStringConst.h"
#include "utils/OptionalHelpers.h"
#include "utils/TypeInfo.h"
#include "utils/TypeTraits.h"

class NamedEntity;

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

   template<class S> friend S & operator<<(S & stream, TypeLookup const & typeLookup);
   template<class S> friend S & operator<<(S & stream, TypeLookup const * typeLookup);

public:

   /**
    * \brief If we want to change from using std::map in future, it's easier if we have a typedef alias for it
    */
   using LookupMap = std::map<BtStringConst const *, TypeInfo>;

   /**
    * \brief Construct a \c TypeLookup that optionally extends an existing one (typically from the parent class)
    *
    * \param className Name of the class for which this is the property type lookup.  Eg for the \c TypeLookup for
    *                  \c Hop, this should be "Hop".  Used for error logging.
    * \param initializerList The mappings for this \c TypeLookup.  Eg for the \c TypeLookup for \c Hop, this would be
    *                        the type mappings for PropertyNames::Hop::... properties (but not the
    *                        PropertyNames::NamedEntity::... properties
    * \param parentClassLookup Pointer(s) to the \c TypeLookup for the parent class(es), or \c {} if there is none.
    *                          Usually there is a single pointer; multiple pointers are only needed to handle multiple
    *                          inheritance.
    *
    *                          Eg for the \c TypeLookup for \c Hop, this should point to the \c TypeLookup for
    *                          \c Ingredient because \c Hop inherits directly from \c Ingredient.  (And, in turn the
    *                          \c parentClassLookup of the \c TypeLookup for \c Ingredient should point to
    *                          \c NamedEntity.)
    */
   TypeLookup(char       const * const                     className,
              std::initializer_list<LookupMap::value_type> initializerList,
              std::initializer_list<TypeLookup const *>    parentClassLookups = {});

   /**
    * \brief Get the type info for a given property name
    *
    *        NOTE that property names are not globally unique, hence why each model class has its own \c TypeLookup
    *             static member.
    */
   TypeInfo const & getType(BtStringConst const & propertyName) const;

private:
   /**
    * \brief Used by \c getType when doing parent class lookup
    */
   TypeInfo const * typeInfoFor(BtStringConst const & propertyName) const;

   char const * const m_className;
   LookupMap const m_lookupMap;
   std::vector<TypeLookup const *> const m_parentClassLookups;
};

/**
 * \brief This is an additional concept for determining whether a class has a `static TypeLookup const typeLookup`
 *        member.
 */
template <typename T> concept HasTypeLookup = (std::is_base_of_v<NamedEntity, T> &&
                                               std::same_as<decltype(T::typeLookup), TypeLookup const>);
//
// Default assumption is that a type (eg int) doesn't have its own typeLookup function.  Then we override this for types
// (such as our own classes inheriting from NamedEntity) that do.  We have to handle all the different types of pointers
// here.
//
template<typename      T> struct TypeLookupOf                     : std::integral_constant<TypeLookup const *, nullptr       > {};
template<HasTypeLookup T> struct TypeLookupOf<T>                  : std::integral_constant<TypeLookup const *, &T::typeLookup> {};
template<HasTypeLookup T> struct TypeLookupOf<T *>                : std::integral_constant<TypeLookup const *, &T::typeLookup> {};
template<HasTypeLookup T> struct TypeLookupOf<std::shared_ptr<T>> : std::integral_constant<TypeLookup const *, &T::typeLookup> {};
template<HasTypeLookup T> struct TypeLookupOf<std::unique_ptr<T>> : std::integral_constant<TypeLookup const *, &T::typeLookup> {};

/**
 * \brief This macro simplifies the entries in the \c initializerList parameter of a \c TypeLookup constructor call.  It
 *        also makes it easier for us to modify the structure of \c LookupMapEntry or \c LookupMap in future if we need
 *        to.
 *
 *        For the purposes of calling the \c TypeLookup constructor, the caller doesn't have to worry about what we
 *        are storing or how.  For each property, you just provide the name of the property, the member variable in
 *        which it is stored, and, if appropriate, the QuantityFieldType for the property eg:
 *           PROPERTY_TYPE_LOOKUP_ENTRY(Hop, notes    , m_notes                                     ),
 *           PROPERTY_TYPE_LOOKUP_ENTRY(Hop, alpha_pct, m_alpha_pct, NonPhysicalQuantity::Percentage),
 *           PROPERTY_TYPE_LOOKUP_ENTRY(Hop, amount_kg, m_amount_kg, Measurement::Mass              ),
 *        The macro and the templates above etc then do the necessary.
 *
 *        Note that the introduction of __VA_OPT__ in C++20 makes dealing with the optional third argument a LOT less
 *        painful than it would otherwise be!
 */
#define PROPERTY_TYPE_LOOKUP_ENTRY(className, propName, memberVar, ...) \
   {&PropertyNames::className::propName, \
    TypeInfo::construct<decltype(className::memberVar)>(PropertyNames::className::propName, \
                                                        className::localisedName_##propName, \
                                                        TypeLookupOf<decltype(className::memberVar)>::value \
                                                        __VA_OPT__ (, __VA_ARGS__))}

/**
 * \brief This is a trick to allow us to get the return type of a pointer to a member function with a similar syntax
 *        to the way we get it for a member variable.
 *
 *        See https://stackoverflow.com/questions/76325552/c-get-return-type-of-a-pointer-to-a-member-function
 */
//! @{
template<typename MembFnPtr> struct MemberFunctionReturnType;
template<typename Ret, typename Obj, typename... Args> struct MemberFunctionReturnType<Ret(Obj::*)(Args...)> {
   using type = Ret;
};
template<typename Ret, typename Obj, typename... Args> struct MemberFunctionReturnType<Ret(Obj::*)(Args...) const> {
   using type = Ret;
};
template<auto MembFnPtr> using MemberFunctionReturnType_t = typename MemberFunctionReturnType<decltype(MembFnPtr)>::type;
//! @}

/**
 * \brief Using the same trick as \c MemberFunctionReturnType_t, we can, with a bit of extra work, also get the type of
 *        the first parameter to a member function.
 */
//! @{
template<typename First, typename...> struct FirstTypeInPack {
   using type = First;
};
template<typename MembFnPtr> struct MemberFunctionFirstParamType;
template<typename Ret, class Obj, typename... Args> struct MemberFunctionFirstParamType<Ret (Obj::*)(Args...)> {
   using type = typename FirstTypeInPack<Args...>::type;
};
template<auto MembFnPtr> using MemberFunctionFirstParamType_t = typename MemberFunctionFirstParamType<decltype(MembFnPtr)>::type;
//! @}

/**
 * \brief Similar to \c PROPERTY_TYPE_LOOKUP_ENTRY but used when we do not have a member variable and instead must use
 *        the return value of a getter member function.  This is usually when we have some combo getters/setters that
 *        exist primarily for the benefit of BeerJSON.  Eg, The \c Fermentable::betaGlucanWithUnits member function
 *        combines \c Fermentable::m_betaGlucan and \c Fermentable::betaGlucanIsMassPerVolume into a
 *        \c std::optional<MassOrVolumeConcentrationAmt> return value, so, in \c Fermentable::typeLookup, we include the
 *        following:
 *
 *           PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(Fermentable, betaGlucanWithUnits, betaGlucanWithUnits, Measurement::PqEitherMassOrVolumeConcentration),
 *
 *        It would be neat to include the following in the macro:
 *           static_assert(std::is_member_function_pointer_v<decltype(&getterMemberFunction)>)
 *        However, because the macro is designed to be used inside an initialiser list, we can't.
 *
 *        TBD: We could go further and combine \c propName and \c getter, since they are always the same, but I prefer
 *             to keep the syntax similar to \c PROPERTY_TYPE_LOOKUP_ENTRY
 */
#define PROPERTY_TYPE_LOOKUP_NO_MV(className, propName, getter, ...) \
   {&PropertyNames::className::propName, \
    TypeInfo::construct<MemberFunctionReturnType_t<&className::getter>>(PropertyNames::className::propName, \
                                                                        className::localisedName_##propName, \
                                                                        TypeLookupOf<MemberFunctionReturnType_t<&className::getter>>::value \
                                                                        __VA_OPT__ (, __VA_ARGS__))}

/**
 * \brief Convenience functions for logging
 */
//! @{
template<class S>
S & operator<<(S & stream, TypeLookup const & typeLookup) {
   stream << "TypeLookup for " << typeLookup.m_className;
   return stream;
}

template<class S>
S & operator<<(S & stream, TypeLookup const * typeLookup) {
   if (!typeLookup) {
      stream << "nullptr";
   } else {
      stream << *typeLookup;
   }
   return stream;
}

//! @}


#endif
