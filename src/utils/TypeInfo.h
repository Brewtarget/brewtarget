/*======================================================================================================================
 * utils/TypeInfo.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef UTILS_TYPEINFO_H
#define UTILS_TYPEINFO_H
#pragma once

#include <optional>
#include <typeindex>
#include <variant>

#include "measurement/QuantityFieldType.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/Unit.h"
#include "model/NamedEntityCasters.h"
#include "utils/TypeTraits.h"

class BtStringConst;

namespace PropertyNames::None {
   extern BtStringConst const none;
}

namespace DisplayInfo {
   /**
    * \brief Extra info stored in \c TypeInfo (see below) for enum types
    */
   struct Enum {
      /**
       * \brief Values to store in combo box
       */
      EnumStringMapping const & stringMapping;

      /**
       * \brief Localised display names to show on combo box
       */
      EnumStringMapping const & displayNames;
   };

   /**
    * \brief Extra info stored in \c TypeInfo (see below) for boolean types
    *
    *        For most boolean types, we show a combo box (eg "Not mashed" / "Mashed"; "Weight" / "Volume")
    *
    *        These are QString rather than reference to QString as typically initialised with an rvalue (the result of
    *        calling \c QObject::tr).
    */
   struct Bool {
      QString const unsetDisplay;
      QString const setDisplay;
   };

   /**
    * \brief I know we don't need a struct for this, but it's more consistent to use one
    */
   struct Precision {
      unsigned int const precision;
   };

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
    * \brief Pointer to a static member function that returns the localised name of this property, suitable for display
    *        to the user.  For PropertyNames::SomeClass::someProperty, this is usually SomeClass::localisedName_someProperty
    */
   QString (&localisedName) ();

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
   std::optional<QuantityFieldType> fieldType;

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
    * \brief Extra info for how to display this property
    */
   using DisplayAs = std::optional<std::variant<DisplayInfo::Enum,
                                                DisplayInfo::Bool,
                                                DisplayInfo::Precision>>;
   TypeInfo::DisplayAs displayAs;

   /**
    * \brief For a small minority of properties that hold a \c PhysicalQuantity in non-canonical units (eg
    *        \c StepBase::stepTime_days), this field holds what those units are.
    *
    *        NB for \c NonPhysicalQuantity and for \c PhysicalQuantity in canonical units, this field will be null.
    *
    *        (TBD We could consider storing unit here also for canonical units, via calling
    *         Measurement::Unit::getCanonicalUnit.)
    */
   Measurement::Unit const * unit;

   /**
    * \brief Factory functions to construct a \c TypeInfo for a given type.
    *
    *        Note that if \c T is \c std::optional<U> then U can be extracted by \c typename \c T::value_type.
    */
   template<typename T> const static TypeInfo construct(BtStringConst const & propertyName,
                                                        QString (&localisedNameFunction) (),
                                                        TypeLookup const * typeLookup,
                                                        std::optional<QuantityFieldType> fieldType = std::nullopt,
                                                        DisplayAs displayAs = std::nullopt,
                                                        Measurement::Unit const * unit = nullptr) {
      return TypeInfo{makeTypeIndex<T>(),
                      makeClassification<T>(),
                      makePointerType<T>(),
                      makeCasters<T>(),
                      localisedNameFunction,
                      typeLookup,
                      fieldType,
                      propertyName,
                      displayAs,
                      unit};
   }
};


/**
 * \brief Convenience functions for logging
 */
//! @{
template<class S>
S & operator<<(S & stream, TypeInfo const & typeInfo) {
   stream <<
      "««« TypeInfo " << (typeInfo.isOptional() ? "optional" : "non-optional") << " \"" << typeInfo.typeIndex.name() <<
      "\"; fieldType:" << typeInfo.fieldType << "; property name:" << *typeInfo.propertyName << "; typeLookup:" <<
      typeInfo.typeLookup << "»»»";
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
//! @}

/**
 * \brief This macro makes it easier to initialise \c fieldType and \c displayAs in \c TypeInfo constructor for enum
 *        fields in classes where we follow our standard naming convention for enum string mappings and display names.
 *        Eg, instead of writing:
 *
 *           ... NonPhysicalQuantity::Enum, DisplayInfo::Enum{BoilStep::chillingTypeStringMapping,
 *                                                            BoilStep::chillingTypeDisplayNames} ...
 *
 *        we can write:
 *
 *           ... ENUM_INFO(BoilStep::chillingType) ...
 */
#define ENUM_INFO(enumType) NonPhysicalQuantity::Enum, DisplayInfo::Enum{enumType##StringMapping, enumType##DisplayNames}

/**
 * \brief This macro makes it easier to initialise \c fieldType and \c displayAs in \c TypeInfo constructor for boolean
 *        fields.  Eg, instead of writing:
 *
 *           ... NonPhysicalQuantity::Bool, DisplayInfo::Bool{{tr("No"), tr("Yes")}} ...
 *
 *        we can write:
 *
 *           ... BOOL_INFO(tr("No"), tr("Yes")) ...
 */
#define BOOL_INFO(offText, onText) NonPhysicalQuantity::Bool, DisplayInfo::Bool{offText, onText}

#endif
