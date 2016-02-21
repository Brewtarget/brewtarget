/*
 * DatabaseSchemaHelper.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip G. Lee <rocketman768@gmail.com>
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

#include "DatabaseSchemaHelper.h"

#include "brewtarget.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <QSqlError>

const int DatabaseSchemaHelper::dbVersion = 6;

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
QString DatabaseSchemaHelper::COMMA(",");
QString DatabaseSchemaHelper::OPENPAREN("(");
QString DatabaseSchemaHelper::CLOSEPAREN(")");
QString DatabaseSchemaHelper::END(";");

QString DatabaseSchemaHelper::UNIQUE("UNIQUE");

// Types
QString DatabaseSchemaHelper::TYPEINTEGER("INTEGER");
QString DatabaseSchemaHelper::TYPETEXT("TEXT");
QString DatabaseSchemaHelper::TYPEREAL("REAL");
QString DatabaseSchemaHelper::TYPENUMERIC("NUMERIC");
QString DatabaseSchemaHelper::TYPEDATETIME("DATETIME");
QString DatabaseSchemaHelper::TYPEBOOLEAN("BOOLEAN");

//Specials -- these all need to be initialized late.
QString DatabaseSchemaHelper::THENOW;
QString DatabaseSchemaHelper::FALSE;
QString DatabaseSchemaHelper::TRUE;

QString DatabaseSchemaHelper::id;
QString DatabaseSchemaHelper::deleted;
QString DatabaseSchemaHelper::display;

QString DatabaseSchemaHelper::name("name " + TYPETEXT + " not null DEFAULT ''");
QString DatabaseSchemaHelper::displayUnit("display_unit" + SEP + TYPEINTEGER + SEP + DEFAULT + " -1");
QString DatabaseSchemaHelper::displayScale("display_scale" + SEP + TYPEINTEGER + SEP + DEFAULT + " -1");
QString DatabaseSchemaHelper::displayTempUnit("display_temp_unit" + SEP + TYPEINTEGER + SEP + DEFAULT + " -1");
QString DatabaseSchemaHelper::folder("folder " + TYPETEXT + " DEFAULT ''");

QString DatabaseSchemaHelper::tableMeta("bt_alltables");
QString DatabaseSchemaHelper::colMetaClassName("class_name");
QString DatabaseSchemaHelper::colMetaInvId("inventory_table_id");
QString DatabaseSchemaHelper::colMetaChildId("child_table_id");
QString DatabaseSchemaHelper::colMetaDateCreated("created");
QString DatabaseSchemaHelper::colMetaVersion("version");
QString DatabaseSchemaHelper::colMetaTableId("table_id");

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
QString DatabaseSchemaHelper::colHopHtype("htype");
QString DatabaseSchemaHelper::colHopForm("form");
QString DatabaseSchemaHelper::colHopBeta("beta");
QString DatabaseSchemaHelper::colHopHsi("hsi");
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

// Mashes can be unnamed, so we need a different definition here.
QString DatabaseSchemaHelper::tableMash("mash");
QString DatabaseSchemaHelper::colMashName("name " + TYPETEXT + " DEFAULT ''");
QString DatabaseSchemaHelper::colMashGrainTemp("grain_temp");
QString DatabaseSchemaHelper::colMashNotes("notes");
QString DatabaseSchemaHelper::colMashTunTemp("tun_temp");
QString DatabaseSchemaHelper::colMashSpargeTemp("sparge_temp");
QString DatabaseSchemaHelper::colMashPh("ph");
QString DatabaseSchemaHelper::colMashTunWeight("tun_weight");
QString DatabaseSchemaHelper::colMashTunSpecificHeat("tun_specific_heat");
QString DatabaseSchemaHelper::colMashEquipAdjust("equip_adjust");

QString DatabaseSchemaHelper::tableMashStep("mashstep");
QString DatabaseSchemaHelper::colMashStepType("mstype varchar(32) DEFAULT 'Infusion'");
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

bool DatabaseSchemaHelper::upgrade = false;
// Default namespace hides functions from everything outside this file.

bool DatabaseSchemaHelper::create(QSqlDatabase db, Brewtarget::DBTypes dbType)
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
   // NOTE: This is order dependent. When using databases that actually
   //       enforce constraints (not sqlite), anything with a foreign key has
   //       to be created *after* what the foreign key references

   QSqlQuery q(db);
   bool ret = true;

   // Some stuff just needs evaluated late
   select_dbStrings(dbType);

   // This has to be done outside the transaction
   ret = create_meta(q);
   // Start transaction
   bool hasTransaction = db.transaction();

   // Create the core (beerXML) tables
   ret &= create_beerXMLTables(q);
   if ( ! ret ) {
      Brewtarget::logE("create_beerXMLTables() failed");
   }

   // Create the bt relational tables
   ret &= create_btTables(q);
   if ( ! ret ) {
      Brewtarget::logE("create_btTables() failed");
   }

   // Recipe relational tables
   ret &= create_inRecipeTables(q);
   if ( ! ret ) {
      Brewtarget::logE("create_inRecipeTables() failed");
   }

   // Triggers are still a pain
   ret &= create_increment_trigger(q,dbType);
   if ( ! ret ) {
      Brewtarget::logE("create_increment_trigger() failed");
   }
   ret &= create_decrement_trigger(q,dbType);
   if ( ! ret ) {
      Brewtarget::logE("create_decrement_trigger() failed");
   }

   // Ingredient inheritance tables============================================
   ret &= create_childrenTables(q);
   if ( ! ret ) {
      Brewtarget::logE("create_childrenTables() failed");
   }

   // Inventory tables=========================================================
   ret &= create_inventoryTables(q);
   if ( ! ret ) {
      Brewtarget::logE("create_inventoryTables() failed");
   }

   // Commit transaction
   if( hasTransaction )
      ret &= db.commit();

   if ( ! ret ) {
      Brewtarget::logE("db.commit() failed");
   }

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
         ret &= migrate_to_202(q);
         break;
      case 2: // == '2.0.2'
         ret &= migrate_to_210(q);
         break;
      case 3: // == '2.1.0'
         ret &= migrate_to_4(q);
         break;
      case 4:
         ret &= migrate_to_5(q);
         break;
      case 5:
         ret &= migrate_to_6(q);
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
         " SET " + colSettingsVersion + "=" + QString::number(oldVersion+1) + " WHERE id=1"
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

   // Late eval of some strings
   select_dbStrings(Brewtarget::dbType());
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

QString DatabaseSchemaHelper::foreignKey( QString const& column, QString const& foreignTable )
{
   return QString("FOREIGN KEY(%1) REFERENCES %2(id)").arg(column).arg(foreignTable);
}

void DatabaseSchemaHelper::select_dbStrings(Brewtarget::DBTypes dbType)
{
   Brewtarget::DBTypes thisDbType = dbType;

   if ( thisDbType == Brewtarget::NODB )
      thisDbType = Brewtarget::dbType();

   switch( thisDbType ) {
      case Brewtarget::PGSQL:
         id = "id SERIAL PRIMARY KEY";
         TYPEDATETIME = "TIMESTAMP";
         THENOW = "CURRENT_TIMESTAMP";
         break;
      default:
         id = "id INTEGER PRIMARY KEY autoincrement";
         TYPEDATETIME = "DATETIME";
         THENOW = "CURRENT_DATETIME";
   }
   TRUE  = Brewtarget::dbTrue(thisDbType);
   FALSE = Brewtarget::dbFalse(thisDbType);

   deleted = QString("deleted" + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE );
   display = QString("display" + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + TRUE);
}

bool DatabaseSchemaHelper::create_table(QSqlQuery q, QString create, QString tableName, 
                  Brewtarget::DBTable tableid, QString className, 
                  Brewtarget::DBTable inv_id, Brewtarget::DBTable child_id)
{
   try {
      if ( ! q.exec(create) )
         throw QString("Creating %1 table failed: %2 : %3").arg(tableName).arg(create).arg(q.lastError().text());

      if ( ! upgrade && ! insert_meta(q,tableName,tableid,className,inv_id,child_id))
         throw QString("Could not insert into meta");
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      q.finish();
      return false;
   }

   q.finish();
   return true;
}

bool DatabaseSchemaHelper::insert_meta(QSqlQuery q, QString const& name, 
                  Brewtarget::DBTable tableid, QString className, 
                  Brewtarget::DBTable inv_id, Brewtarget::DBTable child_id)
{
   QString insert = QString(
         INSERTINTO + SEP + tableMeta + SEP + 
         OPENPAREN +
            "name"             + COMMA +
            colMetaClassName   + COMMA +
            colMetaTableId     + COMMA +
            colMetaInvId       + COMMA +
            colMetaChildId     + 
         CLOSEPAREN + SEP +
         "VALUES('%1','%2',%3,%4,%5)")
      .arg(name)
      .arg(className)
      .arg(tableid)
      .arg(inv_id)
      .arg(child_id);

   try {
      if ( ! q.exec(insert) ) 
         throw QString("Inserting into meta table failed: %1 : %2").arg(insert).arg(q.lastError().text());
   }
   catch( QString e ) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      q.finish();
      return false;
   }

   q.finish();
   return true;
}

bool DatabaseSchemaHelper::create_meta(QSqlQuery q)
{
   QString create = QString(
      CREATETABLE + SEP + tableMeta + SEP + OPENPAREN +
         id                                                                     + COMMA +
         name                                                                   + COMMA +
         colMetaClassName   + SEP + TYPETEXT     + SEP + DEFAULT + SEP + "''"   + COMMA +
         colMetaInvId       + SEP + TYPEINTEGER  + SEP + DEFAULT + SEP + "0"   + COMMA +
         colMetaChildId     + SEP + TYPEINTEGER  + SEP + DEFAULT + SEP + "0"   + COMMA +
         colMetaDateCreated + SEP + TYPEDATETIME + SEP + DEFAULT + SEP + THENOW + COMMA +
         colMetaVersion     + SEP + TYPEINTEGER  + SEP + DEFAULT + SEP + "%1"   + COMMA +
         colMetaTableId     + SEP + TYPEINTEGER  + SEP + "not null" +
      CLOSEPAREN).arg(dbVersion);
  
   return create_table(q,create,tableMeta,Brewtarget::BTALLTABLE);
}

bool DatabaseSchemaHelper::create_beerXMLTables(QSqlQuery q)
{
   bool ret = true;

   ret &= create_settings(q);
   ret &= create_equipment(q);
   ret &= create_fermentable(q);
   ret &= create_hop(q);
   ret &= create_misc(q);
   ret &= create_style(q);
   ret &= create_yeast(q);
   ret &= create_water(q);
   ret &= create_mash(q);
   ret &= create_mashstep(q);
   ret &= create_recipe(q);
   ret &= create_brewnote(q);
   ret &= create_instruction(q);

   return ret;
}

bool DatabaseSchemaHelper::create_btTables(QSqlQuery q)
{
   // The following bt_* tables simply point to ingredients provided by brewtarget.
   // This is to make updating and pushing new ingredients easy.
   // NOTE: they MUST be named bt_<table>, where <table> is the table name that
   // they refer to, and they MUST contain fields 'id' and '<table>_id'.
   bool ret = true;

   ret &= create_btTable(q, tableBtEquipment, tableEquipment, Brewtarget::BT_EQUIPTABLE);
   ret &= create_btTable(q, tableBtFermentable, tableFermentable, Brewtarget::BT_FERMTABLE);
   ret &= create_btTable(q, tableBtHop, tableHop, Brewtarget::BT_HOPTABLE);
   ret &= create_btTable(q, tableBtMisc, tableMisc, Brewtarget::BT_MISCTABLE);
   ret &= create_btTable(q, tableBtStyle, tableStyle, Brewtarget::BT_STYLETABLE);
   ret &= create_btTable(q, tableBtYeast, tableYeast, Brewtarget::BT_YEASTTABLE);
   ret &= create_btTable(q, tableBtWater, tableWater, Brewtarget::BT_WATERTABLE);

   return ret;
}

bool DatabaseSchemaHelper::create_inRecipeTables(QSqlQuery q)
{
   bool ret = true;

   ret &= create_recipeChildTable(q, tableFermInRec, tableFermentable, Brewtarget::FERMINRECTABLE);
   ret &= create_recipeChildTable(q, tableHopInRec, tableHop, Brewtarget::HOPINRECTABLE);
   ret &= create_recipeChildTable(q, tableMiscInRec, tableMisc, Brewtarget::MISCINRECTABLE);
   ret &= create_recipeChildTable(q, tableWaterInRec, tableWater, Brewtarget::WATERINRECTABLE);
   ret &= create_recipeChildTable(q, tableYeastInRec, tableYeast, Brewtarget::YEASTINRECTABLE);
   // Instructions are the oddball -- they add an extra field nobody else adds
   ret &= create_recipeChildTable(q, tableInsInRec, tableInstruction, Brewtarget::INSTINRECTABLE);

   return ret;
}

bool DatabaseSchemaHelper::create_inventoryTables(QSqlQuery q)
{
   bool ret = true;

   ret &= create_inventoryTable(q,tableFermInventory,tableFermentable, Brewtarget::FERMINVTABLE);
   ret &= create_inventoryTable(q,tableHopInventory,tableHop, Brewtarget::HOPINVTABLE);
   ret &= create_inventoryTable(q,tableMiscInventory,tableMisc, Brewtarget::MISCINVTABLE);
   ret &= create_inventoryTable(q,tableYeastInventory,tableYeast, Brewtarget::YEASTINVTABLE);

   return ret ;
}

bool DatabaseSchemaHelper::create_childrenTables(QSqlQuery q)
{
   bool ret = true;
   ret &= create_childTable(q,tableEquipChildren,tableEquipment, Brewtarget::EQUIPCHILDTABLE);
   ret &= create_childTable(q,tableFermChildren,tableFermentable, Brewtarget::FERMCHILDTABLE);
   ret &= create_childTable(q,tableHopChildren ,tableHop, Brewtarget::HOPCHILDTABLE);
   ret &= create_childTable(q,tableMiscChildren ,tableMisc, Brewtarget::MISCCHILDTABLE);
   ret &= create_childTable(q,tableRecChildren ,tableRecipe, Brewtarget::RECIPECHILDTABLE);
   ret &= create_childTable(q,tableStyleChildren ,tableStyle, Brewtarget::STYLECHILDTABLE);
   ret &= create_childTable(q,tableWaterChildren ,tableWater, Brewtarget::WATERCHILDTABLE);
   ret &= create_childTable(q,tableYeastChildren ,tableYeast, Brewtarget::YEASTCHILDTABLE);

   return ret;
}

bool DatabaseSchemaHelper::create_childTable( QSqlQuery q, QString const& tableName, QString const& foreignTable, Brewtarget::DBTable tableid)
{
   QString create = 
            CREATETABLE + SEP + tableName + SEP + OPENPAREN +
            id                                             + COMMA +
            "parent_id" + SEP + TYPEINTEGER  + COMMA +
            "child_id"  + SEP + TYPEINTEGER  + SEP + UNIQUE              + COMMA +
            foreignKey("parent_id", foreignTable)          + COMMA +
            foreignKey("child_id", foreignTable) +
            CLOSEPAREN;

   return create_table(q,create,tableName,tableid);
}

// This ones a bit ugly, but the meta stuff is
bool DatabaseSchemaHelper::create_settings(QSqlQuery q)
{
   bool ret = true;
   QString create = CREATETABLE + SEP + tableSettings + OPENPAREN +
                    id                                                + COMMA +
                    colSettingsVersion            + SEP + TYPEINTEGER + COMMA +
                    colSettingsRepopulateChildren + SEP + TYPEINTEGER +
                    ")";
   ret =  create_table(q,create,tableSettings,Brewtarget::SETTINGTABLE);
   if ( ! ret ) {
      Brewtarget::logE(QString("Creating %1 table failed: %2 : %3").arg(tableSettings).arg(create).arg(q.lastError().text()));
   }
   else {
      QString insert = INSERTINTO + SEP + tableSettings + QString(" VALUES(1,%1,1)").arg(dbVersion);
      ret &= q.exec( insert );
      if ( ! ret ) {
         Brewtarget::logE(QString("Inserting into settings table failed: %1 : %2").arg(insert).arg(q.lastError().text()));
      }
   }

   return ret;
}

bool DatabaseSchemaHelper::create_equipment(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableEquipment + SEP + OPENPAREN +
      id                                                                          + COMMA +
      // BeerXML properties----------------------------------------------------
      name                                                                        + COMMA +
      colEquipBoilSize        + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipBatchSize       + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipTunVolume       + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipTunWeight       + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipTunSpecificHeat + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipTopUpWater      + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipTrubChillerLoss + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipEvapRate        + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipBoilTime        + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipCalcBoilVolume  + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE    + COMMA +
      colEquipLauterDeadspace + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipTopUpKettle     + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipHopUtilization  + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipNotes           + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"    + COMMA +
      // Our BeerXML extensions
      colEquipRealEvapRate    + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colEquipBoilingPoint    + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "100.0" + COMMA +
      colEquipAbsorption      + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "1.085" + COMMA +
      // Metadata
      deleted                                                                     + COMMA +
      display                                                                     + COMMA +
      folder +
      CLOSEPAREN;

   return create_table(q,create,tableEquipment,Brewtarget::EQUIPTABLE,"Equipment",Brewtarget::NOTABLE,Brewtarget::EQUIPCHILDTABLE);
}

bool DatabaseSchemaHelper::create_fermentable(QSqlQuery q)
{
   QString create =
      CREATETABLE + SEP + tableFermentable + SEP + OPENPAREN +
      id                                                                          + COMMA +
      // BeerXML properties----------------------------------------------------
      name                                                                        + COMMA +
      colFermFtype          + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "'Grain'" + COMMA +
      colFermAmount         + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colFermYield          + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colFermColor          + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colFermAddAfterBoil   + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE     + COMMA +
      colFermOrigin         + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"      + COMMA +
      colFermSupplier       + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"      + COMMA +
      colFermNotes          + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"      + COMMA +
      colFermCoarseFineDiff + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colFermMoisture       + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colFermDiastaticPower + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colFermProtein        + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colFermMaxInBatch     + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "100.0"   + COMMA +
      colFermRecommendMash  + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE     + COMMA +
      colFermIsMashed       + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE     + COMMA +
      colFermIbuGalLb       + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      // Display stuff---------------------------------------------------------
      displayUnit                                                                 + COMMA +
      displayScale                                                                + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                     + COMMA +
      display                                                                     + COMMA +
      folder +
      CLOSEPAREN;

   return create_table(q,create,tableFermentable,Brewtarget::FERMTABLE, "Fermentable",Brewtarget::FERMINVTABLE,Brewtarget::FERMCHILDTABLE );
}

bool DatabaseSchemaHelper::create_hop(QSqlQuery q)
{
   QString create = 
      CREATETABLE         + SEP + tableHop + OPENPAREN +
      id                                                                      + COMMA +
      // BeerXML properties----------------------------------------------------
      name                                                                    + COMMA +
      colHopAlpha         + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colHopAmount        + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colHopUse           + SEP + TYPETEXT + SEP + DEFAULT + SEP + "'Boil'"   + COMMA +
      colHopTime          + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colHopNotes         + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"       + COMMA +
      colHopHtype         + SEP + TYPETEXT + SEP + DEFAULT + SEP + "'Both'"   + COMMA +
      colHopForm          + SEP + TYPETEXT + SEP + DEFAULT + SEP + "'Pellet'" + COMMA +
      colHopBeta          + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colHopHsi           + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colHopOrigin        + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"       + COMMA +
      colHopSubstitutes   + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"       + COMMA +
      colHopHumulene      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colHopCaryophyllene + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colHopCohumulone    + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colHopMyrcene       + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      // Display stuff---------------------------------------------------------
      displayUnit                                                             + COMMA +
      displayScale                                                            + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                 + COMMA +
      display                                                                 + COMMA +
      folder +
      CLOSEPAREN;

   return create_table(q,create,tableHop,Brewtarget::HOPTABLE,"Hop",Brewtarget::HOPINVTABLE,Brewtarget::HOPCHILDTABLE );
}

bool DatabaseSchemaHelper::create_misc(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableMisc + SEP + OPENPAREN +
      id                                                                          + COMMA +
      // BeerXML properties----------------------------------------------------
      name + COMMA +
      colMiscMtype          + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "'Other'" + COMMA +
      colMiscUse            + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "'Boil'"  + COMMA +
      colMiscTime           + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colMiscAmount         + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"     + COMMA +
      colMiscAmountIsWeight + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + TRUE      + COMMA +
      colMiscUseFor         + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"      + COMMA +
      colMiscNotes          + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"      + COMMA +
      // Display stuff---------------------------------------------------------
      displayUnit                                                                 + COMMA +
      displayScale                                                                + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                     + COMMA +
      display                                                                     + COMMA +
      folder +
      CLOSEPAREN;

   return create_table(q,create,tableMisc,Brewtarget::MISCTABLE,"Misc",Brewtarget::MISCINVTABLE,Brewtarget::MISCCHILDTABLE );
}

bool DatabaseSchemaHelper::create_style(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableStyle + SEP + OPENPAREN +
      id                                                                   + COMMA +
      // BeerXML properties----------------------------------------------------
      name                                                                 + COMMA +
      colStyleType        + SEP + TYPETEXT + SEP + DEFAULT + SEP + "'Ale'" + COMMA +
      colStyleCat         + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"    + COMMA +
      colStyleCatNum      + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"    + COMMA +
      colStyleLetter      + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"    + COMMA +
      colStyleGuide       + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"    + COMMA +
      colStyleOgMin       + SEP + TYPEREAL + SEP + DEFAULT + SEP + "1.0"   + COMMA +
      colStyleOgMax       + SEP + TYPEREAL + SEP + DEFAULT + SEP + "1.1"   + COMMA +
      colStyleFgMin       + SEP + TYPEREAL + SEP + DEFAULT + SEP + "1.0"   + COMMA +
      colStyleFgMax       + SEP + TYPEREAL + SEP + DEFAULT + SEP + "1.1"   + COMMA +
      colStyleIbuMin      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colStyleIbuMax      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "100.0" + COMMA +
      colStyleColorMin    + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colStyleColorMax    + SEP + TYPEREAL + SEP + DEFAULT + SEP + "100.0" + COMMA +
      colStyleAbvMin      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colStyleAbvMax      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "100.0" + COMMA +
      colStyleCarbMin     + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0"   + COMMA +
      colStyleCarbMax     + SEP + TYPEREAL + SEP + DEFAULT + SEP + "100.0" + COMMA +
      colStyleNotes       + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"    + COMMA +
      colStyleProfile     + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"    + COMMA +
      colStyleIngredients + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"    + COMMA +
      colStyleExamples    + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"    + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                              + COMMA +
      display                                                              + COMMA +
      folder +
      CLOSEPAREN;
   return create_table(q,create,tableStyle,Brewtarget::STYLETABLE,"Style",Brewtarget::NOTABLE,Brewtarget::STYLECHILDTABLE);
}

bool DatabaseSchemaHelper::create_yeast(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableYeast + SEP + OPENPAREN +
      id                                                                            + COMMA +
      // BeerXML properties----------------------------------------------------
      name                                                                          + COMMA +
      colYeastType           + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "'Ale'"    + COMMA +
      colYeastForm           + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "'Liquid'" + COMMA +
      colYeastAmount         + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colYeastAmountIsWeight + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE      + COMMA +
      colYeastLab            + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"       + COMMA +
      colYeastProductId      + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"       + COMMA +
      colYeastTempMin        + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"      + COMMA +
      colYeastTempMax        + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "32.0"     + COMMA +
      colYeastFlocc          + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "'Medium'" + COMMA +
      colYeastAtten          + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "75.0"     + COMMA +
      colYeastNotes          + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"       + COMMA +
      colYeastBestFor        + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"       + COMMA +
      colYeastRecultures     + SEP + TYPEINTEGER + SEP + DEFAULT + SEP + "0"        + COMMA +
      colYeastReuseMax       + SEP + TYPEINTEGER + SEP + DEFAULT + SEP + "10"       + COMMA +
      colYeastSecondary      + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE      + COMMA +
      // Display stuff---------------------------------------------------------
      displayUnit                                                                   + COMMA +
      displayScale                                                                  + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                       + COMMA +
      display                                                                       + COMMA +
      folder +
      CLOSEPAREN;

   return create_table(q,create,tableYeast,Brewtarget::YEASTTABLE,"Yeast",Brewtarget::YEASTINVTABLE,Brewtarget::YEASTCHILDTABLE );
}

bool DatabaseSchemaHelper::create_water(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableWater + SEP + "(" +
      id                                                             + COMMA +
      // BeerXML properties----------------------------------------------------
      name                                                           + COMMA +
      colWaterAmount  + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0" + COMMA +
      colWaterCa      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0" + COMMA +
      colWaterBicarb  + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0" + COMMA +
      colWaterSulfate + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0" + COMMA +
      colWaterCl      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0" + COMMA +
      colWaterNa      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0" + COMMA +
      colWaterMg      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0" + COMMA +
      colWaterPh      + SEP + TYPEREAL + SEP + DEFAULT + SEP + "7.0" + COMMA +
      colWaterNotes   + SEP + TYPETEXT + SEP + DEFAULT + SEP + "''"  + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                        + COMMA +
      display                                                        + COMMA +
      folder +
      CLOSEPAREN;

   return create_table(q,create,tableWater,Brewtarget::WATERTABLE,"Water",Brewtarget::NOTABLE,Brewtarget::WATERCHILDTABLE);
}

bool DatabaseSchemaHelper::create_mash(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableMash + SEP + OPENPAREN +
      id                                                                        + COMMA +
      // BeerXML properties----------------------------------------------------
      colMashName + COMMA +
      colMashGrainTemp       + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colMashNotes           + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"   + COMMA +
      colMashTunTemp         + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "20.0" + COMMA +
      colMashSpargeTemp      + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "74.0" + COMMA +
      colMashPh              + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "7.0"  + COMMA +
      colMashTunWeight       + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colMashTunSpecificHeat + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colMashEquipAdjust     + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + TRUE   + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                   + COMMA +
      display                                                                   + COMMA +
      folder +
      CLOSEPAREN;

   return create_table(q, create, tableMash,Brewtarget::MASHTABLE, "Mash");
}

bool DatabaseSchemaHelper::create_mashstep(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableMashStep + SEP + OPENPAREN +
      id                                                                       + COMMA +
      // BeerXML properties----------------------------------------------------
      name                                                                     + COMMA +
      colMashStepType      + SEP                                               + COMMA +
      colMashStepInfAmount + SEP + TYPEREAL    + SEP  + DEFAULT + SEP + "0.0"  + COMMA +
      colMashStepTemp      + SEP + TYPEREAL    + SEP  + DEFAULT + SEP + "67.0" + COMMA +
      colMashStepTime      + SEP + TYPEREAL    + SEP  + DEFAULT + SEP + "0.0"  + COMMA +
      colMashStepRampTime  + SEP + TYPEREAL    + SEP  + DEFAULT + SEP + "0.0"  + COMMA +
      colMashStepEndTemp   + SEP + TYPEREAL    + SEP  + DEFAULT + SEP + "67.0" + COMMA +
      colMashStepInfTemp   + SEP + TYPEREAL    + SEP  + DEFAULT + SEP + "67.0" + COMMA +
      colMashStepDecAmount + SEP + TYPEREAL    + SEP  + DEFAULT + SEP + "0.0"  + COMMA +
      // Relational data-------------------------------------------------------
      colMashStepMashId    + SEP + TYPEINTEGER                                 + COMMA +
      colMashStepNumber    + SEP + TYPEINTEGER + SEP  + DEFAULT + SEP + "0"    + COMMA +
      // Display stuff---------------------------------------------------------
      displayUnit                                                              + COMMA +
      displayScale                                                             + COMMA +
      displayTempUnit                                                          + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                  + COMMA +
      display                                                                  + COMMA +
      folder                                                                   + COMMA +
      foreignKey(colMashStepMashId, tableMash) +
      CLOSEPAREN;

   return
      create_table(q,create,tableMashStep,Brewtarget::MASHSTEPTABLE,"MashStep");
}

bool DatabaseSchemaHelper::create_brewnote(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableBrewnote + SEP + OPENPAREN +
      id                                                                          + COMMA +
      colBNoteBrewDate        + SEP + TYPEDATETIME + SEP + DEFAULT + SEP + THENOW + COMMA +
      colBNoteFermentDate     + SEP + TYPEDATETIME + SEP + DEFAULT + SEP + THENOW + COMMA +
      colBNoteSg              + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"  + COMMA +
      colBNoteBkVolume        + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteStrikeTemp      + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "70.0" + COMMA +
      colBNoteFinalMashTemp   + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "67.0" + COMMA +
      colBNoteOg              + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"  + COMMA +
      colBNotePostboilVolume  + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteFermenterVolume + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNotePitchTemp       + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "20.0" + COMMA +
      colBNoteFg              + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"  + COMMA +
      colBNoteBkEff           + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "70.0" + COMMA +
      colBNoteAbv             + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNotePredOg          + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"  + COMMA +
      colBNoteEff             + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "70.0" + COMMA +
      colBNotePredAbv         + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteProjBoilGrav    + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"  + COMMA +
      colBNoteProjStrikeTemp  + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "70.0" + COMMA +
      colBNoteProjFinTemp     + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "67.0" + COMMA +
      colBNoteProjFinMashTemp + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "67.0" + COMMA +
      colBNoteProjBkVol       + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteProjOg          + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"  + COMMA +
      colBNoteProjFermVol     + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteProjFg          + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"  + COMMA +
      colBNoteProjEff         + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "70.0" + COMMA +
      colBNoteProjAbv         + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteProjAtten       + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "75.0" + COMMA +
      colBNoteProjPoints      + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteProjFermPoints  + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteBoilOff         + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteFinalVolume     + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"  + COMMA +
      colBNoteNotes           + SEP + TYPETEXT     + SEP + DEFAULT + SEP + "''"   + COMMA +
      // Relational data-------------------------------------------------------
      colBNoteRecipeId        + SEP + TYPEINTEGER                                 + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                     + COMMA +
      display                                                                     + COMMA +
      folder                                                                      + COMMA +
      foreignKey(colBNoteRecipeId, tableRecipe) +
      CLOSEPAREN;

   return
      create_table(q,create,tableBrewnote,Brewtarget::BREWNOTETABLE,"BrewNote");
}

bool DatabaseSchemaHelper::create_instruction(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableInstruction + SEP + OPENPAREN +
      id                                                                        + COMMA +
      name                                                                      + COMMA +
      colInsDirections + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "''"         + COMMA +
      colInsHasTimer   + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE        + COMMA +
      colInsTimerVal   + SEP + TYPETEXT    + SEP + DEFAULT + SEP + "'00:00:00'" + COMMA +
      colInsCompleted  + SEP + TYPEBOOLEAN + SEP + DEFAULT + SEP + FALSE        + COMMA +
      colInsInterval   + SEP + TYPEREAL    + SEP + DEFAULT + SEP + "0.0"        + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                   + COMMA +
      display +
      // instructions aren't displayed in trees, and get no folder
      CLOSEPAREN;

   return create_table(q,create,tableInstruction,Brewtarget::INSTRUCTIONTABLE,"Instruction");
}

bool DatabaseSchemaHelper::create_recipe(QSqlQuery q)
{
   QString create = 
      CREATETABLE + SEP + tableRecipe + SEP + OPENPAREN +
      id                                                                                                 + COMMA +
      // BeerXML properties----------------------------------------------------
      name                                                                                               + COMMA +
      colRecType         + SEP + TYPETEXT     + SEP + DEFAULT + SEP + "'All Grain'"                      + COMMA +
      colRecBrewer       + SEP + TYPETEXT     + SEP + DEFAULT + SEP + "''"                               + COMMA +
      colRecAsstBrewer   + SEP + TYPETEXT     + SEP + DEFAULT + SEP + "'Brewtarget: free beer software'" + COMMA +
      colRecBatchSize    + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      colRecBoilSize     + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      colRecBoilTime     + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      colRecEff          + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "70.0"                             + COMMA +
      colRecOg           + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"                              + COMMA +
      colRecFg           + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"                              + COMMA +
      colRecFermStages   + SEP + TYPEINTEGER  + SEP + DEFAULT + SEP + "1"                                + COMMA +
      colRecPrimAge      + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      colRecPrimTemp     + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "20.0"                             + COMMA +
      colRecSecAge       + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      colRecSecTemp      + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "20.0"                             + COMMA +
      colRecTerAge       + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      colRecTerTemp      + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "20.0"                             + COMMA +
      colRecAge          + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      colRecAgeTemp      + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "20.0"                             + COMMA +
      colRecDate         + SEP + TYPEDATETIME + SEP + DEFAULT + SEP + THENOW                             + COMMA +
      colRecCarbVol      + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      colRecForceCarb    + SEP + TYPEBOOLEAN  + SEP + DEFAULT + SEP + FALSE                              + COMMA +
      colRecPrimSug      + SEP + TYPETEXT     + SEP + DEFAULT + SEP + "''"                               + COMMA +
      colRecCarbTemp     + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "20.0"                             + COMMA +
      colRecPrimSugEquiv + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"                              + COMMA +
      colRecKegPrimFact  + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "1.0"                              + COMMA +
      colRecNotes        + SEP + TYPETEXT     + SEP + DEFAULT + SEP + "''"                               + COMMA +
      colRecTasteNotes   + SEP + TYPETEXT     + SEP + DEFAULT + SEP + "''"                               + COMMA +
      colRecTasteRating  + SEP + TYPEREAL     + SEP + DEFAULT + SEP + "0.0"                              + COMMA +
      // Relational data-------------------------------------------------------
      colRecStyleId      + SEP + TYPEINTEGER                                                             + COMMA +
      colRecMashId       + SEP + TYPEINTEGER                                                             + COMMA +
      colRecEquipId      + SEP + TYPEINTEGER                                                             + COMMA +
      // Metadata--------------------------------------------------------------
      deleted                                                                                            + COMMA +
      display                                                                                            + COMMA +
      folder                                                                                             + COMMA +
      foreignKey(colRecStyleId, tableStyle)                                                              + COMMA +
      foreignKey(colRecMashId, tableMash)                                                                + COMMA +
      foreignKey(colRecEquipId, tableEquipment) + 
      CLOSEPAREN;

   return create_table(q,create,tableRecipe,Brewtarget::RECTABLE,"Recipe",Brewtarget::NOTABLE,Brewtarget::RECIPECHILDTABLE);
}

bool DatabaseSchemaHelper::create_btTable(QSqlQuery q, QString tableName, QString foreignTableName, Brewtarget::DBTable tableid)
{
   QString foreignIdName = QString("%1_id").arg(foreignTableName);
   QString create = 
      CREATETABLE + SEP + tableName + SEP + OPENPAREN +
      id                                           + COMMA +
      foreignIdName + SEP + TYPEINTEGER            + COMMA +
      foreignKey(foreignIdName, foreignTableName) +
      CLOSEPAREN;
   return create_table(q,create,tableName,tableid);
}

bool DatabaseSchemaHelper::create_recipeChildTable( QSqlQuery q, QString tableName, QString foreignTableName, Brewtarget::DBTable tableid)
{
   QString index = QString("%1_id").arg(foreignTableName);
   QString create = 
           CREATETABLE + SEP + tableName + SEP + OPENPAREN +
           id                                               + COMMA +
           index + SEP + TYPEINTEGER                        + COMMA +
           "recipe_id" + SEP + TYPEINTEGER                  + COMMA;
   // silly special cases
   if ( tableName == tableInsInRec ) 
      create += "instruction_number " + TYPEINTEGER + SEP + DEFAULT + SEP + "0" + COMMA;

   create += foreignKey(index, foreignTableName) + COMMA +
             foreignKey("recipe_id", tableRecipe) +
             CLOSEPAREN;

   return create_table(q,create,tableName,tableid);
}

bool DatabaseSchemaHelper::create_inventoryTable(QSqlQuery q, QString tableName, QString foreignTableName, Brewtarget::DBTable tableid)
{
   QString foreignIdName = QString("%1_id").arg(foreignTableName);
   QString field;
   // For yeast, homebrewers don't usually keep stores of yeast. They keep
   // packets or vials or some other type of discrete integer quantity. So, I
   // don't know how useful a real-valued inventory amount would be for yeast.
   if ( tableName == tableYeastInventory ) {
      field = "quanta" + SEP + TYPEINTEGER + SEP + DEFAULT +SEP + "0";
   }
   else {
      field = "amount" + SEP + TYPEREAL + SEP + DEFAULT + SEP + "0.0";
   }

   QString cName = tableName == tableYeastInventory ? "quanta" : "amount";

   QString create = 
      CREATETABLE   + SEP + tableName + SEP + OPENPAREN +
      id                                                + COMMA +
      field                                             + COMMA +
      foreignIdName + SEP + TYPEINTEGER + SEP + UNIQUE  + COMMA +
      foreignKey(foreignIdName, foreignTableName) +
      CLOSEPAREN;

   return create_table(q,create,tableName,tableid);
}

bool DatabaseSchemaHelper::create_increment_trigger(QSqlQuery q, Brewtarget::DBTypes dbType)
{
   // Mmm. This looks evil, doesn't it?
   Brewtarget::DBTypes thisType = dbType == Brewtarget::NODB ?  Brewtarget::dbType() : dbType;
   bool ret;

   switch( thisType ) {
      case Brewtarget::PGSQL:
         ret = create_pgsql_increment_trigger(q);
         break;
      default:
         ret = create_sqlite_increment_trigger(q);
   }

   return ret;
}

// This trigger automatically makes a new instruction in a recipe the last.
bool DatabaseSchemaHelper::create_sqlite_increment_trigger(QSqlQuery q)
{
   bool ret = true;
   QString create =
      QString() +
      "CREATE TRIGGER inc_ins_num AFTER INSERT ON instruction_in_recipe " +
      "BEGIN " +
         "UPDATE instruction_in_recipe SET instruction_number = " +
           "(SELECT max(instruction_number) FROM instruction_in_recipe WHERE recipe_id = new.recipe_id) + 1 " +
           "WHERE rowid = new.rowid; " +
      "END";

   ret =  q.exec(create);
   if ( ! ret ) {
      Brewtarget::logE(QString("Creating trigger failed: %1 : %2").arg(create).arg(q.lastError().text()));
   }
   return ret;
}

bool DatabaseSchemaHelper::create_pgsql_increment_trigger(QSqlQuery q)
{
   // Triggers in PGSQL are harder. We have to define a function, then assign
   // that function to the trigger
   bool ret = true;
   QString function = QString() +
         "CREATE OR REPLACE FUNCTION increment_instruction_num() RETURNS TRIGGER AS $BODY$ " +
         "BEGIN " +
            "UPDATE instruction_in_recipe SET instruction_number = " +
            "(SELECT max(instruction_number) FROM instruction_in_recipe WHERE recipe_id = NEW.recipe_id) + 1 " +
            "WHERE id = NEW.id;" +
            "return NULL;" +
         "END;" +
         "$BODY$"+
         " LANGUAGE plpgsql;";
   QString trigger = QString() +
         "CREATE TRIGGER inc_ins_num AFTER INSERT ON instruction_in_recipe " +
         "FOR EACH ROW EXECUTE PROCEDURE increment_instruction_num();";


   // This makes the function
   ret = q.exec( function );
   if ( ! ret ) {
      Brewtarget::logE(QString("Creating function failed: %1 : %2").arg(function).arg(q.lastError().text()));
   }

   ret &= q.exec(trigger);
   if ( ! ret ) {
      Brewtarget::logE(QString("Creating trigger failed: %1 : %2").arg(trigger).arg(q.lastError().text()));
   }
   return ret;
}

bool DatabaseSchemaHelper::create_decrement_trigger(QSqlQuery q, Brewtarget::DBTypes dbType)
{
   // Mmm. This looks evil, doesn't it?
   Brewtarget::DBTypes thisType = dbType == Brewtarget::NODB ?  Brewtarget::dbType() : dbType;
   bool ret;
   switch( thisType ) {
      case Brewtarget::PGSQL:
         ret = create_pgsql_decrement_trigger(q);
         break;
      default:
         ret = create_sqlite_decrement_trigger(q);
   }

   return ret;
}

bool DatabaseSchemaHelper::create_sqlite_decrement_trigger(QSqlQuery q)
{
   bool ret = true;
   QString create = QString() +
      "CREATE TRIGGER dec_ins_num AFTER DELETE ON instruction_in_recipe " +
      "BEGIN "
         "UPDATE instruction_in_recipe SET instruction_number = instruction_number - 1 " +
            "WHERE recipe_id = old.recipe_id AND instruction_number > old.instruction_number; " +
      "END";

   // This trigger automatically decrements all instruction numbers greater than the one
   // deleted in the given recipe.
   ret =  q.exec(create);
   if ( ! ret ) {
      Brewtarget::logE(QString("Creating trigger failed: %1 : %2").arg(create).arg(q.lastError().text()));
   }
   return ret;
}

bool DatabaseSchemaHelper::create_pgsql_decrement_trigger(QSqlQuery q)
{
   // Triggers in PGSQL are harder. We have to define a function, then assign
   // that function to the trigger
   bool ret = true;
   QString function = QString() +
         "CREATE OR REPLACE FUNCTION decrement_instruction_num() RETURNS TRIGGER AS $BODY$ " +
         "BEGIN " +
            "UPDATE instruction_in_recipe SET instruction_number = instruction_number - 1 " +
            "WHERE recipe_id = OLD.recipe_id AND instruction_id > OLD.instruction_id;" +
            "return NULL;" +
         "END;" +
         "$BODY$"+
         " LANGUAGE plpgsql;";
   QString trigger =QString() + 
         "CREATE TRIGGER dec_ins_num AFTER DELETE ON instruction_in_recipe " +
         "FOR EACH ROW EXECUTE PROCEDURE decrement_instruction_num();";


   // This makes the function
   ret = q.exec( function );
   if ( ! ret ) {
      Brewtarget::logE(QString("Creating function failed: %1 : %2").arg(function).arg(q.lastError().text()));
   }

   ret &= q.exec(trigger);
   if ( ! ret ) {
      Brewtarget::logE(QString("Creating trigger failed: %1 : %2").arg(trigger).arg(q.lastError().text()));
   }
   return ret;
}

bool DatabaseSchemaHelper::migrate_to_202(QSqlQuery q)
{
   bool ret = true;

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

   return ret;
}

bool DatabaseSchemaHelper::migrate_to_210(QSqlQuery q)
{
   bool ret = true;
   QStringList getFolders = QStringList() << tableEquipment << tableFermentable << tableHop <<
      tableMisc << tableStyle << tableYeast << tableWater << tableMash <<
      tableBrewnote << tableRecipe;

   QStringList rebuildTables = QStringList() << tableEquipChildren << tableFermChildren <<
      tableHopChildren << tableMiscChildren << tableRecChildren <<
      tableStyleChildren << tableWaterChildren << tableYeastChildren <<
      tableFermInventory << tableHopInventory << tableMiscInventory << tableYeastInventory;

   foreach(const QString &table, getFolders)
   {
      ret &= q.exec(
               ALTERTABLE + SEP + table + SEP +
               ADDCOLUMN + SEP + "folder" + SEP + TYPETEXT + SEP + DEFAULT + " ''"
            );
   }

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

   // Drop and re-create children and inventory tables with new UNIQUE requirement
   foreach(const QString &table, rebuildTables )
   {
      ret &= q.exec(
         DROPTABLE + SEP + table
      );
   }
   // This happens before the bt_alltables exists. Setting upgrade to true
   // prevents create_table from trying to insert into it.
   upgrade = true;
   ret &= create_childrenTables(q);
   ret &= create_inventoryTables(q);
   // Just to avoid any weird side effects
   upgrade = false;

   return ret;
}

bool DatabaseSchemaHelper::migrate_to_4(QSqlQuery q)
{
   bool ret = true;

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

   return ret;
}

bool DatabaseSchemaHelper::migrate_to_5(QSqlQuery q)
{
   bool ret = true;
   // Drop the previous bugged TRIGGER
   ret &= q.exec( QString() +
      "DROP TRIGGER dec_ins_num"
   );

   // Create the good trigger
   ret &= create_decrement_trigger(q);

   return ret;
}

bool DatabaseSchemaHelper::migrate_to_6(QSqlQuery q) {
   bool ret = true;

   ret = create_meta(q);

   ret &= insert_meta(q,tableSettings,    Brewtarget::SETTINGTABLE);
   ret &= insert_meta(q,tableEquipment,   Brewtarget::EQUIPTABLE,       "Equipment",   Brewtarget::NOTABLE,       Brewtarget::EQUIPCHILDTABLE);
   ret &= insert_meta(q,tableFermentable, Brewtarget::FERMTABLE,        "Fermentable", Brewtarget::FERMINVTABLE,  Brewtarget::FERMCHILDTABLE);
   ret &= insert_meta(q,tableHop,         Brewtarget::HOPTABLE,         "Hop",         Brewtarget::HOPINVTABLE,   Brewtarget::HOPCHILDTABLE);
   ret &= insert_meta(q,tableMisc,        Brewtarget::MISCTABLE,        "Misc",        Brewtarget::MISCINVTABLE,  Brewtarget::MISCCHILDTABLE);
   ret &= insert_meta(q,tableStyle,       Brewtarget::STYLETABLE,       "Style",       Brewtarget::NOTABLE,       Brewtarget::STYLECHILDTABLE);
   ret &= insert_meta(q,tableYeast,       Brewtarget::YEASTTABLE,       "Yeast",       Brewtarget::YEASTINVTABLE, Brewtarget::YEASTCHILDTABLE);
   ret &= insert_meta(q,tableWater,       Brewtarget::WATERTABLE,       "Water",       Brewtarget::NOTABLE,       Brewtarget::WATERCHILDTABLE);
   ret &= insert_meta(q,tableRecipe,      Brewtarget::RECTABLE,         "Recipe",      Brewtarget::NOTABLE,       Brewtarget::RECIPECHILDTABLE);
   ret &= insert_meta(q,tableMash,        Brewtarget::MASHTABLE,        "Mash");
   ret &= insert_meta(q,tableMashStep,    Brewtarget::MASHSTEPTABLE,    "MashStep");
   ret &= insert_meta(q,tableBrewnote,    Brewtarget::BREWNOTETABLE,    "BrewNote");
   ret &= insert_meta(q,tableInstruction, Brewtarget::INSTRUCTIONTABLE, "Instruction");

   ret &= insert_meta(q,tableBtEquipment,   Brewtarget::BT_EQUIPTABLE);
   ret &= insert_meta(q,tableBtFermentable, Brewtarget::BT_FERMTABLE);
   ret &= insert_meta(q,tableBtHop,         Brewtarget::BT_HOPTABLE);
   ret &= insert_meta(q,tableBtMisc,        Brewtarget::BT_MISCTABLE);
   ret &= insert_meta(q,tableBtStyle,       Brewtarget::BT_STYLETABLE);
   ret &= insert_meta(q,tableBtYeast,       Brewtarget::BT_YEASTTABLE);
   ret &= insert_meta(q,tableBtWater,       Brewtarget::BT_WATERTABLE);

   ret &= insert_meta(q,tableFermInRec,     Brewtarget::FERMINRECTABLE);
   ret &= insert_meta(q,tableHopInRec,      Brewtarget::HOPINRECTABLE);
   ret &= insert_meta(q,tableMiscInRec,     Brewtarget::MISCINRECTABLE);
   ret &= insert_meta(q,tableWaterInRec,    Brewtarget::WATERINRECTABLE);
   ret &= insert_meta(q,tableYeastInRec,    Brewtarget::YEASTINRECTABLE);
   ret &= insert_meta(q,tableInsInRec,      Brewtarget::INSTINRECTABLE);

   ret &= insert_meta(q,tableEquipChildren, Brewtarget::EQUIPCHILDTABLE);
   ret &= insert_meta(q,tableFermChildren,  Brewtarget::FERMCHILDTABLE);
   ret &= insert_meta(q,tableHopChildren,   Brewtarget::HOPCHILDTABLE);
   ret &= insert_meta(q,tableMiscChildren,  Brewtarget::MISCCHILDTABLE);
   ret &= insert_meta(q,tableRecChildren,   Brewtarget::RECIPECHILDTABLE);
   ret &= insert_meta(q,tableStyleChildren, Brewtarget::STYLECHILDTABLE);
   ret &= insert_meta(q,tableWaterChildren, Brewtarget::WATERCHILDTABLE);
   ret &= insert_meta(q,tableYeastChildren, Brewtarget::YEASTCHILDTABLE);

   ret &= insert_meta(q,tableFermInventory, Brewtarget::FERMINVTABLE);
   ret &= insert_meta(q,tableHopInventory,  Brewtarget::HOPINVTABLE);
   ret &= insert_meta(q,tableMiscInventory, Brewtarget::MISCINVTABLE);
   ret &= insert_meta(q,tableYeastInventory,Brewtarget::YEASTINVTABLE);

   return ret;
}
