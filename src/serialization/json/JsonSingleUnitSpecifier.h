/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonSingleUnitSpecifier.h is part of Brewtarget, and is copyright the following authors 2022:
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
#ifndef SERIALIZATION_JSON_JSONSINGLEUNITSPECIFIER_H
#define SERIALIZATION_JSON_JSONSINGLEUNITSPECIFIER_H
#pragma once

#include <string_view>
#include <QVector>

#include "serialization/json/JsonXPath.h"

/**
 * \brief Defines the expected unit for an JSON value:unit pairs that only ever uses one unit (eg percentages in
 *        BeerJSON).
 *
 *        We need this when outputting JSON to know how to write the value:unit pair.  But it is also useful when
 *        reading a JSON file, as it allows us to assert that the unit we're reading is what we expected, which should
 *        catch programming errors (eg if we try to read a percentage field into a pH attribute).
 *
 * \c validUnits tells us what the unit is allowed to be.  This is a list because, in some cases, there can be
 *               more than on name for the same unit - eg, in BeerJSON, "1", "unit", "each", "dimensionless" and "pkg"
 *               are all allowed for a quantity without any physical units.
 *
 *               The first item in the list is the one we will always use for generating this type of JSON field.
 *
 * \c unitField is the key used to pull out the string value representing the units of the measurement, usually "unit" in
 *              BeerJSON
 *
 * \c valueField is the key used to pull out the double value representing the measurement itself
 */
struct JsonSingleUnitSpecifier {
   QVector<std::string_view> const validUnits;
   JsonXPath const unitField = JsonXPath{"unit"};
   JsonXPath const valueField = JsonXPath{"value"};
};

#endif
