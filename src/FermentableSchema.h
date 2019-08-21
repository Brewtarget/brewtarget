/*
 * FermentableTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
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

#ifndef _FERMTABLESCHEMA_H
#define _FERMTABLESCHEMA_H
// These will collide, so I define them once in the TableSchemaConst file
// const QString kcolEquipName("name");
// const QString kcolEquipNotes("notes");
//
// Column names
const QString kcolFermType("ftype");
static const QString kcolFermAmount("amount");
// no inventory column
static const QString kcolFermYield("yield");
static const QString kcolFermColor("color");
static const QString kcolFermAddAfterBoil("add_after_boil");
static const QString kcolFermOrigin("origin");
static const QString kcolFermSupplier("supplier");
static const QString kcolFermCoarseFineDiff("coarse_fine_diff");
static const QString kcolFermMoisture("moisture");
static const QString kcolFermDiastaticPower("diastatic_power");
static const QString kcolFermProtein("protein");
static const QString kcolFermMaxInBatch("max_in_batch");
static const QString kcolFermRecommendMash("recommend_mash");
static const QString kcolFermIsMashed("is_mashed");
static const QString kcolFermIBUGalPerLb("ibu_gal_per_lb");

// Prop names
// These are defined in the main constants file
// const QString kpropName("name");
// const QString kpropNotes("notes");
// const QString kpropType("type");
// const QString kpropInventory("inventory");
// static const QString kpropAmountKg("amount_kg");
// static const QString kpropOrigin("origin");
static const QString kpropYield("yield_pct");
static const QString kpropColor("color_srm");
static const QString kpropAddAfterBoil("addAfterBoil");
static const QString kpropSupplier("supplier");
static const QString kpropCoarseFineDiff("coarseFineDiff_pct");
static const QString kpropMoisture("moisture_pct");
static const QString kpropDiastaticPower("diastaticPower_lintner");
static const QString kpropProtein("protein_pct");
static const QString kpropMaxInBatch("maxInBatch_pct");
static const QString kpropRecommendMash("recommendMash");
static const QString kpropIsMashed("isMashed");
static const QString kpropIBUGalPerLb("ibuGalPerLb");

// XML prop names
// const QString kXmlNameProp("NAME");
// const QString kXmlNotesProp("NOTES");
// static const QString kxmlPropAmount("AMOUNT");

static const QString kxmlPropInventory("INVENTORY");
static const QString kxmlPropYield("YIELD");
static const QString kxmlPropColor("COLOR");
static const QString kxmlPropAddAfterBoil("ADD_AFTER_BOIL");
static const QString kxmlPropOrigin("ORIGIN");
static const QString kxmlPropSupplier("SUPPLIER");
static const QString kxmlPropCoarseFineDiff("COARSE_FINE_DIFF");
static const QString kxmlPropMoisture("MOISTURE");
static const QString kxmlPropDiastaticPower("DIASTATIC_POWER");
static const QString kxmlPropProtein("PROTEIN");
static const QString kxmlPropMaxInBatch("MAX_IN_BATCH");
static const QString kxmlPropRecommendMash("RECOMMEND_MASH");
static const QString kxmlPropIsMashed("IS_MASHED");
static const QString kxmlPropIBUGalPerLb("IBU_GAL_PER_LB");

#endif // define _FERMTABLESCHEMA_H
