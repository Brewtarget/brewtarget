/*
 * DatabaseSchemaHelper.h is part of Brewtarget, and is Copyright the following
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

#include "brewtarget.h"
#include <QString>
#include <QSqlDatabase>

/*!
 * \brief Helper to Database that manages schema stuff
 * \author Philip G. Lee
 *
 * This helper has static methods available only to Database that help it
 * manage the schema.
 */
class DatabaseSchemaHelper
{
   friend class Database;

public:
   
   // No public methods. Database is the only class able to access
   // DatabaseSchemaHelper methods.
   
private:

   //! \brief Database version. Increment on any schema change.
   static const int dbVersion;

   // Commands and keywords
   static QString CREATETABLE;
   static QString ALTERTABLE;
   static QString DROPTABLE;
   static QString ADDCOLUMN;
   static QString UPDATE;
   static QString SET;
   static QString INSERTINTO;
   static QString DEFAULT;
   static QString SELECT;
   static QString SEP;
   static QString UNIQUE;
   static QString COMMA;
   static QString OPENPAREN;
   static QString CLOSEPAREN;
   static QString END;
   
   // Types
   static QString TYPEINTEGER;
   static QString TYPETEXT;
   static QString TYPEREAL;
   static QString TYPENUMERIC;
   static QString TYPEDATETIME;
   static QString TYPEBOOLEAN;

   // Special values
   static QString THENOW;
   static QString FALSE;
   static QString TRUE;
   
   // ID string for EVERY table.
   static QString id;
   static QString name;
   static QString displayUnit;
   static QString displayScale;
   static QString displayTempUnit;
   static QString deleted;
   static QString display;
   static QString folder;
   
   // =============================Table Names/Columns=========================
   // the meta table
   static QString tableMeta;
   static QString colMetaClassName;
   static QString colMetaInvId;
   static QString colMetaChildId;
   static QString colMetaDateCreated;
   static QString colMetaVersion;
   static QString colMetaTableId;

   // Settings table
   static QString tableSettings;
   static QString colSettingsVersion;
   static QString colSettingsRepopulateChildren;
   
   // Equipment table
   static QString tableEquipment;
   static QString colEquipBoilSize;
   static QString colEquipBatchSize;
   static QString colEquipTunVolume;
   static QString colEquipTunWeight;
   static QString colEquipTunSpecificHeat;
   static QString colEquipTopUpWater;
   static QString colEquipTrubChillerLoss;
   static QString colEquipEvapRate;
   static QString colEquipBoilTime;
   static QString colEquipCalcBoilVolume;
   static QString colEquipLauterDeadspace;
   static QString colEquipTopUpKettle;
   static QString colEquipHopUtilization;
   static QString colEquipNotes;
   static QString colEquipRealEvapRate;
   static QString colEquipBoilingPoint;
   static QString colEquipAbsorption;
   
   // Fermentable table
   static QString tableFermentable;
   static QString colFermFtype;
   static QString colFermAmount;
   static QString colFermYield;
   static QString colFermColor;
   static QString colFermAddAfterBoil;
   static QString colFermOrigin;
   static QString colFermSupplier;
   static QString colFermNotes;
   static QString colFermCoarseFineDiff;
   static QString colFermMoisture;
   static QString colFermDiastaticPower;
   static QString colFermProtein;
   static QString colFermMaxInBatch;
   static QString colFermRecommendMash;
   static QString colFermIsMashed;
   static QString colFermIbuGalLb;
   
   // Hop table
   static QString tableHop;
   static QString colHopAlpha;
   static QString colHopAmount;
   static QString colHopUse;
   static QString colHopTime;
   static QString colHopNotes;
   static QString colHopHtype;
   static QString colHopForm;
   static QString colHopBeta;
   static QString colHopHsi;
   static QString colHopOrigin;
   static QString colHopSubstitutes;
   static QString colHopHumulene;
   static QString colHopCaryophyllene;
   static QString colHopCohumulone;
   static QString colHopMyrcene;
   
   // Misc table
   static QString tableMisc;
   static QString colMiscMtype;
   static QString colMiscUse;
   static QString colMiscTime;
   static QString colMiscAmount;
   static QString colMiscAmountIsWeight;
   static QString colMiscUseFor;
   static QString colMiscNotes;
   
   // Style table
   static QString tableStyle;
   static QString colStyleType;
   static QString colStyleCat;
   static QString colStyleCatNum;
   static QString colStyleLetter;
   static QString colStyleGuide;
   static QString colStyleOgMin;
   static QString colStyleOgMax;
   static QString colStyleFgMin;
   static QString colStyleFgMax;
   static QString colStyleIbuMin;
   static QString colStyleIbuMax;
   static QString colStyleColorMin;
   static QString colStyleColorMax;
   static QString colStyleAbvMin;
   static QString colStyleAbvMax;
   static QString colStyleCarbMin;
   static QString colStyleCarbMax;
   static QString colStyleNotes;
   static QString colStyleProfile;
   static QString colStyleIngredients;
   static QString colStyleExamples;
   
