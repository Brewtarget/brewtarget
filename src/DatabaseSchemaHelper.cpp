/*
 * DatabaseHelper.cpp is part of Brewtarget, and is Copyright 2009-2014 by
 * the following authors:
 *   - Philip G. Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DatabaseSchemaHelper.h"

#include "brewtarget.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <QSqlError>

const int DatabaseSchemaHelper::dbVersion = 4;

// Commands and keywords
QString DatabaseSchemaHelper::CREATETABLE("CREATE TABLE");
QString DatabaseSchemaHelper::ALTERTABLE("ALTER TABLE");
QString DatabaseSchemaHelper::DROPTABLE("DROP TABLE");
QString DatabaseSchemaHelper::ADDCOLUMN("ADD COLUMN");
QString DatabaseSchemaHelper::UPDATE("UPDATE");
QString DatabaseSchemaHelper::SET("SET");
QString DatabaseSchemaHelper::INSERTINTO("INSERT INTO");
QString DatabaseSchemaHelper::DEFAULT("DEFAULT");
QString DatabaseSchemaHelper::SELECT("SELECT");
QString DatabaseSchemaHelper::SEP(" ");
QString DatabaseSchemaHelper::UNIQUE("UNIQUE");

// Types
QString DatabaseSchemaHelper::TYPEINTEGER("INTEGER");
QString DatabaseSchemaHelper::TYPETEXT("TEXT");
QString DatabaseSchemaHelper::TYPEREAL("REAL");
QString DatabaseSchemaHelper::TYPENUMERIC("NUMERIC");
QString DatabaseSchemaHelper::TYPEDATETIME("DATETIME");

QString DatabaseSchemaHelper::id("id " + TYPEINTEGER + " PRIMARY KEY autoincrement");
QString DatabaseSchemaHelper::name("name " + TYPETEXT + " not null DEFAULT ''");
QString DatabaseSchemaHelper::displayUnit("display_unit" + SEP + TYPEINTEGER + SEP + DEFAULT + " -1");
QString DatabaseSchemaHelper::displayScale("display_scale" + SEP + TYPEINTEGER + SEP + DEFAULT + " -1");
QString DatabaseSchemaHelper::displayTempUnit("display_temp_unit" + SEP + TYPEINTEGER + SEP + DEFAULT + " -1");
QString DatabaseSchemaHelper::deleted("deleted" + SEP + TYPEINTEGER + SEP + DEFAULT + " 0");
QString DatabaseSchemaHelper::display("display" + SEP + TYPEINTEGER + SEP + DEFAULT + " 1");
QString DatabaseSchemaHelper::folder("folder " + TYPETEXT + " DEFAULT ''");

QString DatabaseSchemaHelper::tableSettings("settings");
QString DatabaseSchemaHelper::colSettingsVersion("version");
QString DatabaseSchemaHelper::colSettingsRepopulateChildren("repopulateChildrenOnNextStart");

QString DatabaseSchemaHelper::tableEquipment("equipment");
QString DatabaseSchemaHelper::colEquipBoilSize("boil_size");
QString DatabaseSchemaHelper::colEquipBatchSize("batch_size");
QString DatabaseSchemaHelper::colEquipTunVolume("tun_volume");
QString DatabaseSchemaHelper::colEquipTunWeight("tun_weight");
QString DatabaseSchemaHelper::colEquipTunSpecificHeat("tun_specific_heat");
QString DatabaseSchemaHelper::colEquipTopUpWater("top_up_water");
QString DatabaseSchemaHelper::colEquipTrubChillerLoss("trub_chiller_loss");
QString DatabaseSchemaHelper::colEquipEvapRate("evap_rate");
QString DatabaseSchemaHelper::colEquipBoilTime("boil_time");
QString DatabaseSchemaHelper::colEquipCalcBoilVolume("calc_boil_volume");
QString DatabaseSchemaHelper::colEquipLauterDeadspace("lauter_deadspace");
QString DatabaseSchemaHelper::colEquipTopUpKettle("top_up_kettle");
QString DatabaseSchemaHelper::colEquipHopUtilization("hop_utilization");
QString DatabaseSchemaHelper::colEquipNotes("notes");
QString DatabaseSchemaHelper::colEquipRealEvapRate("real_evap_rate");
QString DatabaseSchemaHelper::colEquipBoilingPoint("boiling_point");
QString DatabaseSchemaHelper::colEquipAbsorption("absorption");

QString DatabaseSchemaHelper::tableFermentable("fermentable");
QString DatabaseSchemaHelper::colFermFtype("ftype");
QString DatabaseSchemaHelper::colFermAmount("amount");
QString DatabaseSchemaHelper::colFermYield("yield");
QString DatabaseSchemaHelper::colFermColor("color");
QString DatabaseSchemaHelper::colFermAddAfterBoil("add_after_boil");
QString DatabaseSchemaHelper::colFermOrigin("origin");
QString DatabaseSchemaHelper::colFermSupplier("supplier");
QString DatabaseSchemaHelper::colFermNotes("notes");
QString DatabaseSchemaHelper::colFermCoarseFineDiff("coarse_fine_diff");
QString DatabaseSchemaHelper::colFermMoisture("moisture");
QString DatabaseSchemaHelper::colFermDiastaticPower("diastatic_power");
QString DatabaseSchemaHelper::colFermProtein("protein");
QString DatabaseSchemaHelper::colFermMaxInBatch("max_in_batch");
QString DatabaseSchemaHelper::colFermRecommendMash("recommend_mash");
QString DatabaseSchemaHelper::colFermIsMashed("is_mashed");
QString DatabaseSchemaHelper::colFermIbuGalLb("ibu_gal_per_lb");

QString DatabaseSchemaHelper::tableHop("hop");
QString DatabaseSchemaHelper::colHopAlpha("alpha");
QString DatabaseSchemaHelper::colHopAmount("amount");
QString DatabaseSchemaHelper::colHopUse("use");
QString DatabaseSchemaHelper::colHopTime("time");
QString DatabaseSchemaHelper::colHopNotes("notes");
QString DatabaseSchemaHelper::colHopHtype("htyp");
QString DatabaseSchemaHelper::colHopForm("form");
QString DatabaseSchemaHelper::colHopBeta("beta");
QString DatabaseSchemaHelper::colHopHsi("his");
QString DatabaseSchemaHelper::colHopOrigin("origin");
QString DatabaseSchemaHelper::colHopSubstitutes("substitutes");
QString DatabaseSchemaHelper::colHopHumulene("humulene");
QString DatabaseSchemaHelper::colHopCaryophyllene("caryophyllene");
QString DatabaseSchemaHelper::colHopCohumulone("cohumulone");
QString DatabaseSchemaHelper::colHopMyrcene("myrcene");

QString DatabaseSchemaHelper::tableMisc("misc");
QString DatabaseSchemaHelper::colMiscMtype("mtype");
QString DatabaseSchemaHelper::colMiscUse("use");
QString DatabaseSchemaHelper::colMiscTime("time");
QString DatabaseSchemaHelper::colMiscAmount("amount");
QString DatabaseSchemaHelper::colMiscAmountIsWeight("amount_is_weight");
QString DatabaseSchemaHelper::colMiscUseFor("use_for");
QString DatabaseSchemaHelper::colMiscNotes("notes");

QString DatabaseSchemaHelper::tableStyle("style");
QString DatabaseSchemaHelper::colStyleType("s_type");
QString DatabaseSchemaHelper::colStyleCat("category");
QString DatabaseSchemaHelper::colStyleCatNum("category_number");
QString DatabaseSchemaHelper::colStyleLetter("style_letter");
QString DatabaseSchemaHelper::colStyleGuide("style_guide");
QString DatabaseSchemaHelper::colStyleOgMin("og_min");
QString DatabaseSchemaHelper::colStyleOgMax("og_max");
QString DatabaseSchemaHelper::colStyleFgMin("fg_min");
QString DatabaseSchemaHelper::colStyleFgMax("fg_max");
QString DatabaseSchemaHelper::colStyleIbuMin("ibu_min");
QString DatabaseSchemaHelper::colStyleIbuMax("ibu_max");
QString DatabaseSchemaHelper::colStyleColorMin("color_min");
QString DatabaseSchemaHelper::colStyleColorMax("color_max");
QString DatabaseSchemaHelper::colStyleAbvMin("abv_min");
QString DatabaseSchemaHelper::colStyleAbvMax("abv_max");
QString DatabaseSchemaHelper::colStyleCarbMin("carb_min");
QString DatabaseSchemaHelper::colStyleCarbMax("carb_max");
QString DatabaseSchemaHelper::colStyleNotes("notes");
QString DatabaseSchemaHelper::colStyleProfile("profile");
QString DatabaseSchemaHelper::colStyleIngredients("ingredients");
QString DatabaseSchemaHelper::colStyleExamples("examples");

QString DatabaseSchemaHelper::tableYeast("yeast");
QString DatabaseSchemaHelper::colYeastType("ytype");
QString DatabaseSchemaHelper::colYeastForm("form");
QString DatabaseSchemaHelper::colYeastAmount("amount");
QString DatabaseSchemaHelper::colYeastAmountIsWeight("amount_is_weight");
QString DatabaseSchemaHelper::colYeastLab("laboratory");
QString DatabaseSchemaHelper::colYeastProductId("product_id");
QString DatabaseSchemaHelper::colYeastTempMin("min_temperature");
QString DatabaseSchemaHelper::colYeastTempMax("max_temperature");
QString DatabaseSchemaHelper::colYeastFlocc("flocculation");
QString DatabaseSchemaHelper::colYeastAtten("attenuation");
QString DatabaseSchemaHelper::colYeastNotes("notes");
QString DatabaseSchemaHelper::colYeastBestFor("best_for");
QString DatabaseSchemaHelper::colYeastRecultures("times_cultured");
QString DatabaseSchemaHelper::colYeastReuseMax("max_reuse");
QString DatabaseSchemaHelper::colYeastSecondary("add_to_secondary");

QString DatabaseSchemaHelper::tableWater("water");
QString DatabaseSchemaHelper::colWaterAmount("amount");
QString DatabaseSchemaHelper::colWaterCa("calcium");
QString DatabaseSchemaHelper::colWaterBicarb("bicarbonate");
QString DatabaseSchemaHelper::colWaterSulfate("sulfate");
QString DatabaseSchemaHelper::colWaterCl("chloride");
QString DatabaseSchemaHelper::colWaterNa("sodium");
QString DatabaseSchemaHelper::colWaterMg("magnesium");
QString DatabaseSchemaHelper::colWaterPh("ph");
QString DatabaseSchemaHelper::colWaterNotes("notes");

QString DatabaseSchemaHelper::tableMash("mash");
QString DatabaseSchemaHelper::colMashGrainTemp("grain_temp");
QString DatabaseSchemaHelper::colMashNotes("notes");
QString DatabaseSchemaHelper::colMashTunTemp("tun_temp");
QString DatabaseSchemaHelper::colMashSpargeTemp("sparge_temp");
QString DatabaseSchemaHelper::colMashPh("ph");
QString DatabaseSchemaHelper::colMashTunWeight("tun_weight");
QString DatabaseSchemaHelper::colMashTunSpecificHeat("tun_specific_heat");
QString DatabaseSchemaHelper::colMashEquipAdjust("equip_adjust");

QString DatabaseSchemaHelper::tableMashStep("mashstep");
QString DatabaseSchemaHelper::colMashStepType("mstype");
QString DatabaseSchemaHelper::colMashStepInfAmount("infuse_amount");
QString DatabaseSchemaHelper::colMashStepTemp("step_temp");
QString DatabaseSchemaHelper::colMashStepTime("step_time");
QString DatabaseSchemaHelper::colMashStepRampTime("ramp_time");
QString DatabaseSchemaHelper::colMashStepEndTemp("end_temp");
QString DatabaseSchemaHelper::colMashStepInfTemp("infuse_temp");
QString DatabaseSchemaHelper::colMashStepDecAmount("decoction_amount");
QString DatabaseSchemaHelper::colMashStepMashId("mash_id");
QString DatabaseSchemaHelper::colMashStepNumber("step_number");
   
// Brewnotes table
QString DatabaseSchemaHelper::tableBrewnote("brewnote");
QString DatabaseSchemaHelper::colBNoteBrewDate("brewDate");
QString DatabaseSchemaHelper::colBNoteFermentDate("fermentDate");
QString DatabaseSchemaHelper::colBNoteSg("sg");
QString DatabaseSchemaHelper::colBNoteBkVolume("volume_into_bk");
QString DatabaseSchemaHelper::colBNoteStrikeTemp("strike_temp");
QString DatabaseSchemaHelper::colBNoteFinalMashTemp("mash_final_temp");
QString DatabaseSchemaHelper::colBNoteOg("og");
QString DatabaseSchemaHelper::colBNotePostboilVolume("post_boil_volume");
QString DatabaseSchemaHelper::colBNoteFermenterVolume("volume_into_fermenter");
QString DatabaseSchemaHelper::colBNotePitchTemp("pitch_temp");
QString DatabaseSchemaHelper::colBNoteFg("fg");
QString DatabaseSchemaHelper::colBNoteBkEff("eff_into_bk");
QString DatabaseSchemaHelper::colBNoteAbv("abv");
QString DatabaseSchemaHelper::colBNotePredOg("predicted_og");
QString DatabaseSchemaHelper::colBNoteEff("brewhouse_eff");
QString DatabaseSchemaHelper::colBNotePredAbv("predicted_abv");
QString DatabaseSchemaHelper::colBNoteProjBoilGrav("projected_boil_grav");
QString DatabaseSchemaHelper::colBNoteProjStrikeTemp("projected_strike_temp");
QString DatabaseSchemaHelper::colBNoteProjFinTemp("projected_fin_temp");
QString DatabaseSchemaHelper::colBNoteProjFinMashTemp("projected_mash_fin_temp");
QString DatabaseSchemaHelper::colBNoteProjBkVol("projected_vol_into_bk");
QString DatabaseSchemaHelper::colBNoteProjOg("projected_og");
QString DatabaseSchemaHelper::colBNoteProjFermVol("projected_vol_into_ferm");
QString DatabaseSchemaHelper::colBNoteProjFg("projected_fg");
QString DatabaseSchemaHelper::colBNoteProjEff("projected_eff");
QString DatabaseSchemaHelper::colBNoteProjAbv("projected_abv");
QString DatabaseSchemaHelper::colBNoteProjAtten("projected_atten");
QString DatabaseSchemaHelper::colBNoteProjPoints("projected_points");
QString DatabaseSchemaHelper::colBNoteProjFermPoints("projected_ferm_points");
QString DatabaseSchemaHelper::colBNoteBoilOff("boil_off");
QString DatabaseSchemaHelper::colBNoteFinalVolume("final_volume");
QString DatabaseSchemaHelper::colBNoteNotes("notes");
QString DatabaseSchemaHelper::colBNoteRecipeId("recipe_id");

QString DatabaseSchemaHelper::tableInstruction("instruction");
QString DatabaseSchemaHelper::colInsDirections("directions");
QString DatabaseSchemaHelper::colInsHasTimer("hasTimer");
QString DatabaseSchemaHelper::colInsTimerVal("timerValue");
QString DatabaseSchemaHelper::colInsCompleted("completed");
QString DatabaseSchemaHelper::colInsInterval("interval");

QString DatabaseSchemaHelper::tableRecipe("recipe");
QString DatabaseSchemaHelper::colRecType("type");
QString DatabaseSchemaHelper::colRecBrewer("brewer");
QString DatabaseSchemaHelper::colRecAsstBrewer("assistant_brewer");
QString DatabaseSchemaHelper::colRecBatchSize("batch_size");
QString DatabaseSchemaHelper::colRecBoilSize("boil_size");
QString DatabaseSchemaHelper::colRecBoilTime("boil_time");
QString DatabaseSchemaHelper::colRecEff("efficiency");
QString DatabaseSchemaHelper::colRecOg("og");
QString DatabaseSchemaHelper::colRecFg("fg");
QString DatabaseSchemaHelper::colRecFermStages("fermentation_stages");
QString DatabaseSchemaHelper::colRecPrimAge("primary_age");
QString DatabaseSchemaHelper::colRecPrimTemp("primary_temp");
QString DatabaseSchemaHelper::colRecSecAge("secondary_age");
QString DatabaseSchemaHelper::colRecSecTemp("secondary_temp");
QString DatabaseSchemaHelper::colRecTerAge("tertiary_age");
QString DatabaseSchemaHelper::colRecTerTemp("tertiary_temp");
QString DatabaseSchemaHelper::colRecAge("age");
QString DatabaseSchemaHelper::colRecAgeTemp("age_temp");
QString DatabaseSchemaHelper::colRecDate("date");
QString DatabaseSchemaHelper::colRecCarbVol("carb_volume");
QString DatabaseSchemaHelper::colRecForceCarb("forced_carb");
QString DatabaseSchemaHelper::colRecPrimSug("priming_sugar_name");
QString DatabaseSchemaHelper::colRecCarbTemp("carbonationTemp_c");
QString DatabaseSchemaHelper::colRecPrimSugEquiv("priming_sugar_equiv");
QString DatabaseSchemaHelper::colRecKegPrimFact("keg_priming_factor");
QString DatabaseSchemaHelper::colRecNotes("notes");
QString DatabaseSchemaHelper::colRecTasteNotes("taste_notes");
QString DatabaseSchemaHelper::colRecTasteRating("taste_rating");
QString DatabaseSchemaHelper::colRecStyleId("style_id");
QString DatabaseSchemaHelper::colRecMashId("mash_id");
QString DatabaseSchemaHelper::colRecEquipId("equipment_id");

QString DatabaseSchemaHelper::tableBtEquipment("bt_equipment");

QString DatabaseSchemaHelper::tableBtFermentable("bt_fermentable");

QString DatabaseSchemaHelper::tableBtHop("bt_hop");

QString DatabaseSchemaHelper::tableBtMisc("bt_misc");

QString DatabaseSchemaHelper::tableBtStyle("bt_style");

QString DatabaseSchemaHelper::tableBtYeast("bt_yeast");

QString DatabaseSchemaHelper::tableBtWater("bt_water");

QString DatabaseSchemaHelper::tableFermInRec("fermentable_in_recipe");

QString DatabaseSchemaHelper::tableHopInRec("hop_in_recipe");

QString DatabaseSchemaHelper::tableMiscInRec("misc_in_recipe");

QString DatabaseSchemaHelper::tableWaterInRec("water_in_recipe");

QString DatabaseSchemaHelper::tableYeastInRec("yeast_in_recipe");

QString DatabaseSchemaHelper::tableInsInRec("instruction_in_recipe");

QString DatabaseSchemaHelper::tableEquipChildren("equipment_children");

QString DatabaseSchemaHelper::tableFermChildren("fermentable_children");

QString DatabaseSchemaHelper::tableHopChildren("hop_children");

QString DatabaseSchemaHelper::tableMiscChildren("misc_children");

QString DatabaseSchemaHelper::tableRecChildren("recipe_children");

QString DatabaseSchemaHelper::tableStyleChildren("style_children");

QString DatabaseSchemaHelper::tableWaterChildren("water_children");

QString DatabaseSchemaHelper::tableYeastChildren("yeast_children");

QString DatabaseSchemaHelper::tableFermInventory("fermentable_in_inventory");

QString DatabaseSchemaHelper::tableHopInventory("hop_in_inventory");

QString DatabaseSchemaHelper::tableMiscInventory("misc_in_inventory");

QString DatabaseSchemaHelper::tableYeastInventory("yeast_in_inventory");

// Default namespace hides functions from everything outside this file.
namespace {
   
   QString FOREIGNKEY( QString const& column, QString const& foreignTable )
   {
      return QString("FOREIGN KEY(%1) REFERENCES %2(id)").arg(column).arg(foreignTable);
   }
   
   QString childrenTable( QString const& foreignTable )
   {
      return QString() +
         "id INTEGER PRIMARY KEY autoincrement," +
         "parent_id INTEGER," +
         "child_id INTEGER," +
         FOREIGNKEY("parent_id", foreignTable) + "," +
         FOREIGNKEY("child_id", foreignTable);
   }
}

bool DatabaseSchemaHelper::create(QSqlDatabase db)
{
   //--------------------------------------------------------------------------
   // NOTE: if you edit this function, increment dbVersion and edit
   // migrateNext() appropriately.
   //--------------------------------------------------------------------------

   // NOTE: none of the BeerXML property names should EVER change. This is to
   //       ensure backwards compatability when rolling out ingredient updates to
   //       old versions.

   // NOTE: deleted=1 means the ingredient is "deleted" and should not be shown in
   //                 any list.
   //       deleted=0 means it isn't deleted and may or may not be shown.
   //       display=1 means the ingredient should be shown in a list, available to
   //                 be put into a recipe.
   //       display=0 means the ingredient is in a recipe already and should not
   //                 be shown in a list, available to be put into a recipe.
   
   QSqlQuery q(db);
   bool ret = true;
   
   // Start transaction
   bool hasTransaction = db.transaction();
   
   // Settings table
   ret &= q.exec(
      CREATETABLE + SEP + tableSettings + "(" +
      id + "," +
      colSettingsVersion            + SEP + TYPEINTEGER + "," +
      colSettingsRepopulateChildren + SEP + TYPEINTEGER +
      ")"
   );
   ret &= q.exec(
      INSERTINTO + SEP + tableSettings + QString(" VALUES(1,%1,1)").arg(dbVersion)
   );
   
   // Equipment
   ret &= q.exec(
      CREATETABLE + SEP + tableEquipment + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colEquipBoilSize        + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipBatchSize       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipTunVolume       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipTunWeight       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipTunSpecificHeat + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipTopUpWater      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipTrubChillerLoss + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipEvapRate        + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipBoilTime        + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipCalcBoilVolume  + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colEquipLauterDeadspace + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipTopUpKettle     + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipHopUtilization  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipNotes           + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      // Our BeerXML extensions
      colEquipRealEvapRate + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colEquipBoilingPoint + SEP + TYPEREAL + SEP + DEFAULT + " 100.0" + "," +
      colEquipAbsorption   + SEP + TYPEREAL + SEP + DEFAULT + " 1.085" + "," +
      // Metadata
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Fermentable
   ret &= q.exec(
      CREATETABLE + SEP + tableFermentable + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colFermFtype          + SEP + TYPETEXT + SEP + DEFAULT + " 'Grain'" + "," +
      colFermAmount         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colFermYield          + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colFermColor          + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colFermAddAfterBoil   + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colFermOrigin         + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colFermSupplier       + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colFermNotes          + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colFermCoarseFineDiff + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colFermMoisture       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colFermDiastaticPower + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colFermProtein        + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colFermMaxInBatch     + SEP + TYPEREAL + SEP + DEFAULT + " 100.0" + "," +
      colFermRecommendMash  + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colFermIsMashed       + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colFermIbuGalLb       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      // Display stuff---------------------------------------------------------
      displayUnit + "," +
      displayScale + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Hop
   ret &= q.exec(
      CREATETABLE + SEP + tableHop + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colHopAlpha         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colHopAmount        + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colHopUse           + SEP + TYPETEXT + SEP + DEFAULT + " 'Boil'" + "," +
      colHopTime          + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colHopNotes         + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colHopHtype         + SEP + TYPETEXT + SEP + DEFAULT + " 'Both'" + "," +
      colHopForm          + SEP + TYPETEXT + SEP + DEFAULT + " 'Pellet'" + "," +
      colHopBeta          + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colHopHsi           + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colHopOrigin        + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colHopSubstitutes   + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colHopHumulene      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colHopCaryophyllene + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colHopCohumulone    + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colHopMyrcene       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      // Display stuff---------------------------------------------------------
      displayUnit + "," +
      displayScale + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Misc
   ret &= q.exec(
      CREATETABLE + SEP + tableMisc + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colMiscMtype          + SEP + TYPETEXT + SEP + DEFAULT + " 'Other'" + "," +
      colMiscUse            + SEP + TYPETEXT + SEP + DEFAULT + " 'Boil'" + "," +
      colMiscTime           + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colMiscAmount         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colMiscAmountIsWeight + SEP + TYPEINTEGER + SEP + DEFAULT + " 1" + "," +
      colMiscUseFor         + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colMiscNotes          + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      // Display stuff---------------------------------------------------------
      displayUnit + "," +
      displayScale + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Style
   ret &= q.exec(
      CREATETABLE + SEP + tableStyle + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colStyleType        + SEP + TYPETEXT + SEP + DEFAULT + " 'Ale'" + "," +
      colStyleCat         + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colStyleCatNum      + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colStyleLetter      + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colStyleGuide       + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colStyleOgMin       + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colStyleOgMax       + SEP + TYPEREAL + SEP + DEFAULT + " 1.1" + "," +
      colStyleFgMin       + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colStyleFgMax       + SEP + TYPEREAL + SEP + DEFAULT + " 1.1" + "," +
      colStyleIbuMin      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colStyleIbuMax      + SEP + TYPEREAL + SEP + DEFAULT + " 100.0" + "," +
      colStyleColorMin    + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colStyleColorMax    + SEP + TYPEREAL + SEP + DEFAULT + " 100.0" + "," +
      colStyleAbvMin      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colStyleAbvMax      + SEP + TYPEREAL + SEP + DEFAULT + " 100.0" + "," +
      colStyleCarbMin     + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colStyleCarbMax     + SEP + TYPEREAL + SEP + DEFAULT + " 100.0" + "," +
      colStyleNotes       + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colStyleProfile     + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colStyleIngredients + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colStyleExamples    + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Yeast
   ret &= q.exec(
      CREATETABLE + SEP + tableYeast + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colYeastType           + SEP + TYPETEXT + SEP + DEFAULT + " 'Ale'" + "," +
      colYeastForm           + SEP + TYPETEXT + SEP + DEFAULT + " 'Liquid'" + "," +
      colYeastAmount         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colYeastAmountIsWeight + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colYeastLab            + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colYeastProductId      + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colYeastTempMin        + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colYeastTempMax        + SEP + TYPEREAL + SEP + DEFAULT + " 32.0" + "," +
      colYeastFlocc          + SEP + TYPETEXT + SEP + DEFAULT + " 'Medium'" + "," +
      colYeastAtten          + SEP + TYPEREAL + SEP + DEFAULT + " 75.0" + "," +
      colYeastNotes          + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colYeastBestFor        + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colYeastRecultures     + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colYeastReuseMax       + SEP + TYPEINTEGER + SEP + DEFAULT + " 10" + "," +
      colYeastSecondary      + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      // Display stuff---------------------------------------------------------
      displayUnit + "," +
      displayScale + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Water
   ret &= q.exec(
      CREATETABLE + SEP + tableWater + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colWaterAmount  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colWaterCa      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colWaterBicarb  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colWaterSulfate + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colWaterCl      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colWaterNa      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colWaterMg      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colWaterPh      + SEP + TYPEREAL + SEP + DEFAULT + " 7.0" + "," +
      colWaterNotes   + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Mash
   ret &= q.exec(
      CREATETABLE + SEP + tableMash + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colMashGrainTemp       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colMashNotes           + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colMashTunTemp         + SEP + TYPEREAL + SEP + DEFAULT + " 20.0" + "," +
      colMashSpargeTemp      + SEP + TYPEREAL + SEP + DEFAULT + " 74.0" + "," +
      colMashPh              + SEP + TYPEREAL + SEP + DEFAULT + " 7.0" + "," +
      colMashTunWeight       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colMashTunSpecificHeat + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colMashEquipAdjust     + SEP + TYPEINTEGER + SEP + DEFAULT + " 1" + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // MashStep
   ret &= q.exec(
      CREATETABLE + SEP + tableMashStep + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colMashStepType      + SEP + 
      colMashStepInfAmount + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colMashStepTemp      + SEP + TYPEREAL + SEP + DEFAULT + " 67.0" + "," +
      colMashStepTime      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colMashStepRampTime  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colMashStepEndTemp   + SEP + TYPEREAL + SEP + DEFAULT + " 67.0" + "," +
      colMashStepInfTemp   + SEP + TYPEREAL + SEP + DEFAULT + " 67.0" + "," +
      colMashStepDecAmount + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      // Relational data-------------------------------------------------------
      colMashStepMashId    + SEP + TYPEINTEGER + "," +
      colMashStepNumber    + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      FOREIGNKEY(colMashStepMashId, tableMash) + "," +
      // Display stuff---------------------------------------------------------
      displayUnit + "," +
      displayScale + "," +
      displayTempUnit + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Brewnote
   ret &= q.exec(
      CREATETABLE + SEP + tableBrewnote + SEP + "(" +
      id + "," +
      colBNoteBrewDate        + SEP + TYPEDATETIME + SEP + DEFAULT + " CURRENT_DATETIME" + "," +
      colBNoteFermentDate     + SEP + TYPEDATETIME + SEP + DEFAULT + " CURRENT_DATETIME" + "," +
      colBNoteSg              + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colBNoteBkVolume        + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteStrikeTemp      + SEP + TYPEREAL + SEP + DEFAULT + " 70.0" + "," +
      colBNoteFinalMashTemp   + SEP + TYPEREAL + SEP + DEFAULT + " 67.0" + "," +
      colBNoteOg              + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colBNotePostboilVolume  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteFermenterVolume + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNotePitchTemp       + SEP + TYPEREAL + SEP + DEFAULT + " 20.0" + "," +
      colBNoteFg              + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colBNoteBkEff           + SEP + TYPEREAL + SEP + DEFAULT + " 70.0" + "," +
      colBNoteAbv             + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNotePredOg          + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colBNoteEff             + SEP + TYPEREAL + SEP + DEFAULT + " 70.0" + "," +
      colBNotePredAbv         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteProjBoilGrav    + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colBNoteProjStrikeTemp  + SEP + TYPEREAL + SEP + DEFAULT + " 70.0" + "," +
      colBNoteProjFinTemp     + SEP + TYPEREAL + SEP + DEFAULT + " 67.0" + "," +
      colBNoteProjFinMashTemp + SEP + TYPEREAL + SEP + DEFAULT + " 67.0" + "," +
      colBNoteProjBkVol       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteProjOg          + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colBNoteProjFermVol     + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteProjFg          + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colBNoteProjEff         + SEP + TYPEREAL + SEP + DEFAULT + " 70.0" + "," +
      colBNoteProjAbv         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteProjAtten       + SEP + TYPEREAL + SEP + DEFAULT + " 75.0" + "," +
      colBNoteProjPoints      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteProjFermPoints  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteBoilOff         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteFinalVolume     + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colBNoteNotes           + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      // Relational data-------------------------------------------------------
      colBNoteRecipeId + SEP + TYPEINTEGER + "," +
      FOREIGNKEY(colBNoteRecipeId, tableRecipe) + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // Instruction
   ret &= q.exec(
      CREATETABLE + SEP + tableInstruction + SEP + "(" +
      id + "," +
      name + "," +
      colInsDirections + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colInsHasTimer   + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colInsTimerVal   + SEP + TYPETEXT + SEP + DEFAULT + " '00:00:00'" + "," +
      colInsCompleted  + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colInsInterval   + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display +
      // instructions aren't displayed in trees, and get no folder
      ")"
   );
   
   // Recipe
   ret &= q.exec(
      CREATETABLE + SEP + tableRecipe + SEP + "(" +
      id + "," +
      // BeerXML properties----------------------------------------------------
      name + "," +
      colRecType         + SEP + TYPETEXT + SEP + DEFAULT + " 'All Grain'" + "," +
      colRecBrewer       + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colRecAsstBrewer   + SEP + TYPETEXT + SEP + DEFAULT + " 'Brewtarget: free beer software'" + "," +
      colRecBatchSize    + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colRecBoilSize     + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colRecBoilTime     + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colRecEff          + SEP + TYPEREAL + SEP + DEFAULT + " 70.0" + "," +
      colRecOg           + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colRecFg           + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colRecFermStages   + SEP + TYPEINTEGER + SEP + DEFAULT + " 1" + "," +
      colRecPrimAge      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colRecPrimTemp     + SEP + TYPEREAL + SEP + DEFAULT + " 20.0" + "," +
      colRecSecAge       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colRecSecTemp      + SEP + TYPEREAL + SEP + DEFAULT + " 20.0" + "," +
      colRecTerAge       + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colRecTerTemp      + SEP + TYPEREAL + SEP + DEFAULT + " 20.0" + "," +
      colRecAge          + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colRecAgeTemp      + SEP + TYPEREAL + SEP + DEFAULT + " 20.0" + "," +
      colRecDate         + SEP + TYPEDATETIME + SEP + DEFAULT + " CURRENT_DATETIME" + "," +
      colRecCarbVol      + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      colRecForceCarb    + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      colRecPrimSug      + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colRecCarbTemp     + SEP + TYPEREAL + SEP + DEFAULT + " 20.0" + "," +
      colRecPrimSugEquiv + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colRecKegPrimFact  + SEP + TYPEREAL + SEP + DEFAULT + " 1.0" + "," +
      colRecNotes        + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colRecTasteNotes   + SEP + TYPETEXT + SEP + DEFAULT + " ''" + "," +
      colRecTasteRating  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      // Relational data-------------------------------------------------------
      colRecStyleId + SEP + TYPEINTEGER + "," +
      colRecMashId  + SEP + TYPEINTEGER + "," +
      colRecEquipId + SEP + TYPEINTEGER + "," +
      FOREIGNKEY(colRecStyleId, tableStyle) + "," +
      FOREIGNKEY(colRecMashId, tableMash) + "," +
      FOREIGNKEY(colRecEquipId, tableEquipment) + "," +
      // Metadata--------------------------------------------------------------
      deleted + "," +
      display + "," +
      folder +
      ")"
   );
   
   // The following bt_* tables simply point to ingredients provided by brewtarget.
   // This is to make updating and pushing new ingredients easy.
   // NOTE: they MUST be named bt_<table>, where <table> is the table name that
   // they refer to, and they MUST contain fields 'id' and '<table>_id'.
   
   ret &= q.exec(
      CREATETABLE + SEP + tableBtEquipment + SEP + "(" +
      id + "," +
      "equipment_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("equipment_id", tableEquipment) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableBtFermentable + SEP + "(" +
      id + "," +
      "fermentable_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("fermentable_id", tableFermentable) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableBtHop + SEP + "(" +
      id + "," +
      "hop_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("hop_id", tableHop) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableBtMisc + SEP + "(" +
      id + "," +
      "misc_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("misc_id", tableMisc) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableBtStyle + SEP + "(" +
      id + "," +
      "style_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("style_id", tableStyle) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableBtYeast + SEP + "(" +
      id + "," +
      "yeast_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("yeast_id", tableYeast) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableBtWater + SEP + "(" +
      id + "," +
      "water_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("water_id", tableWater) +
      ")"
   );
   
   // Recipe relational tables
   ret &= q.exec(
      CREATETABLE + SEP + tableFermInRec + SEP + "(" +
      id + "," +
      "fermentable_id" + SEP + TYPEINTEGER + "," +
      "recipe_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("fermentable_id", tableFermentable) + "," +
      FOREIGNKEY("recipe_id", tableRecipe) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableHopInRec + SEP + "(" +
      id + "," +
      "hop_id" + SEP + TYPEINTEGER + "," +
      "recipe_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("hop_id", tableHop) + "," +
      FOREIGNKEY("recipe_id", tableRecipe) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableMiscInRec + SEP + "(" +
      id + "," +
      "misc_id" + SEP + TYPEINTEGER + "," +
      "recipe_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("misc_id", tableMisc) + "," +
      FOREIGNKEY("recipe_id", tableRecipe) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableWaterInRec + SEP + "(" +
      id + "," +
      "water_id" + SEP + TYPEINTEGER + "," +
      "recipe_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("water_id", tableWater) + "," +
      FOREIGNKEY("recipe_id", tableRecipe) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableYeastInRec + SEP + "(" +
      id + "," +
      "yeast_id" + SEP + TYPEINTEGER + "," +
      "recipe_id" + SEP + TYPEINTEGER + "," +
      FOREIGNKEY("yeast_id", tableYeast) + "," +
      FOREIGNKEY("recipe_id", tableRecipe) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableInsInRec + SEP + "(" +
      id + "," +
      "instruction_id" + SEP + TYPEINTEGER + "," +
      "recipe_id" + SEP + TYPEINTEGER + "," +
      // instruction_number is the order of the instruction in the recipe.
      "instruction_number" + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      FOREIGNKEY("instruction_id", tableInstruction) + "," +
      FOREIGNKEY("recipe_id", tableRecipe) +
      ")"
   );
   
   // This trigger automatically makes a new instruction in a recipe the last.
   ret &= q.exec( QString() +
      "CREATE TRIGGER inc_ins_num AFTER INSERT ON instruction_in_recipe " +
      "BEGIN " +
         "UPDATE instruction_in_recipe SET instruction_number = " +
           "(SELECT max(instruction_number) FROM instruction_in_recipe WHERE recipe_id = new.recipe_id) + 1 " +
           "WHERE rowid = new.rowid; " +
      "END"
   );
   
   // This trigger automatically decrements all instruction numbers greater than the one
   // deleted in the given recipe.
   ret &= q.exec( QString() +
      "CREATE TRIGGER dec_ins_num AFTER DELETE ON instruction_in_recipe " +
      "BEGIN "
         "UPDATE instruction_in_recipe SET instruction_number = instruction_number - 1 " +
            "WHERE recipe_id = old.recipe_id AND instruction_id > old.instruction_id; " +
      "END"
   );
   
   // Ingredient inheritance tables============================================
   
   ret &= q.exec(
      CREATETABLE + SEP + tableEquipChildren + SEP + "(" +
      childrenTable(tableEquipment) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableFermChildren + SEP + "(" +
      childrenTable(tableFermentable) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableHopChildren + SEP + "(" +
      childrenTable(tableHop) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableMiscChildren + SEP + "(" +
      childrenTable(tableMisc) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableRecChildren + SEP + "(" +
      childrenTable(tableRecipe) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableStyleChildren + SEP + "(" +
      childrenTable(tableStyle) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableWaterChildren + SEP + "(" +
      childrenTable(tableWater) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableYeastChildren + SEP + "(" +
      childrenTable(tableYeast) +
      ")"
   );
   
   // Inventory tables=========================================================
   
   ret &= q.exec(
      CREATETABLE + SEP + tableFermInventory + SEP + "(" +
      id + "," +
      "fermentable_id" + SEP + TYPEINTEGER + SEP + UNIQUE + "," +
      "amount"         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      FOREIGNKEY("fermentable_id", tableFermentable) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableHopInventory + SEP + "(" +
      id + "," +
      "hop_id" + SEP + TYPEINTEGER + SEP + UNIQUE + "," +
      "amount" + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      FOREIGNKEY("hop_id", tableHop) +
      ")"
   );
   
   ret &= q.exec(
      CREATETABLE + SEP + tableMiscInventory + SEP + "(" +
      id + "," +
      "misc_id" + SEP + TYPEINTEGER + SEP + UNIQUE + "," +
      "amount"  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
      FOREIGNKEY("misc_id", tableMisc) +
      ")"
   );
   
   
   // For yeast, homebrewers don't usually keep stores of yeast. They keep
   // packets or vials or some other type of discrete integer quantity. So, I
   // don't know how useful a real-valued inventory amount would be for yeast.
   ret &= q.exec(
      CREATETABLE + SEP + tableYeastInventory + SEP + "(" +
      id + "," +
      "yeast_id" + SEP + TYPEINTEGER + SEP + UNIQUE + "," +
      "quanta"   + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
      FOREIGNKEY("yeast_id", tableYeast) +
      ")"
   );
   
   // Commit transaction
   if( hasTransaction )
      ret &= db.commit();
   
   return ret;
}

bool DatabaseSchemaHelper::migrateNext(int oldVersion, QSqlDatabase db)
{
   QSqlQuery q(db);
   bool ret = true;

   // NOTE: use this to debug your migration
#define CHECKQUERY if(!ret) qDebug() << QString("ERROR: %1\nQUERY: %2").arg(q.lastError().text()).arg(q.lastQuery());

   // NOTE: Add a new case when adding a new schema change
   switch(oldVersion)
   {
      case 1: // == '2.0.0'
         // Add settings table
         ret &= q.exec(
            CREATETABLE + SEP + tableSettings + "(" +
            id + "," +
            colSettingsVersion + SEP + TYPETEXT +
            ")"
         );
         
         // Add "projected_ferm_points" to brewnote table
         ret &= q.exec(
            ALTERTABLE + SEP + tableBrewnote + SEP +
            ADDCOLUMN + SEP + "projected_ferm_points" + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"
         );
         
         ret &= q.exec(
            UPDATE + SEP + tableBrewnote + SEP +
            SET + SEP + "projected_ferm_points = -1.0"
         );
         
         // Update version to 2.0.2
         ret &= q.exec(
            INSERTINTO + SEP + tableSettings + " VALUES(1,'2.0.2')"
         );
         
         break;
         
      case 2: // == '2.0.2'
         
         // Add 'folder' column to some tables
         ret &= q.exec(
            ALTERTABLE + SEP + tableEquipment + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableFermentable + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableHop + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableMisc + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableStyle + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableYeast + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableWater + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableMash + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableBrewnote + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         ret &= q.exec(
            ALTERTABLE + SEP + tableRecipe + SEP +
            ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
         );
         
         // Put the "Bt:.*" recipes into /brewtarget folder
         ret &= q.exec(
            UPDATE + SEP + tableRecipe + SEP +
            SET + SEP + "folder='/brewtarget' WHERE name LIKE 'Bt:%'"
         );
         
         // Update version to 2.1.0
         ret &= q.exec(
            UPDATE + SEP + tableSettings + SEP +
            SET + SEP + colSettingsVersion + "='2.1.0' WHERE id=1"
         );
         
         // Used to trigger the code to populate the ingredient inheritance tables
         ret &= q.exec(
            ALTERTABLE + SEP + tableSettings + SEP +
            ADDCOLUMN + SEP + "repopulateChildrenOnNextStart" + SEP + TYPEINTEGER
         );
         
         ret &= q.exec(
            UPDATE + SEP + tableSettings + SEP +
            SET + SEP + "repopulateChildrenOnNextStart=1"
         );
         
         // Drop and re-create children tables with new UNIQUE requirement
         ret &= q.exec(
            DROPTABLE + SEP + tableEquipChildren
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableFermChildren
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableHopChildren
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableMiscChildren
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableRecChildren
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableStyleChildren
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableWaterChildren
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableYeastChildren
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableEquipChildren + SEP + "(" +
            childrenTable(tableEquipment) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableFermChildren + SEP + "(" +
            childrenTable(tableFermentable) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableHopChildren + SEP + "(" +
            childrenTable(tableHop) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableMiscChildren + SEP + "(" +
            childrenTable(tableMisc) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableRecChildren + SEP + "(" +
            childrenTable(tableRecipe) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableStyleChildren + SEP + "(" +
            childrenTable(tableStyle) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableWaterChildren + SEP + "(" +
            childrenTable(tableWater) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableYeastChildren + SEP + "(" +
            childrenTable(tableYeast) +
            ")"
         );
         
         // Drop and re-create inventory tables with new UNIQUE requirement
         ret &= q.exec(
            DROPTABLE + SEP + tableFermInventory
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableHopInventory
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableMiscInventory
         );
         
         ret &= q.exec(
            DROPTABLE + SEP + tableYeastInventory
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableFermInventory + SEP + "(" +
            id + "," +
            "fermentable_id" + SEP + TYPEINTEGER + SEP + UNIQUE + "," +
            "amount"         + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
            FOREIGNKEY("fermentable_id", tableFermentable) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableHopInventory + SEP + "(" +
            id + "," +
            "hop_id" + SEP + TYPEINTEGER + SEP + UNIQUE + "," +
            "amount" + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
            FOREIGNKEY("hop_id", tableHop) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableMiscInventory + SEP + "(" +
            id + "," +
            "misc_id" + SEP + TYPEINTEGER + SEP + UNIQUE + "," +
            "amount"  + SEP + TYPEREAL + SEP + DEFAULT + " 0.0" + "," +
            FOREIGNKEY("misc_id", tableMisc) +
            ")"
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableYeastInventory + SEP + "(" +
            id + "," +
            "yeast_id" + SEP + TYPEINTEGER + SEP + UNIQUE + "," +
            "quanta"   + SEP + TYPEINTEGER + SEP + DEFAULT + " 0" + "," +
            FOREIGNKEY("yeast_id", tableYeast) +
            ")"
         );
         
         break;
         
      case 3: // == '2.1.0'
         
         // Save old settings
         ret &= q.exec(
            "CREATE TEMP TABLE oldsettings AS SELECT * FROM " + tableSettings
         );
         
         // Drop the old settings with text version, and create new table
         // with intever version.
         ret &= q.exec(
            DROPTABLE + SEP + tableSettings
         );
         
         ret &= q.exec(
            CREATETABLE + SEP + tableSettings + "(" +
            id + "," +
            colSettingsVersion            + SEP + TYPEINTEGER + "," +
            colSettingsRepopulateChildren + SEP + TYPEINTEGER +
            ")"
         );
         
         // Update version to 4, saving other settings
         ret &= q.exec(
            INSERTINTO + SEP + tableSettings +
            QString(" (id,%1,%2)").arg(colSettingsVersion).arg(colSettingsRepopulateChildren) + " " +
            QString("SELECT 1, 4, %1 FROM oldsettings").arg(colSettingsRepopulateChildren)
         );
         
         // Cleanup
         ret &= q.exec(
            DROPTABLE + SEP + "oldsettings"
         );
         
         break;
         
      default:
         Brewtarget::logE(QString("Unknown version %1").arg(oldVersion));
         return false;
   }
   
   // Set the database version
   if( oldVersion > 3 )
   {
      ret &= q.exec(
         UPDATE + SEP + tableSettings +
         " SET " + colSettingsVersion + "=" + (oldVersion+1) + " WHERE id=1"
      );
   }
   
   return ret;
#undef CHECKQUERY
}

bool DatabaseSchemaHelper::migrate(int oldVersion, int newVersion, QSqlDatabase db)
{
   if( oldVersion >= newVersion || newVersion > dbVersion )
   {
      qDebug() << QString("DatabaseSchemaHelper::migrate(%1, %2): You are an imbecile").arg(oldVersion).arg(newVersion);
      return false;
   }
   
   bool ret = true;
   
   // Start a transaction
   db.transaction();
   
   for( ; oldVersion < newVersion && ret; ++oldVersion )
      ret &= migrateNext(oldVersion, db);
   
   // If any statement failed to execute, rollback database to last good state.
   if( ret )
      ret &= db.commit();
   else
   {
      Brewtarget::logE("Rolling back");
      db.rollback();
   }
   
   return ret;
}

int DatabaseSchemaHelper::currentVersion(QSqlDatabase db)
{
   QVariant ver;
   QSqlQuery q(
      SELECT + SEP + colSettingsVersion + " FROM " + tableSettings + " WHERE id=1",
      db
   );
   
   // No settings table in version 2.0.0
   if( q.next() )
   {
      int field = q.record().indexOf(colSettingsVersion);
      ver = q.value(field);
   }
   else
      ver = QString("2.0.0");
   
   // Get the string before we kill it by convert()-ing
   QString stringVer( ver.toString() );
   
   // Initially, versioning was done with strings, so we need to convert
   // the old version strings to integer versions
   if( ver.convert(QVariant::Int) )
      return ver.toInt();
   else
   {
      if( stringVer == "2.0.0" )
         return 1;
      else if( stringVer == "2.0.2" )
         return 2;
      else if( stringVer == "2.1.0" )
         return 3;
   }
   
   Brewtarget::logE("Could not find database version");
   return -1;
}