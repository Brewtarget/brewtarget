/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/TypeLookup.cpp is part of Brewtarget, and is copyright the following authors 2023:
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
#include "utils/TypeLookup.h"

#include <typeinfo>
#include <type_traits>

#include <QDate>
#include <QDebug>
#include <QString>

#include "Logging.h"
#include "measurement/Amount.h"
#include "utils/BtStringConst.h"

namespace PropertyNames::None {
   BtStringConst const none{"none"};
}

bool TypeInfo::isEnum() const {
   if (this->classification == TypeInfo::Classification::RequiredEnum ||
       this->classification == TypeInfo::Classification::OptionalEnum) {
      return true;
   }
   return false;
}

bool TypeInfo::isOptional() const {
   if (this->classification == TypeInfo::Classification::OptionalEnum ||
       this->classification == TypeInfo::Classification::OptionalOther) {
      return true;
   }
   return false;
}

TypeLookup::TypeLookup(char       const * const                                 className,
                       std::initializer_list<TypeLookup::LookupMap::value_type> initializerList,
                       std::initializer_list<TypeLookup const *>                parentClassLookups) :
   m_className{className},
   m_lookupMap{initializerList},
   m_parentClassLookups{parentClassLookups} {
   return;
}

TypeInfo const * TypeLookup::typeInfoFor(BtStringConst const & propertyName) const {
   // Normally keep this log statement commented out otherwise it generates too many lines in the log file
//   qDebug() << Q_FUNC_INFO << this << "Searching for" << *propertyName;
   auto match = std::find_if(
      this->m_lookupMap.begin(),
      this->m_lookupMap.end(),
      [& propertyName](auto const & record) { return propertyName == *record.first; }
   );

   if (match != this->m_lookupMap.end()) {
      return &match->second;
   }

   for (auto parentClassLookup : this->m_parentClassLookups) {
      auto result = parentClassLookup->typeInfoFor(propertyName);
      if (result) {
         return result;
      }
   }

   return nullptr;
}

TypeInfo const & TypeLookup::getType(BtStringConst const & propertyName) const {
   auto result = this->typeInfoFor(propertyName);
   if (result) {
      return *result;
   }

   // It's a coding error if we tried to look up a property that we don't know about
   qCritical() <<
      Q_FUNC_INFO << "Can't find type info for property \"" << *propertyName << "\" of class" << this->m_className;
   qCritical().noquote() << Q_FUNC_INFO << "Stack trace:" << Logging::getStackTrace();
   Q_ASSERT(false);
   throw std::bad_typeid();
}
