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

// Columns for the recipe table
static const QString kcolRecipeType("type");
static const QString kcolRecipeBrewer("brewer");
static const QString kcolRecipeBatchSize("batch_size");
static const QString kcolRecipeBoilSize("boil_size");
static const QString kcolRecipeBoilTime("boil_time");
static const QString kcolRecipeEfficiency("efficiency");
static const QString kcolRecipeAsstBrewer("assistant_brewer");
static const QString kcolRecipeTasteNotes("taste_notes");
static const QString kcolRecipeTasteRating("taste_rating");
static const QString kcolRecipeOG("og");
static const QString kcolRecipeFG("fg");
static const QString kcolRecipeFermentationStages("fermentation_stages");
static const QString kcolRecipePrimaryAgeDays("primary_age");
static const QString kcolRecipePrimaryTemp("primary_temp");
static const QString kcolRecipeSecondaryAgeDays("secondary_age");
static const QString kcolRecipeSecondaryTemp("secondary_temp");
static const QString kcolRecipeTertiaryAgeDays("tertiary_age");
static const QString kcolRecipeTertiaryTemp("tertiary_temp");
static const QString kcolRecipeAge("age");
static const QString kcolRecipeAgeTemp("age_temp");
static const QString kcolRecipeDate("date");
static const QString kcolRecipeCarbonationVols("carb_volume");
static const QString kcolRecipeForcedCarbonation("forced_carb");
static const QString kcolRecipePrimingSugarName("priming_sugar_name");
static const QString kcolRecipeCarbonationTemp("carbonationTemp_c");
static const QString kcolRecipePrimingSugarEquiv("priming_sugar_equiv");
static const QString kcolRecipeKegPrimingFactor("keg_priming_factor");

// Some of the foreign keys
static const QString kcolRecipeEquipmentId("equipment_id");
static const QString kcolRecipeMashId("mash_id");
static const QString kcolRecipeStyleId("style_id");
static const QString kcolRecipeAncestorId("ancestor_id");

// properties for the recipe object
static const QString kpropBrewer("brewer");
static const QString kpropEfficiency("efficiency_pct");
static const QString kpropAsstBrewer("asstBrewer");
static const QString kpropTasteNotes("tasteNotes");
static const QString kpropTasteRating("tasteRating");
static const QString kpropOG("og");
static const QString kpropFG("fg");
static const QString kpropFermentationStages("fermentationStages");
static const QString kpropPrimaryAgeDays("primaryAge_days");
static const QString kpropPrimaryTemp("primaryTemp_c");
static const QString kpropSecondaryAgeDays("secondaryAge_days");
static const QString kpropSecondaryTemp("secondaryTemp_c");
static const QString kpropTertiaryAgeDays("tertiaryAge_days");
static const QString kpropTertiaryTemp("tertiaryTemp_c");
static const QString kpropAge("age");
static const QString kpropAgeTemp("ageTemp_c");
static const QString kpropDate("date");
static const QString kPoints("points");
static const QString kpropCarbonationVols("carbonation_vols");
static const QString kpropForcedCarbonation("forcedCarbonation");
static const QString kpropPrimingSugarName("primingSugarName");
static const QString kpropCarbonationTemp("carbonationTemp_c");
static const QString kpropPrimingSugarEquiv("primingSugarEquiv");
static const QString kpropKegPrimingFactor("kegPrimingFactor");

static const QString kpropEquipmentId("equipment_id");
static const QString kpropMashId("mash_id");
static const QString kpropStyleId("style_id");
static const QString kpropAncestorId("ancestor_id");

// Huh. Took a long time for a xmlPropType to show up
static const QString kxmlPropType("TYPE");
static const QString kxmlPropBrewer("BREWER");
static const QString kxmlPropEfficiency("EFFICIENCY");
static const QString kxmlPropAsstBrewer("ASST_BREWER");
static const QString kxmlPropTasteNotes("TASTE_NOTES");
static const QString kxmlPropTasteRating("TASTE_RATING");
static const QString kxmlPropOG("OG");
static const QString kxmlPropFG("FG");
static const QString kxmlPropFermentationStages("FERMENTATION_STAGES");
static const QString kxmlPropPrimaryAgeDays("PRIMARY_AGE");
static const QString kxmlPropPrimaryTemp("PRIMARY_TEMP");
static const QString kxmlPropSecondaryAgeDays("SECONDARY_AGE");
static const QString kxmlPropSecondaryTemp("SECONDARY_TEMP");
static const QString kxmlPropTertiaryAgeDays("TERTIARY_AGE");
static const QString kxmlPropTertiaryTemp("TERTIARY_TEMP");
static const QString kxmlPropAge("AGE");
static const QString kxmlPropAgeTemp("AGE_TEMP");
static const QString kxmlPropDate("DATE");
static const QString kxmlPropCarbonationVols("CARBONATION");
static const QString kxmlPropForcedCarbonation("FORCED_CARBONATION");
static const QString kxmlPropPrimingSugarName("PRIMING_SUGAR_NAME");
static const QString kxmlPropCarbonationTemp("CARBONATION_TEMP");
static const QString kxmlPropPrimingSugarEquiv("PRIMING_SUGAR_EQUIV");
static const QString kxmlPropKegPrimingFactor("KEG_PRIMING_FACTOR");

#endif // _RECIPETABLESCHEMA_H
