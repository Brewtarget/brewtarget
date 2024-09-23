/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/TypeLookup.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <vector>

#include "BtFieldType.h"
#include "model/NamedEntityCasters.h"
#include "utils/BtStringConst.h"
#include "utils/OptionalHelpers.h"
#include "utils/TypeTraits.h"

class BtStringConst;
class NamedEntity;

namespace PropertyNames::None {
   extern BtStringConst const none;
}

class TypeLookup;

/**
 * \brief Extends \c std::type_index with some other info we need about a type for serialisation, specifically whether
 *        it is an enum and/or whether it is \c std::optional.
 */
struct TypeInfo {
   /**
    * \brief This is the type ID of the \b underlying type, eg should be the same for \c int and \c std::optional<int>.
    *
    *        \c std::type_index is essentially a wrapper around pointer to \c std::type_info.  It is guaranteed unique
    *        for each different type and guaranteed to compare equal for two properties of the same type.  (This is
    *        better than using raw pointers as they are not guaranteed to be identical for two properties of the same
    *        type.)
    *
    *        Note that we cannot use \c std::type_info::name() for this purpose as "the returned string can be identical
    *        for several types".
    */
   std::type_index typeIndex;

   /**
    * \brief Templated factory function strips out the `std::optional` wrapper
    */
   template<typename   T> static std::type_index makeTypeIndex() {return typeid(T                     ); }
   template<IsOptional T> static std::type_index makeTypeIndex() {return typeid(typename T::value_type); }

   /**
    * \brief This classification covers the main special cases we need to deal with, viz whether a property is optional
    *        (so we have to deal with std::optional wrapper around the underlying type) and whether it is an enum (where
    *        we treat it as an int for generic handling because it makes the serialisation code a lot simpler).
    */
   enum class Classification {
      RequiredEnum ,
      RequiredOther,
      OptionalEnum ,
      OptionalOther,
   };
   Classification classification;
   template<typename        T> static Classification makeClassification(); // Only specialisations
   template<IsRequiredEnum  T> static Classification makeClassification() {return Classification::RequiredEnum ;}
   template<IsRequiredOther T> static Classification makeClassification() {return Classification::RequiredOther;}
   template<IsOptionalEnum  T> static Classification makeClassification() {return Classification::OptionalEnum ;}
   template<IsOptionalOther T> static Classification makeClassification() {return Classification::OptionalOther;}

   /**
    * \brief For fields that are pointers, we need to be able to distinguish the type of pointer.
    */
   enum class PointerType {
      NotPointer   ,
      RawPointer   ,
      SharedPointer,
   };
   template<typename        T> static PointerType makePointerType() {return PointerType::NotPointer   ;}
   template<IsRawPointer    T> static PointerType makePointerType() {return PointerType::RawPointer   ;}
   template<IsSharedPointer T> static PointerType makePointerType() {return PointerType::SharedPointer;}
   PointerType pointerType;

   /**
    * \brief If and only if \c pointerType is PointerType::SharedPointer, this holds the function pointers necessary to
    *        cast to and from equivalent pointers to \c NamedEntity.
    *
    *        In the templated factory function \c makeCasters, we have T is std::shared_ptr<NE> for some class NE, a
    *        subclass of \c NamedEntity, so we need T::element_type to obtain NE to pass to
    *        \c NamedEntityCasters::construct.
    *
    *        NOTE: We are taking a bit of a shortcut here in terms of assuming that any shared pointer is a shared
    *              pointer to a subclass of \c NamedEntity.  I'm pretty sure this is valid at the moment, but if need
    *              something more nuanced in future then we'd need to extend things slightly here.
    */
   std::optional<NamedEntityCasters> namedEntityCasters;
   template<typename        T> static std::optional<NamedEntityCasters> makeCasters() {return std::nullopt;}
   template<IsSharedPointer T> static std::optional<NamedEntityCasters> makeCasters() {return NamedEntityCasters::construct<typename T::element_type>();}

   /**
    * \brief If the type is a subclass of \c NamedEntity (or a raw or smart pointer to one) then this will point to the
    *        \c TypeLookup for that class.  This is used in \c PropertyPath.  Otherwise this will hold \c nullptr.
    */
   TypeLookup const * typeLookup;

