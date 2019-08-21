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

#include "TableSchemaConst.h"
#include "StyleSchema.h"
#include "EquipmentSchema.h"
#include "FermentableSchema.h"
#include "HopSchema.h"
#include "MashSchema.h"
#include "MashStepSchema.h"
#include "MiscSchema.h"
#include "RecipeSchema.h"
#include "YeastSchema.h"
#include "BrewnoteSchema.h"

// We have to hard code this, because we cannot be certain the database is
// available yet -- so no bt_alltables lookups can be allowed
// These HAVE to be in the same order as they are listed in
// Brewtarget::DBTable
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
   ktableRecipe <<
   ktableBrewnote <<
   ktableInstruction <<
// Now for BT internal tables
   ktableBtEquipment <<
   ktableBtFermentable <<
   ktableBtHop <<
   ktableBtMisc <<
   ktableBtStyle <<
   ktableBtYeast <<
   ktableBtWater <<
// Now the in_recipe tables
   ktableFermInRec <<
   ktableHopInRec <<
   ktableMiscInRec <<
   ktableWaterInRec <<
   ktableYeastInRec <<
   ktableInsInRec <<
// child tables next
   ktableEquipChildren <<
   ktableFermChildren <<
   ktableHopChildren <<
   ktableMiscChildren <<
   ktableRecChildren <<
   ktableStyleChildren <<
   ktableWaterChildren <<
   ktableYeastChildren <<
// inventory tables last
   ktableFermInventory <<
   ktableHopInventory <<
   ktableMiscInventory <<
   ktableYeastInventory;

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

   switch( table ) {
      case Brewtarget::NOTABLE:
         break;
      case Brewtarget::BREWNOTETABLE:
         retVal =  defineBrewnoteTable();
         break;
      case Brewtarget::STYLETABLE:
         retVal =  defineStyleTable();
         break;
      case Brewtarget::EQUIPTABLE:
         retVal = defineEquipmentTable();
         break;
      case Brewtarget::FERMTABLE:
         retVal = defineFermentableTable();
         break;
      case Brewtarget::HOPTABLE:
         retVal = defineHopTable();
         break;
      case Brewtarget::MASHTABLE:
         retVal = defineMashTable();
         break;
      case Brewtarget::MASHSTEPTABLE:
         retVal = defineMashstepTable();
         break;
      case Brewtarget::MISCTABLE:
         retVal = defineMiscTable();
         break;
      case Brewtarget::RECTABLE:
         retVal = defineRecipeTable();
         break;
      case Brewtarget::YEASTTABLE:
         retVal = defineYeastTable();
         break;
      default:
         qDebug() << tableName_ << " not implemented yet";
   }

   return retVal;
}

