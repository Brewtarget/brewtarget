/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/OptionalHelpers.cpp is part of Brewtarget, and is copyright the following authors 2023:
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
#include "utils/OptionalHelpers.h"

#include <QDebug>

#include "Logging.h"
#include "utils/MetaTypes.h"
#include "utils/TypeLookup.h"

void Optional::removeOptionalWrapper(QVariant & propertyValue, TypeInfo const & typeInfo, bool * hasValue) {
   // It's a coding error to call this for a non-optional type
   Q_ASSERT(typeInfo.isOptional());

   // Most common field type is double, so check it first
   // QString is also pretty common, but it's never optional because an empty string suffices for "no data"
   if (typeInfo.typeIndex == typeid(double      )) { removeOptionalWrapper<double      >(propertyValue, hasValue); return; }
   if (typeInfo.typeIndex == typeid(int         )) { removeOptionalWrapper<int         >(propertyValue, hasValue); return; }
   if (typeInfo.typeIndex == typeid(unsigned int)) { removeOptionalWrapper<unsigned int>(propertyValue, hasValue); return; }
   if (typeInfo.typeIndex == typeid(bool        )) { removeOptionalWrapper<bool        >(propertyValue, hasValue); return; }
   if (typeInfo.typeIndex == typeid(QDate       )) { removeOptionalWrapper<QDate       >(propertyValue, hasValue); return; }

   // If the native type is an enum, then the QVariant should actually contain an int
   if (typeInfo.isEnum() && propertyValue.canConvert<std::optional<int>>()) {
      removeOptionalWrapper<int>(propertyValue, hasValue);
      return;
   }

   // It's a coding error if we reached here
   qCritical().noquote() <<
      Q_FUNC_INFO << "Unexpected type" << typeInfo << ", propertyValue" << propertyValue << ".  Call stack:" <<
      Logging::getStackTrace();
   Q_ASSERT(false);

   return;
}

QVariant Optional::makeNullOpt(TypeInfo const & typeInfo) {
   if (typeInfo.typeIndex == typeid(double      )) { return QVariant::fromValue<std::optional<double      >>(std::nullopt); }
   if (typeInfo.typeIndex == typeid(int         )) { return QVariant::fromValue<std::optional<int         >>(std::nullopt); }
   if (typeInfo.typeIndex == typeid(unsigned int)) { return QVariant::fromValue<std::optional<unsigned int>>(std::nullopt); }
   if (typeInfo.typeIndex == typeid(bool        )) { return QVariant::fromValue<std::optional<bool        >>(std::nullopt); }

   // It's a coding error if we reached here
   qCritical().noquote() <<
      Q_FUNC_INFO << "Unexpected type" << typeInfo << ".  Call stack:" << Logging::getStackTrace();
   Q_ASSERT(false);

   return QVariant{};
}

/**
 * \return \c true if \c input is empty or blank (ie contains only whitespace), \c false otherwise
 */
[[nodiscard]] bool Optional::isEmptyOrBlank(QString const & input) {
   if (input.isEmpty()) {
      return true;
   }
   if (input.trimmed().isEmpty()) {
      return true;
   }
   return false;
}