   /**
    * \brief Where appropriate, this tells us what is actually being stored.  Eg, \c typeIndex might tells us that a
    *        field is a \c double and \c classification indicates whether it is wrapped in \c std::optional, but this
    *        is what we need to determine whether it is storing \c PhysicalQuantity::Mass (in kilograms) or
    *        \c PhysicalQuantity::Temperature (in Celsius) or \c NonPhysicalQuantity::Percentage, etc.
    *
    *        This is only set for fields where it could have a meaning, eg we wouldn't set it for a foreign key field.
    *
    *        Although we _could_ do some clever stuff to automatically deduce the value of this field in certain cases
    *        (eg for a \c bool type, this is probably \c NonPhysicalQuantity::Bool, for a \c QString type, this is
    *        probably \c NonPhysicalQuantity::String, etc), I have deliberately not done so for these reasons:
    *           - Having a value set here shows this is a property that we want to expose to the user.  Where a property
    *             is for internal use only (but nonetheless stored in the DB etc), then this field should be
    *             \c std::nullopt
    *           - Things that we think can be deduced now might not always remain so.  Eg, at a future date, it is at
    *             least conceivable that there might be some new \c NonPhysicalQuantity that we also want to store in a
    *             \c QString
    *           - Adding all the deduction logic here makes this code more complicated (and thus more liable to bugs)
    *             but only saves us a small amount in each 'static TypeLookup const typeLookup' definition.
    */
   std::optional<BtFieldType> fieldType;

   /**
    * \brief Sometimes it's useful to be able to get the property name from the \c TypeInfo object.  NOTE that there are
    *        valid circumstances where this will be \c PropertyNames::None::none
    */
   BtStringConst const & propertyName;

   /**
    * \return \c true if \c classification is \c RequiredEnum or \c OptionalEnum, \c false otherwise (ie if
    *         \c classification is \c RequiredOther or \c OptionalOther
    */
   bool isEnum() const;

   /**
    * \return \c true if \c classification is \c OptionalEnum or \c OptionalOther, \c false otherwise (ie if
    *         \c classification is \c RequiredEnum or \c RequiredOther
    */
   bool isOptional() const;

   /**
    * \brief Factory functions to construct a \c TypeInfo for a given type.
    *
    *        Note that if \c T is \c std::optional<U> then U can be extracted by \c typename \c T::value_type.
    */
   template<typename T> const static TypeInfo construct(BtStringConst const & propertyName,
                                                        TypeLookup const * typeLookup,
                                                        std::optional<BtFieldType> fieldType = std::nullopt) {
      return TypeInfo{makeTypeIndex<T>(),
                      makeClassification<T>(),
                      makePointerType<T>(),
                      makeCasters<T>(),
                      typeLookup,
                      fieldType,
                      propertyName};
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
    * \brief Get the type ID (and whether it's an enum) for a given property name
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
 *        which it is stored, and, if appropriate, the BtFieldType for the property eg:
 *           PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::notes    , Hop::m_notes                                     ),
 *           PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::alpha_pct, Hop::m_alpha_pct, NonPhysicalQuantity::Percentage),
 *           PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::amount_kg, Hop::m_amount_kg, Measurement::Mass              ),
 *        The macro and the templates above etc then do the necessary.
 *
 *        Note that the introduction of __VA_OPT__ in C++20 makes dealing with the optional third argument a LOT less
 *        painful than it would otherwise be!
 */
#define PROPERTY_TYPE_LOOKUP_ENTRY(propNameConstVar, memberVar, ...) \
   {&propNameConstVar, TypeInfo::construct<decltype(memberVar)>(propNameConstVar, TypeLookupOf<decltype(memberVar)>::value __VA_OPT__ (, __VA_ARGS__))}

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
 *           PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Fermentable::betaGlucanWithUnits, Fermentable::betaGlucanWithUnits, Measurement::PqEitherMassOrVolumeConcentration),
 *
 *        It would be neat to include the following in the macro:
 *           static_assert(std::is_member_function_pointer_v<decltype(&getterMemberFunction)>)
 *        However, because the macro is designed to be used inside an initialiser list, we can't.
 */
#define PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(propNameConstVar, getterMemberFunction, ...) \
   {&propNameConstVar, TypeInfo::construct<MemberFunctionReturnType_t<&getterMemberFunction>>(propNameConstVar, TypeLookupOf<MemberFunctionReturnType_t<&getterMemberFunction>>::value __VA_OPT__ (, __VA_ARGS__))}


/**
 * \brief Convenience functions for logging
 */
//! @{
template<class S>
S & operator<<(S & stream, TypeInfo const & typeInfo) {
   stream <<
      "«TypeInfo " << (typeInfo.isOptional() ? "optional" : "non-optional") << " \"" << typeInfo.typeIndex.name() <<
      "\"; fieldType:" << typeInfo.fieldType << "; property name:" << *typeInfo.propertyName << "; typeLookup:" <<
      typeInfo.typeLookup << "»";
   return stream;
}

template<class S>
S & operator<<(S & stream, TypeInfo const * typeInfo) {
   if (!typeInfo) {
      stream << "nullptr";
   } else {
      stream << *typeInfo;
   }
   return stream;
}

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
