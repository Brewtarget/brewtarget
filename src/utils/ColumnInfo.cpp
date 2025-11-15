/*======================================================================================================================
 * utils/ColumnInfo.cpp is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#include "utils/ColumnInfo.h"

#include "widgets/SmartAmounts.h"

ColumnInfo::ColumnInfo(char const * const   modelName   ,
                       char const * const   columnName  ,
                       char const * const   columnFqName,
                       size_t       const   index       ,
//                       QString      const   label       ,
                       TypeLookup   const & typeLookup  ,
                       PropertyPath         propertyPath,
                       Extras       const   extras      ) :
   modelName   {modelName                           },
   columnName  {columnName                          },
   columnFqName{columnFqName                        },
   index       {index                               },
//   label       {label                               },
   typeInfo    {propertyPath.getTypeInfo(typeLookup)},
   propertyPath{propertyPath                        },
   extras      {extras                              } {
   return;
}

ColumnInfo::~ColumnInfo() = default;

void ColumnInfo::setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement) const {
   SmartAmounts::setForcedSystemOfMeasurement(this->modelName, this->columnName, forcedSystemOfMeasurement);
   return;
}

void ColumnInfo::setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) const {
   SmartAmounts::setForcedRelativeScale(this->modelName, this->columnName, forcedScale);
   return;
}

std::optional<Measurement::SystemOfMeasurement> ColumnInfo::getForcedSystemOfMeasurement() const {
   return SmartAmounts::getForcedSystemOfMeasurement(this->modelName, this->columnName);
}

std::optional<Measurement::UnitSystem::RelativeScale> ColumnInfo::getForcedRelativeScale() const {
   return SmartAmounts::getForcedRelativeScale(this->modelName, this->columnName);
}
