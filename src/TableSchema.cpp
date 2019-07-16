/*
 * TableSchema.cpp is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@gmail.com>
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
#include "database.h"
#include <QString>
#include "TableSchema.h"

const QString kpropName("name");

// Define tables names first, because I need them first
static QString ktableMeta("bt_alltables");
static QString ktableSettings("settings");
static QString ktableEquipment("equipment");
static QString ktableFermentable("fermentable");
static QString ktableHop("hop");
static QString ktableMisc("misc");
static QString ktableYeast("yeast");
static QString ktableWater("water");
static QString ktableMash("mash");
static QString ktableMashStep("mashstep");
static QString ktableBrewnote("brewnote");
static QString ktableInstruction("instruction");
static QString ktableRecipe("recipe");

// BT default tables
static QString ktableBtEquipment("bt_equipment");
static QString ktableBtFermentable("bt_fermentable");
static QString ktableBtHop("bt_hop");
static QString ktableBtMisc("bt_misc");
static QString ktableBtStyle("bt_style");
static QString ktableBtYeast("bt_yeast");
static QString ktableBtWater("bt_water");

// In recipe tables
static QString ktableFermInRec("fermentable_in_recipe");
static QString ktableHopInRec("hop_in_recipe");
static QString ktableMiscInRec("misc_in_recipe");
static QString ktableWaterInRec("water_in_recipe");
static QString ktableYeastInRec("yeast_in_recipe");
static QString ktableInsInRec("instruction_in_recipe");

// Children tables
static QString ktableEquipChildren("equipment_children");
static QString ktableFermChildren("fermentable_children");
static QString ktableHopChildren("hop_children");
static QString ktableMiscChildren("misc_children");
static QString ktableRecChildren("recipe_children");
static QString ktableStyleChildren("style_children");
static QString ktableWaterChildren("water_children");
static QString ktableYeastChildren("yeast_children");

// Inventory tables
static QString ktableFermInventory("fermentable_in_inventory");
static QString ktableHopInventory("hop_in_inventory");
static QString ktableMiscInventory("misc_in_inventory");
static QString ktableYeastInventory("yeast_in_inventory");

// Individual tables. This is going to suck
static QString ktableStyle("style");
static QString kcolStyleType("s_type");
static QString kcolStyleCat("category");
static QString kcolStyleCatNum("category_number");
static QString kcolStyleLetter("style_letter");
static QString kcolStyleGuide("style_guide");
static QString kcolStyleOGMin("og_min");
static QString kcolStyleOGMax("og_max");
static QString kcolStyleFGMin("fg_min");
static QString kcolStyleFGMax("fg_max");
static QString kcolStyleIBUMin("ibu_min");
static QString kcolStyleIBUMax("ibu_max");
static QString kcolStyleColorMin("color_min");
static QString kcolStyleColorMax("color_max");
static QString kcolStyleABVMin("abv_min");
static QString kcolStyleABVMax("abv_max");
static QString kcolStyleCarbMin("carb_min");
static QString kcolStyleCarbMax("carb_max");
static QString kcolStyleNotes("notes");
static QString kcolStyleProfile("profile");
static QString kcolStyleIngredients("ingredients");
static QString kcolStyleExamples("examples");

static QString kcolEquipBoilSize("boil_size");
static QString kcolEquipBatchSize("batch_size");
static QString kcolEquipTunVolume("tun_volume");
static QString kcolEquipTunWeight("tun_weight");
static QString kcolEquipTunSpecificHeat("tun_specific_heat");
static QString kcolEquipTopUpWater("top_up_water");
static QString kcolEquipTrubChillerLoss("trub_chiller_loss");
static QString kcolEquipEvapRate("evap_rate");
static QString kcolEquipBoilTime("boil_time");
static QString kcolEquipCalcBoilVolume("calc_boil_volume");
static QString kcolEquipLauterDeadspace("lauter_deadspace");
static QString kcolEquipTopUpKettle("top_up_kettle");
static QString kcolEquipHopUtilization("hop_utilization");
static QString kcolEquipNotes("notes");
static QString kcolEquipRealEvapRate("real_evap_rate");
static QString kcolEquipBoilingPoint("boiling_point");
static QString kcolEquipAbsorption("absorption");

// properties for objects
const QString kpropType("type");
const QString kpropCategory("category");
const QString kpropCategoryNumber("categoryNumber");
const QString kpropStyleLetter("styleLetter");
const QString kpropStyleGuide("styleGuide");
const QString kpropOGMin("ogMin");
const QString kpropOGMax("ogMax");
const QString kpropFGMin("fgMin");
const QString kpropFGMax("fgMax");
const QString kpropIBUMin("ibuMin");
const QString kpropIBUMax("ibuMax");
const QString kpropColorMin("colorMin_srm");
const QString kpropColorMax("colorMax_srm");
const QString kpropCarbMin("carbMin_vol");
const QString kpropCarbMax("carbMax_vol");
const QString kpropABVMin("abvMin_pct");
const QString kpropABVMax("abvMax_pct");
const QString kpropNotes("notes");
const QString kpropProfile("profile");
const QString kpropIngredients("ingredients");
const QString kpropExamples("examples");

// XML Properties
const QString kXmlNameProp("NAME");

// style first
const QString kXmlCategoryProp("CATEGORY");
const QString kXmlCategoryNumberProp("CATEGORY_NUMBER");
const QString kXmlStyleLetterProp("STYLE_LETTER");
const QString kXmlStyleGuideProp("STYLE_GUIDE");
const QString kXmlOGMinProp("OG_MIN");
const QString kXmlOGMaxProp("OG_MAX");
const QString kXmlFGMinProp("FG_MIN");
const QString kXmlFGMaxProp("FG_MAX");
const QString kXmlIBUMinProp("IBU_MIN");
const QString kXmlIBUMaxProp("IBU_MAX");
const QString kXmlColorMinProp("COLOR_MIN");
const QString kXmlColorMaxProp("COLOR_MAX");
const QString kXmlCarbMinProp("CARB_MIN");
const QString kXmlCarbMaxProp("CARB_MAX");
const QString kXmlABVMinProp("ABV_MIN");
const QString kXmlABVMaxProp("ABV_MAX");
const QString kXmlNotesProp("NOTES");
const QString kXmlProfileProp("PROFILE");
const QString kXmlIngredientsProp("INGREDIENTS");
const QString kXmlExamplesProp("EXAMPLES");

// WE have to hard code this, because we cannot be certain the database is
// available yet -- so not bt_alltables lookups can be allowed

static QStringList dbTableToName  = QStringList() <<
   QString("none") <<  // need to handle the NOTABLE index
   ktableMeta <<
   ktableSettings <<
   ktableEquipment <<
   ktableFermentable <<
   ktableHop <<
   ktableMisc <<
   ktableStyle <<
   ktableYeast <<
   ktableWater <<
   ktableMash <<
   ktableMashStep <<
   ktableBrewnote <<
   ktableInstruction <<
   ktableRecipe <<
   ktableBtEquipment <<
   ktableBtFermentable <<
   ktableBtHop <<
   ktableBtMisc <<
   ktableBtStyle <<
   ktableBtYeast <<
   ktableBtWater <<
   ktableFermInRec <<
   ktableHopInRec <<
   ktableMiscInRec <<
   ktableWaterInRec <<
   ktableYeastInRec <<
   ktableInsInRec <<
   ktableEquipChildren <<
   ktableFermChildren <<
   ktableHopChildren <<
   ktableMiscChildren <<
   ktableRecChildren <<
   ktableStyleChildren <<
   ktableWaterChildren <<
   ktableYeastChildren <<
   ktableFermInventory <<
   ktableHopInventory <<
   ktableMiscInventory <<
   ktableYeastInventory;

// QHash<QString, Brewtarget::DBTable>nametoDbTable;

TableSchema::TableSchema(QString tableName)
    : QObject(0), 
    tableName_(tableName),
    dbTable_(static_cast<Brewtarget::DBTable>(0))
{
   for (int i = 1; i < dbTableToName.size(); ++i ) {
      if ( dbTableToName[i] == tableName ) {
         dbTable_ = static_cast<Brewtarget::DBTable>(i);
         break;
      }
   }
   properties_ = defineTable( dbTable_ );
}

TableSchema::TableSchema(Brewtarget::DBTable table) 
    : QObject(0), 
      tableName_( dbTableToName[ static_cast<int>(table) ] ),
      dbTable_(table),
      properties_( defineTable(table) )
{
}

const QString TableSchema::tableName() { return tableName_; }
const Brewtarget::DBTable TableSchema::dbTable() { return dbTable_; }
const QMap<QString,PropertySchema*> TableSchema::properties() const { return properties_; }

const QStringList TableSchema::allPropertyNames() const { return properties_.keys(); }

const QStringList TableSchema::allColumnNames(Brewtarget::DBTypes type) const
{
   QStringList tmp;

   foreach( PropertySchema* prop, properties_ ) {
      tmp.append(prop->colName(type));
   }
   return tmp;
}

const PropertySchema* TableSchema::property(QString prop) const
{
   if ( properties_.contains(prop) ) {
      return properties_.value(prop);
   }
   
   return nullptr;
}

const QString TableSchema::propertyToColumn(QString prop, Brewtarget::DBTypes type) const
{
   if ( properties_.contains(prop) ) {
      return properties_.value(prop)->colName(type);
   }
   else {
      return QString();
   }
}

const QString TableSchema::propertyToXml(QString prop) const
{
   if ( properties_.contains(prop) ) {
      return properties_.value(prop)->xmlName();
   }
   else {
      return QString();
   }
}

const QString TableSchema::xmlToProperty(QString xmlName) const
{
   QString retval;

   foreach ( PropertySchema* prop, properties_ ) {
      if ( prop->xmlName() == xmlName ) {
         retval = prop->propName();
         break;
      }
   }
   return retval;
}

const QString TableSchema::propertyColumnType(QString prop) const
{
   if ( properties_.contains(prop) ) {
      return properties_.value(prop)->colType();
   }
   else {
      return QString();
   }
}

const QVariant TableSchema::propertyColumnDefault(QString prop) const
{
   if ( properties_.contains(prop) ) {
      return properties_.value(prop)->defaultValue();
   }
   else {
      return QString();
   }
}

const int TableSchema::propertyColumnSize(QString prop) const
{
   if ( properties_.contains(prop) ) {
      return properties_.value(prop)->colSize();
   }
   else {
      return 0;
   }
}

QMap<QString,PropertySchema*> TableSchema::defineTable(Brewtarget::DBTable table)
{
   QMap<QString,PropertySchema*> retVal;

   // bad form in a way, but it reads better I think
   if ( table == Brewtarget::NOTABLE ) {
      return retVal;
   }

   switch( table ) {
      case Brewtarget::STYLETABLE:
         return defineStyleTable();
      default:
         qDebug() << tableName_ << " not implemented yet";
   }

   return retVal;
}

QMap<QString,PropertySchema*> TableSchema::defineStyleTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kpropName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kXmlNameProp, QString("text"), QString(""));
  
   // All db use "category" as the column name
   tmpNames[Brewtarget::NODB] = kcolStyleType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Ale"));
  
   tmpNames[Brewtarget::NODB] = kcolStyleCatNum;
   tmp[kpropCategoryNumber] = new PropertySchema( kpropCategoryNumber, tmpNames, kXmlCategoryNumberProp, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolStyleLetter;
   tmp[kpropStyleLetter] = new PropertySchema( kpropStyleLetter, tmpNames, kXmlStyleLetterProp, QString("text"), QString("") );

   tmpNames[Brewtarget::NODB] = kcolStyleGuide;
   tmp[kpropStyleGuide] = new PropertySchema( kpropStyleGuide, tmpNames, kXmlStyleGuideProp, QString("text"), QString("") );

   tmpNames[Brewtarget::NODB] = kcolStyleOGMin;
   tmp[kpropOGMin] = new PropertySchema( kpropOGMin, tmpNames, kXmlOGMinProp);

   tmpNames[Brewtarget::NODB] = kcolStyleOGMax;
   tmp[kpropOGMax] = new PropertySchema( kpropOGMax, tmpNames, kXmlOGMaxProp);

   tmpNames[Brewtarget::NODB] = kcolStyleFGMin;
   tmp[kpropFGMin] = new PropertySchema( kpropFGMin, tmpNames, kXmlFGMinProp);

   tmpNames[Brewtarget::NODB] = kcolStyleFGMax;
   tmp[kpropFGMax] = new PropertySchema( kpropFGMax, tmpNames, kXmlFGMaxProp);

   tmpNames[Brewtarget::NODB] = kcolStyleIBUMin;
   tmp[kpropIBUMin] = new PropertySchema( kpropIBUMin, tmpNames, kXmlIBUMinProp);

   tmpNames[Brewtarget::NODB] = kcolStyleIBUMax;
   tmp[kpropIBUMax] = new PropertySchema( kpropIBUMax, tmpNames, kXmlIBUMaxProp);

   tmpNames[Brewtarget::NODB] = kcolStyleColorMin;
   tmp[kpropColorMin] = new PropertySchema( kpropColorMin,tmpNames, kXmlColorMinProp);

   tmpNames[Brewtarget::NODB] = kcolStyleColorMax;
   tmp[kpropColorMax] = new PropertySchema( kpropColorMax,tmpNames, kXmlColorMaxProp);

   tmpNames[Brewtarget::NODB] = kcolStyleABVMin;
   tmp[kpropABVMin] = new PropertySchema( kpropABVMin, tmpNames, kXmlABVMinProp);

   tmpNames[Brewtarget::NODB] = kcolStyleABVMax;
   tmp[kpropABVMax] = new PropertySchema( kpropABVMax, tmpNames, kXmlABVMaxProp);

   tmpNames[Brewtarget::NODB] = kcolStyleCarbMin;
   tmp[kpropCarbMin] = new PropertySchema( kpropCarbMin, tmpNames, kXmlCarbMinProp);

   tmpNames[Brewtarget::NODB] = kcolStyleCarbMax;
   tmp[kpropCarbMax] = new PropertySchema( kpropCarbMax, tmpNames, kXmlCarbMaxProp);

   tmpNames[Brewtarget::NODB] = kcolStyleNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames, kXmlNotesProp );

   tmpNames[Brewtarget::NODB] = kcolStyleProfile;
   tmp[kpropProfile] = new PropertySchema( kpropProfile, tmpNames, kXmlProfileProp);

   tmpNames[Brewtarget::NODB] = kcolStyleIngredients;
   tmp[kpropIngredients] = new PropertySchema( kpropIngredients, tmpNames, kXmlIngredientsProp);

   tmpNames[Brewtarget::NODB] = kcolStyleExamples;
   tmp[kpropExamples] = new PropertySchema( kpropExamples, tmpNames, kXmlExamplesProp);
   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineEquipmentTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // this is a special case -- name is name the DB over

   return tmp;
}