   // Yeast table
   static QString tableYeast;
   static QString colYeastType;
   static QString colYeastForm;
   static QString colYeastAmount;
   static QString colYeastAmountIsWeight;
   static QString colYeastLab;
   static QString colYeastProductId;
   static QString colYeastTempMin;
   static QString colYeastTempMax;
   static QString colYeastFlocc;
   static QString colYeastAtten;
   static QString colYeastNotes;
   static QString colYeastBestFor;
   static QString colYeastRecultures;
   static QString colYeastReuseMax;
   static QString colYeastSecondary;
   
   // Water table
   static QString tableWater;
   static QString colWaterAmount;
   static QString colWaterCa;
   static QString colWaterBicarb;
   static QString colWaterSulfate;
   static QString colWaterCl;
   static QString colWaterNa;
   static QString colWaterMg;
   static QString colWaterPh;
   static QString colWaterNotes;
   
   static QString tableMash;
   static QString colMashName;
   static QString colMashGrainTemp;
   static QString colMashNotes;
   static QString colMashTunTemp;
   static QString colMashSpargeTemp;
   static QString colMashPh;
   static QString colMashTunWeight;
   static QString colMashTunSpecificHeat;
   static QString colMashEquipAdjust;
   
   static QString tableMashStep;
   static QString colMashStepType;
   static QString colMashStepInfAmount;
   static QString colMashStepTemp;
   static QString colMashStepTime;
   static QString colMashStepRampTime;
   static QString colMashStepEndTemp;
   static QString colMashStepInfTemp;
   static QString colMashStepDecAmount;
   static QString colMashStepMashId;
   static QString colMashStepNumber;
   
   // Brewnote table
   static QString tableBrewnote;
   static QString colBNoteBrewDate;
   static QString colBNoteFermentDate;
   static QString colBNoteSg;
   static QString colBNoteBkVolume;
   static QString colBNoteStrikeTemp;
   static QString colBNoteFinalMashTemp;
   static QString colBNoteOg;
   static QString colBNotePostboilVolume;
   static QString colBNoteFermenterVolume;
   static QString colBNotePitchTemp;
   static QString colBNoteFg;
   static QString colBNoteBkEff;
   static QString colBNoteAbv;
   static QString colBNotePredOg;
   static QString colBNoteEff;
   static QString colBNotePredAbv;
   static QString colBNoteProjBoilGrav;
   static QString colBNoteProjStrikeTemp;
   static QString colBNoteProjFinTemp;
   static QString colBNoteProjFinMashTemp;
   static QString colBNoteProjBkVol;
   static QString colBNoteProjOg;
   static QString colBNoteProjFermVol;
   static QString colBNoteProjFg;
   static QString colBNoteProjEff;
   static QString colBNoteProjAbv;
   static QString colBNoteProjAtten;
   static QString colBNoteProjPoints;
   static QString colBNoteProjFermPoints;
   static QString colBNoteBoilOff;
   static QString colBNoteFinalVolume;
   static QString colBNoteNotes;
   static QString colBNoteRecipeId;
   
   static QString tableInstruction;
   static QString colInsDirections;
   static QString colInsHasTimer;
   static QString colInsTimerVal;
   static QString colInsCompleted;
   static QString colInsInterval;
   
   static QString tableRecipe;
   static QString colRecType;
   static QString colRecBrewer;
   static QString colRecAsstBrewer;
   static QString colRecBatchSize;
   static QString colRecBoilSize;
   static QString colRecBoilTime;
   static QString colRecEff;
   static QString colRecOg;
   static QString colRecFg;
   static QString colRecFermStages;
   static QString colRecPrimAge;
   static QString colRecPrimTemp;
   static QString colRecSecAge;
   static QString colRecSecTemp;
   static QString colRecTerAge;
   static QString colRecTerTemp;
   static QString colRecAge;
   static QString colRecAgeTemp;
   static QString colRecDate;
   static QString colRecCarbVol;
   static QString colRecForceCarb;
   static QString colRecPrimSug;
   static QString colRecCarbTemp;
   static QString colRecPrimSugEquiv;
   static QString colRecKegPrimFact;
   static QString colRecNotes;
   static QString colRecTasteNotes;
   static QString colRecTasteRating;
   static QString colRecStyleId;
   static QString colRecMashId;
   static QString colRecEquipId;
   
   static QString tableBtEquipment;
   static QString tableBtFermentable;
   static QString tableBtHop;
   static QString tableBtMisc;
   static QString tableBtStyle;
   static QString tableBtYeast;
   static QString tableBtWater;
   
