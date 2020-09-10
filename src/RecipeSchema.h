/*
 * RecipeTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the recipee that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _RECIPETABLESCHEMA_H
#define _RECIPETABLESCHEMA_H

#include <QString>
// Columns for the recipe table
static const QString kcolRecipeType("type");
static const QString kcolRecipeBrewer("brewer");
static const QString kcolRecipeBatchSize("batch_size");
static const QString kcolRecipeBoilSize("boil_size");
static const QString kcolRecipeBoilTime("boil_time");
static const QString kcolRecipeEff("efficiency");
static const QString kcolRecipeAsstBrewer("assistant_brewer");
static const QString kcolRecipeTasteNotes("taste_notes");
static const QString kcolRecipeTasteRating("taste_rating");
static const QString kcolRecipeOG("og");
static const QString kcolRecipeFG("fg");
static const QString kcolRecipeFermStages("fermentation_stages");
static const QString kcolRecipePrimAgeDays("primary_age");
static const QString kcolRecipePrimTemp("primary_temp");
static const QString kcolRecipeSecAgeDays("secondary_age");
static const QString kcolRecipeSecTemp("secondary_temp");
static const QString kcolRecipeTertAgeDays("tertiary_age");
static const QString kcolRecipeTertTemp("tertiary_temp");
static const QString kcolRecipeAge("age");
static const QString kcolRecipeAgeTemp("age_temp");
static const QString kcolRecipeDate("date");
static const QString kcolRecipeCarbVols("carb_volume");
static const QString kcolRecipeForcedCarb("forced_carb");
static const QString kcolRecipePrimSugName("priming_sugar_name");
static const QString kcolRecipeCarbTemp("carbonationtemp_c");
static const QString kcolRecipePrimSugEquiv("priming_sugar_equiv");
static const QString kcolRecipeKegPrimFact("keg_priming_factor");

// Some of the foreign keys
static const QString kcolRecipeEquipmentId("equipment_id");
static const QString kcolRecipeStyleId("style_id");
static const QString kcolRecipeAncestorId("ancestor_id");

// properties for the recipe object
static const QString kpropBrewer("brewer");
static const QString kpropEffPct("efficiency_pct");
static const QString kpropAsstBrewer("asstBrewer");
static const QString kpropTasteNotes("tasteNotes");
static const QString kpropTasteRating("tasteRating");
static const QString kpropFermStages("fermentationStages");
static const QString kpropPrimAgeDays("primaryAge_days");
static const QString kpropPrimTemp("primaryTemp_c");
static const QString kpropSecAgeDays("secondaryAge_days");
static const QString kpropSecTemp("secondaryTemp_c");
static const QString kpropTertAgeDays("tertiaryAge_days");
static const QString kpropTertTemp("tertiaryTemp_c");
static const QString kpropAge("age");
static const QString kpropAgeTemp("ageTemp_c");
static const QString kpropDate("date");
static const QString kpropPoints("points");
static const QString kpropCarbVols("carbonation_vols");
static const QString kpropForcedCarb("forcedCarbonation");
static const QString kpropPrimSugName("primingSugarName");
static const QString kpropCarbTemp("carbonationTemp_c");
static const QString kpropPrimSugEquiv("primingSugarEquiv");
static const QString kpropKegPrimFact("kegPrimingFactor");

static const QString kpropAncestorId("ancestor_id");

// Huh. Took a long time for a xmlPropType to show up
static const QString kxmlPropType("TYPE");
static const QString kxmlPropBrewer("BREWER");
static const QString kxmlPropEff("EFFICIENCY");
static const QString kxmlPropAsstBrewer("ASST_BREWER");
static const QString kxmlPropTasteNotes("TASTE_NOTES");
static const QString kxmlPropTasteRating("TASTE_RATING");
static const QString kxmlPropFermStages("FERMENTATION_STAGES");
static const QString kxmlPropPrimAgeDays("PRIMARY_AGE");
static const QString kxmlPropPrimTemp("PRIMARY_TEMP");
static const QString kxmlPropSecAgeDays("SECONDARY_AGE");
static const QString kxmlPropSecTemp("SECONDARY_TEMP");
static const QString kxmlPropTertAgeDays("TERTIARY_AGE");
static const QString kxmlPropTertTemp("TERTIARY_TEMP");
static const QString kxmlPropAge("AGE");
static const QString kxmlPropAgeTemp("AGE_TEMP");
static const QString kxmlPropDate("DATE");
static const QString kxmlPropCarbVols("CARBONATION");
static const QString kxmlPropForcedCarb("FORCED_CARBONATION");
static const QString kxmlPropPrimSugName("PRIMING_SUGAR_NAME");
static const QString kxmlPropCarbTemp("CARBONATION_TEMP");
static const QString kxmlPropPrimSugEquiv("PRIMING_SUGAR_EQUIV");
static const QString kxmlPropKegPrimFact("KEG_PRIMING_FACTOR");

#endif // _RECIPETABLESCHEMA_H