// Finally, the method to create the actual object
QMap<QString,PropertySchema*> TableSchema::defineStyleTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kpropName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
  
   // All db use "category" as the column name
   tmpNames[Brewtarget::NODB] = kcolStyleType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Ale"));
  
   tmpNames[Brewtarget::NODB] = kcolStyleCategoryNumber;
   tmp[kpropCategoryNumber] = new PropertySchema( kpropCategoryNumber, tmpNames, kxmlPropCategoryNumber, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolStyleStyleLetter;
   tmp[kpropStyleLetter] = new PropertySchema( kpropStyleLetter, tmpNames, kxmlPropStyleLetter, QString("text"), QString("") );

   tmpNames[Brewtarget::NODB] = kcolStyleStyleGuide;
   tmp[kpropStyleGuide] = new PropertySchema( kpropStyleGuide, tmpNames, kxmlPropStyleGuide, QString("text"), QString("") );

   tmpNames[Brewtarget::NODB] = kcolStyleOGMin;
   tmp[kpropOGMin] = new PropertySchema( kpropOGMin, tmpNames, kxmlPropOGMin);

   tmpNames[Brewtarget::NODB] = kcolStyleOGMax;
   tmp[kpropOGMax] = new PropertySchema( kpropOGMax, tmpNames, kxmlPropOGMax);

   tmpNames[Brewtarget::NODB] = kcolStyleFGMin;
   tmp[kpropFGMin] = new PropertySchema( kpropFGMin, tmpNames, kxmlPropFGMin);

   tmpNames[Brewtarget::NODB] = kcolStyleFGMax;
   tmp[kpropFGMax] = new PropertySchema( kpropFGMax, tmpNames, kxmlPropFGMax);

   tmpNames[Brewtarget::NODB] = kcolStyleIBUMin;
   tmp[kpropIBUMin] = new PropertySchema( kpropIBUMin, tmpNames, kxmlPropIBUMin);

   tmpNames[Brewtarget::NODB] = kcolStyleIBUMax;
   tmp[kpropIBUMax] = new PropertySchema( kpropIBUMax, tmpNames, kxmlPropIBUMax);

   tmpNames[Brewtarget::NODB] = kcolStyleColorMin;
   tmp[kpropColorMin] = new PropertySchema( kpropColorMin,tmpNames, kxmlPropColorMin);

   tmpNames[Brewtarget::NODB] = kcolStyleColorMax;
   tmp[kpropColorMax] = new PropertySchema( kpropColorMax,tmpNames, kxmlPropColorMax);

   tmpNames[Brewtarget::NODB] = kcolStyleABVMin;
   tmp[kpropABVMin] = new PropertySchema( kpropABVMin, tmpNames, kxmlPropABVMin);

   tmpNames[Brewtarget::NODB] = kcolStyleABVMax;
   tmp[kpropABVMax] = new PropertySchema( kpropABVMax, tmpNames, kxmlPropABVMax);

   tmpNames[Brewtarget::NODB] = kcolStyleCarbMin;
   tmp[kpropCarbMin] = new PropertySchema( kpropCarbMin, tmpNames, kxmlPropCarbMin);

   tmpNames[Brewtarget::NODB] = kcolStyleCarbMax;
   tmp[kpropCarbMax] = new PropertySchema( kpropCarbMax, tmpNames, kxmlPropCarbMax);

   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames, kxmlPropNotes);

   tmpNames[Brewtarget::NODB] = kcolStyleProfile;
   tmp[kpropProfile] = new PropertySchema( kpropProfile, tmpNames, kxmlPropProfile);

   tmpNames[Brewtarget::NODB] = kcolStyleIngredients;
   tmp[kpropIngredients] = new PropertySchema( kpropIngredients, tmpNames, kxmlPropIngredients);

   tmpNames[Brewtarget::NODB] = kcolStyleExamples;
   tmp[kpropExamples] = new PropertySchema( kpropExamples, tmpNames, kxmlPropExamples);
   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineEquipmentTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kpropName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
  
   tmpNames[Brewtarget::NODB] = kcolBoilSize;
   tmp[kpropBoilSize] = new PropertySchema( kpropBoilSize, tmpNames , kxmlPropBoilSize, QString("real"), QVariant(0.0));
  
   tmpNames[Brewtarget::NODB] = kcolBatchSize;
   tmp[kpropBatchSize] = new PropertySchema( kpropBatchSize, tmpNames , kxmlPropBatchSize, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTunVolume;
   tmp[kpropTunVolume] = new PropertySchema( kpropTunVolume, tmpNames , kxmlPropTunVolume, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTunWeight;
   tmp[kpropTunWeight] = new PropertySchema( kpropTunWeight, tmpNames , kxmlPropTunWeight, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTunSpecificHeat;
   tmp[kpropTunSpecificHeat] = new PropertySchema( kpropTunSpecificHeat, tmpNames , kxmlPropTunSpecificHeat, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTopUpWater;
   tmp[kpropTopUpWater] = new PropertySchema( kpropTopUpWater, tmpNames , kxmlPropTopUpWater, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTrubChillerLoss;
   tmp[kpropTrubChillerLoss] = new PropertySchema( kpropTrubChillerLoss, tmpNames , kxmlPropTrubChillerLoss, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolEvapRate;
   tmp[kpropEvaporationRate] = new PropertySchema( kpropEvaporationRate, tmpNames , kxmlPropEvaporationRate, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBoilTime;
   tmp[kpropBoilTime] = new PropertySchema( kpropBoilTime, tmpNames , kxmlPropBoilTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolCalcBoilVolume;
   tmp[kpropCalcBoilVolume] = new PropertySchema( kpropCalcBoilVolume, tmpNames , kxmlPropCalcBoilVolume, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolLauterDeadspace;
   tmp[kpropLauterDeadspace] = new PropertySchema( kpropLauterDeadspace, tmpNames , kxmlPropLauterDeadspace, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTopUpKettle;
   tmp[kpropTopUpKettle] = new PropertySchema( kpropTopUpKettle, tmpNames , kxmlPropTopUpKettle, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopUtilization;
   tmp[kpropHopUtilization] = new PropertySchema( kpropHopUtilization, tmpNames , kxmlPropHopUtilization, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString());

   tmpNames[Brewtarget::NODB] = kcolRealEvapRate;
   tmp[kpropRealEvaporationRate] = new PropertySchema( kpropRealEvaporationRate, tmpNames , kxmlPropRealEvaporationRate, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBoilingPoint;
   tmp[kpropBoilingPoint] = new PropertySchema( kpropBoilingPoint, tmpNames , kxmlPropBoilingPoint, QString("real"), QVariant(100.0));

   tmpNames[Brewtarget::NODB] = kcolAbsorption;
   tmp[kpropAbsorption] = new PropertySchema( kpropAbsorption, tmpNames , kxmlPropGrainAbsorption, QString("real"), QVariant(1.085));

   return tmp;
}

// Finally, the method to create the actual object
QMap<QString,PropertySchema*> TableSchema::defineFermentableTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
 
   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   // Apparently, there is no equipment type in the beerXML spec
   tmpNames[Brewtarget::NODB] = kcolFermType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Grain"));

   tmpNames[Brewtarget::NODB] = kcolAmount;
   tmp[kpropAmountKg] = new PropertySchema( kpropAmountKg, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   // NOTE: We skip inventory, because that's a weird one and always calculated.

   tmpNames[Brewtarget::NODB] = kcolFermYield;
   tmp[kpropYield] = new PropertySchema( kpropYield, tmpNames , kxmlPropYield, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermColor;
   tmp[kpropColor] = new PropertySchema( kpropColor, tmpNames , kxmlPropColor, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermAddAfterBoil;
   tmp[kpropAddAfterBoil] = new PropertySchema( kpropAddAfterBoil, tmpNames , kxmlPropAddAfterBoil, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolFermOrigin;
   tmp[kpropOrigin] = new PropertySchema( kpropOrigin, tmpNames , kxmlPropOrigin, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolFermSupplier;
   tmp[kpropSupplier] = new PropertySchema( kpropSupplier, tmpNames , kxmlPropSupplier, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolFermCoarseFineDiff;
   tmp[kpropCoarseFineDiff] = new PropertySchema( kpropCoarseFineDiff, tmpNames , kxmlPropCoarseFineDiff, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermMoisture;
   tmp[kpropMoisture] = new PropertySchema( kpropMoisture, tmpNames , kxmlPropMoisture, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermDiastaticPower;
   tmp[kpropDiastaticPower] = new PropertySchema( kpropDiastaticPower, tmpNames , kxmlPropDiastaticPower, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermProtein;
   tmp[kpropProtein] = new PropertySchema( kpropProtein, tmpNames , kxmlPropProtein, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermMaxInBatch;
   tmp[kpropMaxInBatch] = new PropertySchema( kpropMaxInBatch, tmpNames , kxmlPropMaxInBatch, QString("real"), QVariant(100.0));

   tmpNames[Brewtarget::NODB] = kcolFermRecommendMash;
   tmp[kpropRecommendMash] = new PropertySchema( kpropRecommendMash, tmpNames, kxmlPropRecommendMash, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolFermIsMashed;
   tmp[kpropIsMashed] = new PropertySchema( kpropIsMashed, tmpNames , kxmlPropIsMashed, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolFermIBUGalPerLb;
   tmp[kpropIBUGalPerLb] = new PropertySchema( kpropIBUGalPerLb, tmpNames , kxmlPropIBUGalPerLb, QString("real"), QVariant(0.0));

   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineHopTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file. 
   tmpNames[Brewtarget::NODB] = kcolName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
 
   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolAmount;
   tmp[kpropAmountKg] = new PropertySchema( kpropAmountKg, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolUse;
   tmp[kpropUse] = new PropertySchema( kpropUse, tmpNames , QString(""), QString("text"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolTime;
   tmp[kpropTime] = new PropertySchema( kpropTime, tmpNames , kxmlPropTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolOrigin;
   tmp[kpropOrigin] = new PropertySchema( kpropOrigin, tmpNames , QString(""), QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolSubstitutes;
   tmp[kpropSubstitutes] = new PropertySchema( kpropSubstitutes, tmpNames , QString(""), QString("text"), QString(""));

   // NOTE: We skip inventory, because that's a weird one and always calculated.

   tmpNames[Brewtarget::NODB] = kcolHopAlpha;
   tmp[kpropAlpha] = new PropertySchema( kpropAlpha, tmpNames , kxmlPropAlpha, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolHopForm;
   tmp[kpropForm] = new PropertySchema( kpropForm, tmpNames , QString(""), QString("text"), QString("Pellet"));

   tmpNames[Brewtarget::NODB] = kcolHopBeta;
   tmp[kpropBeta] = new PropertySchema( kpropBeta, tmpNames , kxmlPropBeta, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopHSI;
   tmp[kpropHSI] = new PropertySchema( kpropHSI, tmpNames , kxmlPropHSI, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopHumulene;
   tmp[kpropHumulene] = new PropertySchema( kpropHumulene, tmpNames , kxmlPropHumulene, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopCaryophyllene;
   tmp[kpropCaryophyllene] = new PropertySchema( kpropCaryophyllene, tmpNames , kxmlPropCaryophyllene, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopCohumulone;
   tmp[kpropCohumulone] = new PropertySchema( kpropCohumulone, tmpNames , kxmlPropCohumulone, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopMyrcene;
   tmp[kpropMyrcene] = new PropertySchema( kpropMyrcene, tmpNames , kxmlPropMyrcene, QString("real"), QVariant(0.0));

   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineMashTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file. 
   tmpNames[Brewtarget::NODB] = kcolName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
 
   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolMashGrainTemp;
   tmp[kpropGrainTemp] = new PropertySchema( kpropGrainTemp, tmpNames , kxmlPropGrainTemp, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashTunTemp;
   tmp[kpropTunTemp] = new PropertySchema( kpropTunTemp, tmpNames , kxmlPropTunTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolMashSpargeTemp;
   tmp[kpropSpargeTemp] = new PropertySchema( kpropSpargeTemp, tmpNames , kxmlPropSpargeTemp, QString("real"), QVariant(74.0));

   tmpNames[Brewtarget::NODB] = kcolMashPH;
   tmp[kpropPH] = new PropertySchema( kpropPH, tmpNames , kxmlPropPH, QString("real"), QVariant(7.0));

   tmpNames[Brewtarget::NODB] = kcolMashTunWeight;
   tmp[kpropTunWeight] = new PropertySchema( kpropTunWeight, tmpNames , kxmlPropTunWeight, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashTunSpecificHeat;
   tmp[kpropTunSpecificHeat] = new PropertySchema( kpropTunSpecificHeat, tmpNames , kxmlPropTunSpecificHeat, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashEquipAdjust;
   tmp[kpropEquipAdjust] = new PropertySchema( kpropEquipAdjust, tmpNames , kxmlPropEquipAdjust, QString("boolean"), QVariant(true));

   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineMashstepTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
 
   // type is always weird, and I wish we hadn't done it this way
   tmpNames[Brewtarget::NODB] = kcolMashstepType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Infusion"));

   tmpNames[Brewtarget::NODB] = kcolMashstepInfuseAmount;
   tmp[kpropInfuseAmount] = new PropertySchema( kpropInfuseAmount, tmpNames , kxmlPropInfuseAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepStepTemp;
   tmp[kpropStepTemp] = new PropertySchema( kpropStepTemp, tmpNames , kxmlPropStepTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepStepTime;
   tmp[kpropStepTime] = new PropertySchema( kpropStepTime, tmpNames , kxmlPropStepTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepRampTime;
   tmp[kpropRampTime] = new PropertySchema( kpropRampTime, tmpNames , kxmlPropRampTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepEndTemp;
   tmp[kpropEndTemp] = new PropertySchema( kpropEndTemp, tmpNames , kxmlPropEndTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepInfuseTemp;
   tmp[kpropInfuseTemp] = new PropertySchema( kpropInfuseTemp, tmpNames , kxmlPropInfuseTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepDecoctionAmount;
   tmp[kpropDecoctionAmount] = new PropertySchema( kpropDecoctionAmount, tmpNames , kxmlPropDecoctionAmount, QString("real"), QVariant(67.0));

//   tmpNames[Brewtarget::NODB] = kcolMashStepStepNumber;
//   tmp[kpropStepNumber] = new PropertySchema( kpropStepNumber, tmpNames , kxmlPropStepNumber, QString("real"), QVariant(67.0));

   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineMiscTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file. 
   tmpNames[Brewtarget::NODB] = kcolName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
 
   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolAmount;
   tmp[kpropAmountKg] = new PropertySchema( kpropAmountKg, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolUse;
   tmp[kpropUse] = new PropertySchema( kpropUse, tmpNames , QString(""), QString("text"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolTime;
   tmp[kpropTime] = new PropertySchema( kpropTime, tmpNames , kxmlPropTime, QString("real"), QVariant(0.0));

   // type is always weird, and I wish we hadn't done it this way
   tmpNames[Brewtarget::NODB] = kcolMiscType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Other"));

   tmpNames[Brewtarget::NODB] = kcolMiscAmountIsWeight;
   tmp[kpropAmountIsWeight] = new PropertySchema( kpropAmountIsWeight, tmpNames , kxmlPropAmountIsWeight, QString("boolean"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolMiscUseFor;
   tmp[kpropUseFor] = new PropertySchema( kpropUseFor, tmpNames , kxmlPropUseFor, QString("text"), QString(""));

   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineRecipeTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file. 
   tmpNames[Brewtarget::NODB] = kcolName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
 
   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolRecipeType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , kxmlPropType, QString("text"), QString("All Grain"));

   tmpNames[Brewtarget::NODB] = kcolRecipeBrewer;
   tmp[kpropBrewer] = new PropertySchema( kpropBrewer, tmpNames , kxmlPropBrewer, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolRecipeAsstBrewer;
   tmp[kpropAsstBrewer] = new PropertySchema( kpropAsstBrewer, tmpNames , kxmlPropAsstBrewer, QString("text"), QString("Brewtarget"));

   tmpNames[Brewtarget::NODB] = kcolRecipeBatchSize;
   tmp[kpropBatchSize] = new PropertySchema( kpropBatchSize, tmpNames , kxmlPropBatchSize, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeBoilSize;
   tmp[kpropBoilSize] = new PropertySchema( kpropBoilSize, tmpNames , kxmlPropBoilSize, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeBoilTime;
   tmp[kpropBoilTime] = new PropertySchema( kpropBoilTime, tmpNames , kxmlPropBoilTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeEfficiency;
   tmp[kpropEfficiency] = new PropertySchema( kpropEfficiency, tmpNames , kxmlPropEfficiency, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeOG;
   tmp[kpropOG] = new PropertySchema( kpropOG, tmpNames , kxmlPropOG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeFG;
   tmp[kpropFG] = new PropertySchema( kpropFG, tmpNames , kxmlPropFG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeFermentationStages;
   tmp[kpropFermentationStages] = new PropertySchema( kpropFermentationStages, tmpNames , kxmlPropFermentationStages, QString("int"), QVariant(0));

   tmpNames[Brewtarget::NODB] = kcolRecipePrimaryAgeDays;
   tmp[kpropPrimaryAgeDays] = new PropertySchema( kpropPrimaryAgeDays, tmpNames , kxmlPropPrimaryAgeDays, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipePrimaryTemp;
   tmp[kpropPrimaryTemp] = new PropertySchema( kpropPrimaryTemp, tmpNames , kxmlPropPrimaryTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeSecondaryAgeDays;
   tmp[kpropSecondaryAgeDays] = new PropertySchema( kpropSecondaryAgeDays, tmpNames , kxmlPropSecondaryAgeDays, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeSecondaryTemp;
   tmp[kpropSecondaryTemp] = new PropertySchema( kpropSecondaryTemp, tmpNames , kxmlPropSecondaryTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeTertiaryAgeDays;
   tmp[kpropTertiaryAgeDays] = new PropertySchema( kpropTertiaryAgeDays, tmpNames , kxmlPropTertiaryAgeDays, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeTertiaryTemp;
   tmp[kpropTertiaryTemp] = new PropertySchema( kpropTertiaryTemp, tmpNames , kxmlPropTertiaryTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeAge;
   tmp[kpropAge] = new PropertySchema( kpropAge, tmpNames , kxmlPropAge, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeAgeTemp;
   tmp[kpropAgeTemp] = new PropertySchema( kpropAgeTemp, tmpNames , kxmlPropAgeTemp, QString("real"), QVariant(20.0));

   // This one is hard. Not sure about the default value, but I think I need
   // it to be a string?
   tmpNames[Brewtarget::NODB] = kcolRecipeDate;
   tmp[kpropDate] = new PropertySchema( kpropDate, tmpNames , kxmlPropDate, QString("date"), QString("now()"));

   tmpNames[Brewtarget::NODB] = kcolRecipeCarbonationVols;
   tmp[kpropCarbonationVols] = new PropertySchema( kpropCarbonationVols, tmpNames , kxmlPropCarbonationVols, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeForcedCarbonation;
   tmp[kpropForcedCarbonation] = new PropertySchema( kpropForcedCarbonation, tmpNames , kxmlPropForcedCarbonation, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolRecipePrimingSugarName;
   tmp[kpropPrimingSugarName] = new PropertySchema( kpropPrimingSugarName, tmpNames , kxmlPropPrimingSugarName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolRecipeCarbonationTemp;
   tmp[kpropCarbonationTemp] = new PropertySchema( kpropCarbonationTemp, tmpNames , kxmlPropCarbonationTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolRecipePrimingSugarEquiv;
   tmp[kpropPrimingSugarEquiv] = new PropertySchema( kpropPrimingSugarEquiv, tmpNames , kxmlPropPrimingSugarEquiv, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeKegPrimingFactor;
   tmp[kpropKegPrimingFactor] = new PropertySchema( kpropKegPrimingFactor, tmpNames , kxmlPropKegPrimingFactor, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeTasteNotes;
   tmp[kpropTasteNotes] = new PropertySchema( kpropTasteNotes, tmpNames , kxmlPropTasteNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolRecipeTasteRating;
   tmp[kpropTasteRating] = new PropertySchema( kpropTasteRating, tmpNames , kxmlPropTasteRating, QString("real"), QVariant(20.0));

   // These require more thought. I think I may need another attribute on the
   // PropertySchema object to indicate this is a foreignkey? For now, don't
/*

   tmpNames[Brewtarget::NODB] = kcolRecipeEquipmentId;
   tmp[kpropEquipmentId] = new PropertySchema( kpropEquipmentId, tmpNames , QString(), QString("int"), QVariant());

   tmpNames[Brewtarget::NODB] = kcolRecipeMashId;
   tmp[kpropMashId] = new PropertySchema( kpropMashId, tmpNames , QString(), QString("int"), QVariant());

   tmpNames[Brewtarget::NODB] = kcolRecipeStyleId;
   tmp[kpropStyleId] = new PropertySchema( kpropStyleId, tmpNames , QString(), QString("int"), QVariant());

   tmpNames[Brewtarget::NODB] = kcolRecipeAncestorId;
   tmp[kpropAncestorId] = new PropertySchema( kpropAncestorId, tmpNames , QString(), QString("int"), QVariant());
*/
   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineYeastTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file. 
   tmpNames[Brewtarget::NODB] = kcolName;
   tmp[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));
 
   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolYeastType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(), QString("text"), QObject::tr("Ale"));

   tmpNames[Brewtarget::NODB] = kcolYeastForm;
   tmp[kpropForm] = new PropertySchema( kpropForm, tmpNames , QString(), QString("text"), QObject::tr("Liquid"));

   tmpNames[Brewtarget::NODB] = kcolYeastAmount;
   tmp[kpropAmount] = new PropertySchema( kpropAmount, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolYeastAmountIsWeight;
   tmp[kpropAmountIsWeight] = new PropertySchema( kpropAmountIsWeight, tmpNames , kxmlPropAmountIsWeight, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolYeastLab;
   tmp[kpropLab] = new PropertySchema( kpropLab, tmpNames , kxmlPropLab, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolYeastProductID;
   tmp[kpropProductID] = new PropertySchema( kpropProductID, tmpNames , kxmlPropProductID, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolYeastMinTemp;
   tmp[kpropMinTemp] = new PropertySchema( kpropMinTemp, tmpNames , kxmlPropMinTemp, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolYeastMaxTemp;
   tmp[kpropMaxTemp] = new PropertySchema( kpropMaxTemp, tmpNames , kxmlPropMaxTemp, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolYeastFlocculation;
   tmp[kpropFlocculation] = new PropertySchema( kpropFlocculation, tmpNames , QString(), QString("text"), QObject::tr("Medium"));

   tmpNames[Brewtarget::NODB] = kcolYeastAttenuation;
   tmp[kpropAttenuation] = new PropertySchema( kpropAttenuationPct, tmpNames , kxmlPropAttenuation, QString("real"), QVariant(75.0));

   tmpNames[Brewtarget::NODB] = kcolYeastBestFor;
   tmp[kpropBestFor] = new PropertySchema( kpropBestFor, tmpNames , kxmlPropBestFor, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolYeastTimesCultured;
   tmp[kpropTimesCultured] = new PropertySchema( kpropTimesCultured, tmpNames , kxmlPropTimesCultured, QString("int"), QVariant(0));

   tmpNames[Brewtarget::NODB] = kcolYeastMaxReuse;
   tmp[kpropMaxReuse] = new PropertySchema( kpropMaxReuse, tmpNames , kxmlPropMaxReuse, QString("int"), QVariant(10));

   tmpNames[Brewtarget::NODB] = kcolYeastAddToSecondary;
   tmp[kpropAddToSecondary] = new PropertySchema( kpropAddToSecondary, tmpNames , kxmlPropAddToSecondary, QString("boolean"), QVariant(false));

   return tmp;
}

QMap<QString,PropertySchema*> TableSchema::defineBrewnoteTable()
{
   QMap<QString,PropertySchema*> tmp;
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolBrewnoteBrewDate;
   tmp[kpropBrewDate] = new PropertySchema( kpropBrewDate, tmpNames , kxmlPropBrewDate, QString("timestamp"), QDateTime());

   tmpNames[Brewtarget::NODB] = kcolBrewnoteFermentDate;
   tmp[kpropFermentDate] = new PropertySchema( kpropFermentDate, tmpNames , kxmlPropFermentDate, QString("timestamp"), QDateTime());

   tmpNames[Brewtarget::NODB] = kcolBrewnoteSpecificGravity;
   tmp[kpropSpecificGravity] = new PropertySchema( kpropSpecificGravity, tmpNames , kxmlPropSpecificGravity, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteVolumeIntoBoil;
   tmp[kpropVolumeIntoBoil] = new PropertySchema( kpropVolumeIntoBoil, tmpNames , kxmlPropVolumeIntoBoil, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteStrikeTemp;
   tmp[kpropStrikeTemp] = new PropertySchema( kpropStrikeTemp, tmpNames , kxmlPropStrikeTemp, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteMashFinalTemp;
   tmp[kpropMashFinalTemp] = new PropertySchema( kpropMashFinalTemp, tmpNames , kxmlPropMashFinalTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteOriginalGravity;
   tmp[kpropOriginalGravity] = new PropertySchema( kpropOriginalGravity, tmpNames , kxmlPropOriginalGravity, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnotePostBoilVolume;
   tmp[kpropPostBoilVolume] = new PropertySchema( kpropPostBoilVolume, tmpNames , kxmlPropPostBoilVolume, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteVolumeIntoFermenter;
   tmp[kpropVolumeIntoFermenter] = new PropertySchema( kpropVolumeIntoFermenter, tmpNames , kxmlPropVolumeIntoFermenter, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnotePitchTemp;
   tmp[kpropPitchTemp] = new PropertySchema( kpropPitchTemp, tmpNames , kxmlPropPitchTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteFinalGravity;
   tmp[kpropFinalGravity] = new PropertySchema( kpropFinalGravity, tmpNames , kxmlPropFinalGravity, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteEfficiencyIntoBoil;
   tmp[kpropEfficiencyIntoBoil] = new PropertySchema( kpropEfficiencyIntoBoil, tmpNames , kxmlPropEfficiencyIntoBoil, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteABV;
   tmp[kpropABV] = new PropertySchema( kpropABV, tmpNames , kxmlPropABV, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedOG;
   tmp[kpropProjectedOG] = new PropertySchema( kpropProjectedOG, tmpNames , kxmlPropProjectedOG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteBrewhouseEfficiency;
   tmp[kpropBrewhouseEfficiency] = new PropertySchema( kpropBrewhouseEfficiency, tmpNames , kxmlPropBrewhouseEfficiency, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedBoilGravity;
   tmp[kpropProjectedBoilGravity] = new PropertySchema( kpropProjectedBoilGravity, tmpNames , kxmlPropProjectedBoilGravity, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedStrikeTemp;
   tmp[kpropProjectedStrikeTemp] = new PropertySchema( kpropProjectedStrikeTemp, tmpNames , kxmlPropProjectedStrikeTemp, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedMashFinishTemp;
   tmp[kpropProjectedMashFinishTemp] = new PropertySchema( kpropProjectedMashFinishTemp, tmpNames , kxmlPropProjectedMashFinishTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedVolumeIntoBoil;
   tmp[kpropProjectedVolumeIntoBoil] = new PropertySchema( kpropProjectedVolumeIntoBoil, tmpNames , kxmlPropProjectedVolumeIntoBoil, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedOG;
   tmp[kpropProjectedOG] = new PropertySchema( kpropProjectedOG, tmpNames , kxmlPropProjectedOG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedVolumeIntoFermenter;
   tmp[kpropProjectedVolumeIntoFermenter] = new PropertySchema( kpropProjectedVolumeIntoFermenter, tmpNames , kxmlPropProjectedVolumeIntoFermenter, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedFG;
   tmp[kpropProjectedFG] = new PropertySchema( kpropProjectedFG, tmpNames , kxmlPropProjectedFG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedEfficiency;
   tmp[kpropProjectedEfficiency] = new PropertySchema( kpropProjectedEfficiency, tmpNames , kxmlPropProjectedEfficiency, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedABV;
   tmp[kpropProjectedABV] = new PropertySchema( kpropProjectedABV, tmpNames , kxmlPropProjectedABV, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedAttenuation;
   tmp[kpropProjectedAttenuation] = new PropertySchema( kpropProjectedAttenuation, tmpNames , kxmlPropProjectedAttenuation, QString("real"), QVariant(75.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedPoints;
   tmp[kpropProjectedPoints] = new PropertySchema( kpropProjectedPoints, tmpNames , kxmlPropProjectedPoints, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedFermentationPoints;
   tmp[kpropProjectedFermentationPoints] = new PropertySchema( kpropProjectedFermentationPoints, tmpNames , kxmlPropProjectedFermentationPoints, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteBoilOff;
   tmp[kpropBoilOff] = new PropertySchema( kpropBoilOff, tmpNames , kxmlPropBoilOff, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteFinalVolume;
   tmp[kpropFinalVolume] = new PropertySchema( kpropFinalVolume, tmpNames , kxmlPropFinalVolume, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteAttenuation;
   tmp[kpropAttenuation] = new PropertySchema( kpropAttenuation, tmpNames , kxmlPropAttenuation, QString("real"), QVariant(1.0));

   /*
   tmpNames[Brewtarget::NODB] = kcolBrewnoteRecipeId;
   tmp[kpropRecipeId] = new PropertySchema( kpropRecipeId, tmpNames , kxmlPropRecipeId, QString("real"), QVariant(1.0));
   */

   return tmp;
}

