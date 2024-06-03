/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/PropertyPath.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef UTILS_PROPERTYPATH_H
#define UTILS_PROPERTYPATH_H
#pragma once

#include <functional>
#include <initializer_list>

#include <QString>
#include <QVector>

#include "utils/BtStringConst.h"
#include "utils/TypeLookup.h"

class NamedEntity;

/**
 * \brief In our (de)serialisation code (to and from database, BeerXML, BeerJSON), we mostly, but not always, have a
 *        one-to-one relationship between "object in our data model" and "record in the format we are serailising
 *        to/from".  Where such a 1-1 mapping is not possible, we can often bridget the gap with XPaths and XPath-like
 *        identifiers.  Eg, a \c JsonXPath allows a property on a model object to map to a field on a sub-record of
 *        the corresponding BeerJSON record.
 *
 *        This \c PropertyPath class allows us to do the reverse: have a field on a BeerJSON or BeerXML record map to a
 *        property of a "sub-object" of the corresponding model object.
 *
 *        Eg, in a BeerXML RECIPE record, the BOIL_SIZE and BOIL_TIME fields need (now) to map to the \c preBoilSize_l
 *        and \c boilTime_mins properties on the \c Boil object contained/referenced in the \c boil property of a
 *        \c Recipe object.  In an XPath we would express this as \c boil/preBoilSize_l and \c boil/boilTime_mins.
 *        Here, because we use constants for our property names, rather than join, eg, the raw strings "boil" and
 *        "boilTime_mins", we actually want to refer to \c PropertyNames::Recipe::boil and
 *        \c PropertyNames::Boil::boilTime_mins, so it's more natural to do that as:
 *
 *           {PropertyNames::Recipe::boil, PropertyNames::Boil::boilTime_mins}
 */
class PropertyPath {
public:
   /**
    * \brief Most of the time, a \c PropertyPath is that same as a single QProperty name, which we code in a
    *        \c BtStringConst.
    */
   PropertyPath(BtStringConst const & singleProperty);

   /**
    * \brief When we need a non-trivial path, we want a brace-enclosed list of \c BtStringConst QProperty names
    *
    *        Using an \c initializer_list of \c reference_wrapper just saves the caller from having to prefix everything
    *        with '&'.
    */
   PropertyPath(std::initializer_list<std::reference_wrapper<BtStringConst const>> listOfProperties);

   PropertyPath(PropertyPath const & other);

   /**
    * \brief Since we have a user-defined copy constructor, the rule of three says we need a user-defined copy
    *        assignment operator.  This does get used, eg when we're copying XmlRecordDefinition::FieldDefinition
    *        records into a vector to allow us to share such records between two XmlRecordDefinition instances.
    */
   PropertyPath & operator=(PropertyPath const & other);

   ~PropertyPath();

   QString asXPath() const;

   QVector<BtStringConst const *> const & properties() const;

   /**
    * \brief An empty or null \c PropertyPath is actually represented by a single property of value
    *        \c BtString::NULL_STR.  Such a path will return \c true from this function, otherwise \c false is returned.
    */
   bool isNull() const;

   /**
    * \brief Suppose we have a \c PropertyPath of `{PropertyNames::Recipe::boil, PropertyNames::Boil::boilTime_mins}` on
    *        class \c Recipe.  If we want to get its \c TypeInfo, it is not directly available from
    *        \c Recipe::typeLookup.  Instead, we must look at the \c boil property on \c Recipe to get the \c TypeLookup
    *        for the \c Boil class, from which we can then obtain the \c TypeInfo for the \c boilTime_mins property.
    *
    *        This function automates that.  You just give it the \c TypeLookup for the class you're starting from and it
    *        returns the right \c TypeInfo this \c PropertyPath.  Eg, in the example above, the returned value is the
    *        same as calling `Boil::typeLookup.getType(PropertyNames::Boil::boilTime_mins)`.
    */
   TypeInfo const & getTypeInfo(TypeLookup const & baseTypeLookup) const;

   /**
    * \brief Applies this path to \c namedEntity to then call the relevant setter with \c val
    *
    *        Note, if necessary, \c namedEntity.ensureExists() will be called to create contained objects.  (This is why
    *        we can't just take \c QObject as a parameter.)
    *
    *        It is, as ever, caller's responsibility to supply valid base object and valid property path.
    *
    * \return \c true if the property was writeable, \c false if not (and therefore the setter could not be called)
    */
   [[nodiscard]] bool setValue(NamedEntity & obj, QVariant const & val) const;

   /**
    * \brief Counterpart to \c setValue
    */
   QVariant getValue(NamedEntity const & obj) const;

private:
   //! \brief The list of properties in this path
   QVector<BtStringConst const *> m_properties;

   //! \brief The string representation this path (mostly for logging)
   QString m_path;
};

/**
 * \brief Convenience functions for logging
 */
//! @{
template<class S>
S & operator<<(S & stream, PropertyPath const & propertyPath) {
   stream << propertyPath.asXPath();
   return stream;
}
template<class S>
S & operator<<(S & stream, PropertyPath const * propertyPath) {
   if (propertyPath) {
      stream << propertyPath->asXPath();
   } else {
      stream << "nullptr";
   }
   return stream;
}
//! @}

#endif
