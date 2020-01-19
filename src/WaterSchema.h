/*
 * WaterTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the yeaste that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WATERTABLESCHEMA_H
#define _WATERTABLESCHEMA_H

#include <QString>
// Columns for the yeast table
// What isn't here (like name) is defined in TableSchemaConstants
static const QString kcolWaterCalcium("calcium");
static const QString kcolWaterBiCarbonate("bicarbonate");
static const QString kcolWaterSulfate("sulfate");
static const QString kcolWaterChloride("chloride");
static const QString kcolWaterSodium("sodium");
static const QString kcolWaterMagnesium("magnesium");
static const QString kcolWaterAlkalinity("alkalinity");
static const QString kcolWaterType("wtype");

// properties for objects
static const QString kpropCalcium("calcium");
static const QString kpropBiCarbonate("bicarbonate");
static const QString kpropSulfate("sulfate");
static const QString kpropChloride("chloride");
static const QString kpropSodium("sodium");
static const QString kpropMagnesium("magnesium");
static const QString kpropAlkalinity("alkalinity");

// XML properties
// Note -- no XML props for type, form and flocculation
static const QString kxmlPropCalcium("CALCIUM");
static const QString kxmlPropBiCarbonate("BICARBONATE");
static const QString kxmlPropSulfate("SULFATE");
static const QString kxmlPropChloride("CHLORIDE");
static const QString kxmlPropSodium("SODIUM");
static const QString kxmlPropMagnesium("MAGNESIUM");

#endif // _WATERTABLESCHEMA_H
