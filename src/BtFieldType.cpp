/*
 * BtFieldType.cpp is part of Brewtarget, and is copyright the following
 * authors 2022-2023
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
#include "BtFieldType.h"

#include <QDebug>

#include "utils/EnumStringMapping.h"

namespace {
   EnumStringMapping const nonPhysicalQuantityToName {
      {"Date"         , NonPhysicalQuantity::Date         },
      {"String"       , NonPhysicalQuantity::String       },
      {"Count"        , NonPhysicalQuantity::Count        },
      {"Percentage"   , NonPhysicalQuantity::Percentage   },
      {"Bool"         , NonPhysicalQuantity::Bool         },
      {"Dimensionless", NonPhysicalQuantity::Dimensionless},
   };
}

QString GetDisplayName(NonPhysicalQuantity nonPhysicalQuantity) {
   // It's a coding error if we don't find a result (in which case EnumStringMapping::enumToString will log an error and
   // throw an exception).
   return nonPhysicalQuantityToName.enumToString(nonPhysicalQuantity);
}
