/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonMeasureableUnitsMapping.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "serialization/json/JsonMeasureableUnitsMapping.h"

#include <stdexcept>

#include <boost/algorithm/string/predicate.hpp>

#include <QDebug>

JsonMeasureableUnitsMapping::JsonMeasureableUnitsMapping(std::initializer_list<decltype(nameToUnit)::value_type> init,
                                                         JsonXPath const unitField,
                                                         JsonXPath const valueField) :
   nameToUnit{init},
   unitField{unitField},
   valueField{valueField} {
   return;
}

JsonMeasureableUnitsMapping::~JsonMeasureableUnitsMapping() = default;

std::string_view JsonMeasureableUnitsMapping::getNameForUnit(Measurement::Unit const & unitToMatch) const {
   // We could use std::find_if here with a lambda, but a loop with structured bindings is more concise in this instance
   for (auto const & [unitName, unit] : this->nameToUnit) {
      if (unit == &unitToMatch) {
         return unitName;
      }
   }

   // It's almost certainly a coding error if we get here - because we should always have a mapping for a Unit we use.
   qCritical() <<
      Q_FUNC_INFO << "No name found for Unit" << unitToMatch << "while searching mapping for" <<
      this->getPhysicalQuantity();
   throw std::invalid_argument("Unit not found in JsonMeasureableUnitsMapping");
}

Measurement::PhysicalQuantity JsonMeasureableUnitsMapping::getPhysicalQuantity() const {
   // We assume that each mapping only holds Units corresponding to one PhysicalQuantity, so it suffices to return the
   // PhysicalQuantity of the first element in the map`
   return this->nameToUnit.begin()->second->getPhysicalQuantity();
}

bool JsonMeasureableUnitsMapping::containsUnit(std::string_view const unitName, MatchType const matchType) const {

   if (matchType == JsonMeasureableUnitsMapping::MatchType::CaseSensitive) {
      return this->nameToUnit.contains(unitName);
   }

   Q_ASSERT(matchType == JsonMeasureableUnitsMapping::MatchType::CaseInsensitive);

   //
   // In theory this is somewhat inefficient.  In practice, the size of the collections we are searching is so small
   // (just a handful of entries) that it doesn't matter.
   //
   for (auto ii = this->nameToUnit.cbegin(); ii != this->nameToUnit.cend(); ++ii) {
      if (boost::iequals(ii->first, unitName)) {
         return true;
      }
   }
   return false;

}

Measurement::Unit const * JsonMeasureableUnitsMapping::findUnit(std::string_view const unitName,
                                                                MatchType const matchType) const {

   if (matchType == JsonMeasureableUnitsMapping::MatchType::CaseSensitive) {
      return this->nameToUnit.find(unitName)->second;
   }

   Q_ASSERT(matchType == JsonMeasureableUnitsMapping::MatchType::CaseInsensitive);

   //
   // See comment above in containsUnit() about efficiency.
   //
   for (auto ii = this->nameToUnit.cbegin(); ii != this->nameToUnit.cend(); ++ii) {
      if (boost::iequals(ii->first, unitName)) {
         return ii->second;
      }
   }
   return nullptr;
}

Measurement::Unit const * JsonMeasureableUnitsMapping::defaultUnit() const {
   return this->nameToUnit.cbegin()->second;
}

template<class S>
S & JsonMeasureableUnitsMapping::writeToStream(S & stream) const {
   stream <<
      "Unit Field:" << this->unitField << "; Value Field:" << this->valueField << "; Map:\n";
   for (auto const & ii : this->nameToUnit) {
      stream << QString::fromStdString(std::string{ii.first}) << "->" << *ii.second << "\n";
   }
   return stream;
}

template<class S>
S & operator<<(S & stream, JsonMeasureableUnitsMapping const & jmum) {
   return jmum.writeToStream(stream);
}

template<class S>
S & operator<<(S & stream, JsonMeasureableUnitsMapping const * jmum) {
   if (jmum) {
      stream << *jmum;
   } else {
      stream << "NULL";
   }
   return stream;
}

//
// Instantiate the above template functions for the types that are going to use them
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template QDebug &      operator<<(QDebug &      stream, JsonMeasureableUnitsMapping const & jmum);
template QTextStream & operator<<(QTextStream & stream, JsonMeasureableUnitsMapping const & jmum);
template QDebug &      operator<<(QDebug &      stream, JsonMeasureableUnitsMapping const * jmum);
template QTextStream & operator<<(QTextStream & stream, JsonMeasureableUnitsMapping const * jmum);
