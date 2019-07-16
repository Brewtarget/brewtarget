/*
 * TableSchema.h is part of Brewtarget, and is Copyright the following
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
#ifndef _TABLESCHEMA_H
#define _TABLESCHEMA_H

#include "PropertySchema.h"
#include "brewtarget.h"
#include "database.h"
#include <QString>

class DatabaseSchema;

class TableSchema : QObject
{

   friend DatabaseSchema;
   friend Database;

   Q_OBJECT
public:

   const QString tableName();
   const Brewtarget::DBTable dbTable();
   const QMap<QString, PropertySchema*> properties() const;

   const PropertySchema* property(QString prop) const;

   // returns all of the column names
   const QStringList propertyToColumn(QString prop) const;
   // returns just one for the database type at hand
   const QString propertyToColumn(QString prop, Brewtarget::DBTypes type) const;

   // get the type name for this column
   const QString propertyColumnType(QString prop) const;
   // get the default value for this column
   const QVariant propertyColumnDefault(QString prop) const;
   // get the column size of the property's column
   const int propertyColumnSize(QString prop) const;

   const QStringList allPropertyNames() const;
   const QStringList allColumnNames(Brewtarget::DBTypes type = Brewtarget::NODB) const;
private:

   // You can create this either via tableName or DBTable
   TableSchema(QString tablename);
   TableSchema(Brewtarget::DBTable dbTable);

   QString tableName_;
   Brewtarget::DBTable dbTable_;
   QMap<QString,PropertySchema*> properties_;

   QMap<QString,PropertySchema*> defineTable(Brewtarget::DBTable table);
   QMap<QString,PropertySchema*> defineStyleTable();
   QMap<QString,PropertySchema*> defineEquipmentTable();
/*
   static QString TableSchema::kcolMetaClassName("class_name");
   static QString TableSchema::kcolMetaInvId("inventory_table_id");
   static QString TableSchema::kcolMetaChildId("child_table_id");
   static QString TableSchema::kcolMetaDateCreated("created");
   static QString TableSchema::kcolMetaVersion("version");
   static QString TableSchema::kcolMetaTableId("table_id");

   static QString TableSchema::ktableSettings("settings");
   static QString TableSchema::kcolSettingsVersion("version");
   static QString TableSchema::kcolSettingsRepopulateChildren("repopulateChildrenOnNextStart");

   static QString TableSchema::ktableFermentable("fermentable");
   static QString TableSchema::kcolFermFtype("ftype");
   static QString TableSchema::kcolFermAmount("amount");
   static QString TableSchema::kcolFermYield("yield");
   static QString TableSchema::kcolFermColor("color");
   static QString TableSchema::kcolFermAddAfterBoil("add_after_boil");
   static QString TableSchema::kcolFermOrigin("origin");
   static QString TableSchema::kcolFermSupplier("supplier");
   static QString TableSchema::kcolFermNotes("notes");
   static QString TableSchema::kcolFermCoarseFineDiff("coarse_fine_diff");
   static QString TableSchema::kcolFermMoisture("moisture");
   static QString TableSchema::kcolFermDiastaticPower("diastatic_power");
   static QString TableSchema::kcolFermProtein("protein");
   static QString TableSchema::kcolFermMaxInBatch("max_in_batch");
   static QString TableSchema::kcolFermRecommendMash("recommend_mash");
   static QString TableSchema::kcolFermIsMashed("is_mashed");
   static QString TableSchema::kcolFermIbuGalLb("ibu_gal_per_lb");

   static QString TableSchema::ktableHop("hop");
   static QString TableSchema::kcolHopAlpha("alpha");
   static QString TableSchema::kcolHopAmount("amount");
   static QString TableSchema::kcolHopUse("use");
   static QString TableSchema::kcolHopTime("time");
   static QString TableSchema::kcolHopNotes("notes");
   static QString TableSchema::kcolHopHtype("htype");
   static QString TableSchema::kcolHopForm("form");
   static QString TableSchema::kcolHopBeta("beta");
   static QString TableSchema::kcolHopHsi("hsi");
   static QString TableSchema::kcolHopOrigin("origin");
   static QString TableSchema::kcolHopSubstitutes("substitutes");
   static QString TableSchema::kcolHopHumulene("humulene");
   static QString TableSchema::kcolHopCaryophyllene("caryophyllene");
   static QString TableSchema::kcolHopCohumulone("cohumulone");
   static QString TableSchema::kcolHopMyrcene("myrcene");

   static QString TableSchema::ktableMisc("misc");
   static QString TableSchema::kcolMiscMtype("mtype");
   static QString TableSchema::kcolMiscUse("use");
   static QString TableSchema::kcolMiscTime("time");
   static QString TableSchema::kcolMiscAmount("amount");
   static QString TableSchema::kcolMiscAmountIsWeight("amount_is_weight");
   static QString TableSchema::kcolMiscUseFor("use_for");
   static QString TableSchema::kcolMiscNotes("notes");
   
   static QString TableSchema::ktableYeast("yeast");
   static QString TableSchema::kcolYeastType("ytype");
   static QString TableSchema::kcolYeastForm("form");
   static QString TableSchema::kcolYeastAmount("amount");
   static QString TableSchema::kcolYeastAmountIsWeight("amount_is_weight");
   static QString TableSchema::kcolYeastLab("laboratory");
   static QString TableSchema::kcolYeastProductId("product_id");
   static QString TableSchema::kcolYeastTempMin("min_temperature");
   static QString TableSchema::kcolYeastTempMax("max_temperature");
   static QString TableSchema::kcolYeastFlocc("flocculation");
   static QString TableSchema::kcolYeastAtten("attenuation");
   static QString TableSchema::kcolYeastNotes("notes");
   static QString TableSchema::kcolYeastBestFor("best_for");
   static QString TableSchema::kcolYeastRecultures("times_cultured");
   static QString TableSchema::kcolYeastReuseMax("max_reuse");
   static QString TableSchema::kcolYeastSecondary("add_to_secondary");

   static QString TableSchema::ktableWater("water");
   static QString TableSchema::kcolWaterAmount("amount");
   static QString TableSchema::kcolWaterCa("calcium");
   static QString TableSchema::kcolWaterBicarb("bicarbonate");
   static QString TableSchema::kcolWaterSulfate("sulfate");
   static QString TableSchema::kcolWaterCl("chloride");
   static QString TableSchema::kcolWaterNa("sodium");
   static QString TableSchema::kcolWaterMg("magnesium");
   static QString TableSchema::kcolWaterPh("ph");
   static QString TableSchema::kcolWaterNotes("notes");

   // Mashes can be unnamed, so we need a different definition here.
   static QString TableSchema::ktableMash("mash");
   static QString TableSchema::kcolMashName("name " + TYPETEXT + " DEFAULT ''");
   static QString TableSchema::kcolMashGrainTemp("grain_temp");
   static QString TableSchema::kcolMashNotes("notes");
   static QString TableSchema::kcolMashTunTemp("tun_temp");
   static QString TableSchema::kcolMashSpargeTemp("sparge_temp");
   static QString TableSchema::kcolMashPh("ph");
   static QString TableSchema::kcolMashTunWeight("tun_weight");
   static QString TableSchema::kcolMashTunSpecificHeat("tun_specific_heat");
   static QString TableSchema::kcolMashEquipAdjust("equip_adjust");

   static QString TableSchema::ktableMashStep("mashstep");
   static QString TableSchema::kcolMashStepType("mstype varchar(32) DEFAULT 'Infusion'");
   static QString TableSchema::kcolMashStepInfAmount("infuse_amount");
   static QString TableSchema::kcolMashStepTemp("step_temp");
   static QString TableSchema::kcolMashStepTime("step_time");
   static QString TableSchema::kcolMashStepRampTime("ramp_time");
   static QString TableSchema::kcolMashStepEndTemp("end_temp");
   static QString TableSchema::kcolMashStepInfTemp("infuse_temp");
   static QString TableSchema::kcolMashStepDecAmount("decoction_amount");
   static QString TableSchema::kcolMashStepMashId("mash_id");
   static QString TableSchema::kcolMashStepNumber("step_number");

   // Brewnotes table
   static QString TableSchema::ktableBrewnote("brewnote");
   static QString TableSchema::kcolBNoteBrewDate("brewDate");
   static QString TableSchema::kcolBNoteAttenuation("attenuation");
   static QString TableSchema::kcolBNoteFermentDate("fermentDate");
   static QString TableSchema::kcolBNoteSg("sg");
   static QString TableSchema::kcolBNoteBkVolume("volume_into_bk");
   static QString TableSchema::kcolBNoteStrikeTemp("strike_temp");
   static QString TableSchema::kcolBNoteFinalMashTemp("mash_final_temp");
   static QString TableSchema::kcolBNoteOg("og");
   static QString TableSchema::kcolBNotePostboilVolume("post_boil_volume");
   static QString TableSchema::kcolBNoteFermenterVolume("volume_into_fermenter");
   static QString TableSchema::kcolBNotePitchTemp("pitch_temp");
   static QString TableSchema::kcolBNoteFg("fg");
   static QString TableSchema::kcolBNoteBkEff("eff_into_bk");
   static QString TableSchema::kcolBNoteAbv("abv");
   static QString TableSchema::kcolBNotePredOg("predicted_og");
   static QString TableSchema::kcolBNoteEff("brewhouse_eff");
   static QString TableSchema::kcolBNotePredAbv("predicted_abv");
   static QString TableSchema::kcolBNoteProjBoilGrav("projected_boil_grav");
   static QString TableSchema::kcolBNoteProjStrikeTemp("projected_strike_temp");
   static QString TableSchema::kcolBNoteProjFinTemp("projected_fin_temp");
   static QString TableSchema::kcolBNoteProjFinMashTemp("projected_mash_fin_temp");
   static QString TableSchema::kcolBNoteProjBkVol("projected_vol_into_bk");
   static QString TableSchema::kcolBNoteProjOg("projected_og");
   static QString TableSchema::kcolBNoteProjFermVol("projected_vol_into_ferm");
   static QString TableSchema::kcolBNoteProjFg("projected_fg");
   static QString TableSchema::kcolBNoteProjEff("projected_eff");
   static QString TableSchema::kcolBNoteProjAbv("projected_abv");
   static QString TableSchema::kcolBNoteProjAtten("projected_atten");
   static QString TableSchema::kcolBNoteProjPoints("projected_points");
   static QString TableSchema::kcolBNoteProjFermPoints("projected_ferm_points");
   static QString TableSchema::kcolBNoteBoilOff("boil_off");
   static QString TableSchema::kcolBNoteFinalVolume("final_volume");
   static QString TableSchema::kcolBNoteNotes("notes");
   static QString TableSchema::kcolBNoteRecipeId("recipe_id");

   static QString TableSchema::ktableInstruction("instruction");
   static QString TableSchema::kcolInsDirections("directions");
   static QString TableSchema::kcolInsHasTimer("hasTimer");
   static QString TableSchema::kcolInsTimerVal("timerValue");
   static QString TableSchema::kcolInsCompleted("completed");
   static QString TableSchema::kcolInsInterval("interval");

   static QString TableSchema::ktableRecipe("recipe");
   static QString TableSchema::kcolRecType("type");
   static QString TableSchema::kcolRecBrewer("brewer");
   static QString TableSchema::kcolRecAsstBrewer("assistant_brewer");
   static QString TableSchema::kcolRecBatchSize("batch_size");
   static QString TableSchema::kcolRecBoilSize("boil_size");
   static QString TableSchema::kcolRecBoilTime("boil_time");
   static QString TableSchema::kcolRecEff("efficiency");
   static QString TableSchema::kcolRecOg("og");
   static QString TableSchema::kcolRecFg("fg");
   static QString TableSchema::kcolRecFermStages("fermentation_stages");
   static QString TableSchema::kcolRecPrimAge("primary_age");
   static QString TableSchema::kcolRecPrimTemp("primary_temp");
   static QString TableSchema::kcolRecSecAge("secondary_age");
   static QString TableSchema::kcolRecSecTemp("secondary_temp");
   static QString TableSchema::kcolRecTerAge("tertiary_age");
   static QString TableSchema::kcolRecTerTemp("tertiary_temp");
   static QString TableSchema::kcolRecAge("age");
   static QString TableSchema::kcolRecAgeTemp("age_temp");
   static QString TableSchema::kcolRecDate("date");
   static QString TableSchema::kcolRecCarbVol("carb_volume");
   static QString TableSchema::kcolRecForceCarb("forced_carb");
   static QString TableSchema::kcolRecPrimSug("priming_sugar_name");
   static QString TableSchema::kcolRecCarbTemp("carbonationTemp_c");
   static QString TableSchema::kcolRecPrimSugEquiv("priming_sugar_equiv");
   static QString TableSchema::kcolRecKegPrimFact("keg_priming_factor");
   static QString TableSchema::kcolRecNotes("notes");
   static QString TableSchema::kcolRecTasteNotes("taste_notes");
   static QString TableSchema::kcolRecTasteRating("taste_rating");
   static QString TableSchema::kcolRecStyleId("style_id");
   static QString TableSchema::kcolRecMashId("mash_id");
   static QString TableSchema::kcolRecEquipId("equipment_id");

   static QString TableSchema::ktableBtEquipment("bt_equipment");
   static QString TableSchema::ktableBtFermentable("bt_fermentable");
   static QString TableSchema::ktableBtHop("bt_hop");
   static QString TableSchema::ktableBtMisc("bt_misc");
   static QString TableSchema::ktableBtStyle("bt_style");
   static QString TableSchema::ktableBtYeast("bt_yeast");
   static QString TableSchema::ktableBtWater("bt_water");

   static QString TableSchema::ktableFermInRec("fermentable_in_recipe");
   static QString TableSchema::ktableHopInRec("hop_in_recipe");
   static QString TableSchema::ktableMiscInRec("misc_in_recipe");
   static QString TableSchema::ktableWaterInRec("water_in_recipe");
   static QString TableSchema::ktableYeastInRec("yeast_in_recipe");
   static QString TableSchema::ktableInsInRec("instruction_in_recipe");

   static QString TableSchema::ktableEquipChildren("equipment_children");
   static QString TableSchema::ktableFermChildren("fermentable_children");
   static QString TableSchema::ktableHopChildren("hop_children");
   static QString TableSchema::ktableMiscChildren("misc_children");
   static QString TableSchema::ktableRecChildren("recipe_children");
   static QString TableSchema::ktableStyleChildren("style_children");
   static QString TableSchema::ktableWaterChildren("water_children");
   static QString TableSchema::ktableYeastChildren("yeast_children");

   static QString TableSchema::ktableFermInventory("fermentable_in_inventory");
   static QString TableSchema::ktableHopInventory("hop_in_inventory");
   static QString TableSchema::ktableMiscInventory("misc_in_inventory");
   static QString TableSchema::ktableYeastInventory("yeast_in_inventory");
*/
};

#endif
