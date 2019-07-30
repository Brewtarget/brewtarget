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
#include "StyleTableSchema.h"
#include "EquipmentTableSchema.h"
#include "FermentableTableSchema.h"
#include "HopTableSchema.h"
#include "MiscTableSchema.h"

// We have to hard code this, because we cannot be certain the database is
// available yet -- so no bt_alltables lookups can be allowed

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
      case Brewtarget::MISCTABLE:
         retVal = defineMiscTable();
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

// Finally, the method to create the actual object
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
   tmp[kpropAmount] = new PropertySchema( kpropAmount, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

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
   tmp[kpropAmount] = new PropertySchema( kpropAmount, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolUse;
   tmp[kpropUse] = new PropertySchema( kpropUse, tmpNames , QString(""), QString("text"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolTime;
   tmp[kpropAmount] = new PropertySchema( kpropTime, tmpNames , kxmlPropTime, QString("real"), QVariant(0.0));

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
   tmp[kpropAmount] = new PropertySchema( kpropBeta, tmpNames , kxmlPropBeta, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopHSI;
   tmp[kpropAmount] = new PropertySchema( kpropHSI, tmpNames , kxmlPropHSI, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopHumulene;
   tmp[kpropAmount] = new PropertySchema( kpropHumulene, tmpNames , kxmlPropHumulene, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopCaryophyllene;
   tmp[kpropAmount] = new PropertySchema( kpropCaryophyllene, tmpNames , kxmlPropCaryophyllene, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopCohumulone;
   tmp[kpropAmount] = new PropertySchema( kpropCohumulone, tmpNames , kxmlPropCohumulone, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopMyrcene;
   tmp[kpropAmount] = new PropertySchema( kpropMyrcene, tmpNames , kxmlPropMyrcene, QString("real"), QVariant(0.0));

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
   tmp[kpropAmount] = new PropertySchema( kpropAmount, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolUse;
   tmp[kpropUse] = new PropertySchema( kpropUse, tmpNames , QString(""), QString("text"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolTime;
   tmp[kpropAmount] = new PropertySchema( kpropTime, tmpNames , kxmlPropTime, QString("real"), QVariant(0.0));

   // type is always weird, and I wish we hadn't done it this way
   tmpNames[Brewtarget::NODB] = kcolMiscType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Other"));

   tmpNames[Brewtarget::NODB] = kcolMiscAmountIsWeight;
   tmp[kpropAmountIsWeight] = new PropertySchema( kpropAmountIsWeight, tmpNames , kxmlPropAmountIsWeight, QString("boolean"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolMiscUseFor;
   tmp[kpropUseFor] = new PropertySchema( kpropUseFor, tmpNames , kxmlPropUseFor, QString("text"), QString(""));

   return tmp;
}