   static QString tableFermInRec;
   static QString tableHopInRec;
   static QString tableMiscInRec;
   static QString tableWaterInRec;
   static QString tableYeastInRec;
   static QString tableInsInRec;
   
   static QString tableEquipChildren;
   static QString tableFermChildren;
   static QString tableHopChildren;
   static QString tableMiscChildren;
   static QString tableRecChildren;
   static QString tableStyleChildren;
   static QString tableWaterChildren;
   static QString tableYeastChildren;
   
   static QString tableFermInventory;
   static QString tableHopInventory;
   static QString tableMiscInventory;
   static QString tableYeastInventory;
   
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   static bool upgrade;
   /*!
    * \brief Create a blank database whose schema version is \c dbVersion
    */
   static bool create(QSqlDatabase db = QSqlDatabase(), Brewtarget::DBTypes dbType = Brewtarget::NODB);
   
   /*!
    * \brief Migrate from version \c oldVersion to \c oldVersion+1
    */
   static bool migrateNext(int oldVersion, QSqlDatabase db = QSqlDatabase());
   
   /*!
    * \brief Migrate schema from \c oldVersion to \c newVersion
    */
   static bool migrate(int oldVersion, int newVersion, QSqlDatabase db = QSqlDatabase());
   
   //! \brief Current schema version of the given database
   static int currentVersion(QSqlDatabase db = QSqlDatabase());
   static QString foreignKey( QString const& column, QString const& foreignTable );


   static void select_dbStrings(Brewtarget::DBTypes dbType);

   // !\brief create_table is a convenience method to wrap a lot of boiler  plate
   static bool create_table(QSqlQuery q, QString create, QString tableName, Brewtarget::DBTable tableid,
                        QString className="", Brewtarget::DBTable inv_id = Brewtarget::NOTABLE, 
                        Brewtarget::DBTable child_id = Brewtarget::NOTABLE);
   // !\brief I need a meta table
   static bool create_meta(QSqlQuery q);
   // !\brief And another meta table we've already created
   static bool create_settings(QSqlQuery q);
   // \!brief inserts a row into the meta table when a table is created
   static bool insert_meta(QSqlQuery q, QString const& name, Brewtarget::DBTable tableid,
                        QString className="", Brewtarget::DBTable inv_id = Brewtarget::NOTABLE, 
                        Brewtarget::DBTable child_id = Brewtarget::NOTABLE);

   //! brief These create the beerXML tables
   static bool create_equipment(QSqlQuery q);
   static bool create_fermentable(QSqlQuery q);
   static bool create_hop(QSqlQuery q);
   static bool create_misc(QSqlQuery q);
   static bool create_style(QSqlQuery q);
   static bool create_yeast(QSqlQuery q);
   static bool create_water(QSqlQuery q);
   static bool create_mash(QSqlQuery q);
   static bool create_mashstep(QSqlQuery q);
   static bool create_brewnote(QSqlQuery q);
   static bool create_instruction(QSqlQuery q);
   static bool create_recipe(QSqlQuery q);
  
   //! \brief These provide some convenience and reuse
   static bool create_beerXMLTables(QSqlQuery q);
   static bool create_btTables(QSqlQuery q);
   static bool create_inRecipeTables(QSqlQuery q);
   static bool create_inventoryTables(QSqlQuery q);
   static bool create_childrenTables(QSqlQuery q);

   //! \brief This creates a table for a bt_* table
   static bool create_btTable(QSqlQuery q, QString tableName, QString foreignTableName, Brewtarget::DBTable tableid);
   //! \brief This creates a table for *in_recipe 
   static bool create_recipeChildTable(QSqlQuery q, QString tableName, QString foreignTableName, Brewtarget::DBTable tableid);
   //! \brief This creates a table for inventory
   static bool create_inventoryTable(QSqlQuery q, QString tableName, QString foreignTableName, Brewtarget::DBTable tableid);
   //! \brief This creates a beerXML child table
   static bool create_childTable( QSqlQuery q, QString const& tableName, QString const& foreignTable, Brewtarget::DBTable tableid);

   //! \brief Creating triggers is very DB specific. These isolate the specifics
   static bool create_increment_trigger(QSqlQuery q, Brewtarget::DBTypes dbType=Brewtarget::NODB);
   static bool create_pgsql_increment_trigger(QSqlQuery q);
   static bool create_sqlite_increment_trigger(QSqlQuery q);

   static bool create_decrement_trigger(QSqlQuery q, Brewtarget::DBTypes dbType=Brewtarget::NODB);
   static bool create_pgsql_decrement_trigger(QSqlQuery q);
   static bool create_sqlite_decrement_trigger(QSqlQuery q);

   static bool migrate_to_202(QSqlQuery q);
   static bool migrate_to_210(QSqlQuery q);
   static bool migrate_to_4(QSqlQuery q);
   static bool migrate_to_5(QSqlQuery q);
   static bool migrate_to_6(QSqlQuery q);
   
};
