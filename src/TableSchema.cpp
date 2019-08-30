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
#include "PropertySchema.h"
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

TableSchema::TableSchema(Brewtarget::DBTable table)
    : QObject(nullptr),
      m_tableName( dbTableToName[ static_cast<int>(table) ] ),
      m_dbTable(table)
{
    // for this bit of ugly, I gain a lot of utility.
    defineTable();
}

const QString TableSchema::tableName() { return m_tableName; }
Brewtarget::DBTable TableSchema::dbTable() { return m_dbTable; }
const QMap<QString,PropertySchema*> TableSchema::properties() const { return m_properties; }

const QStringList TableSchema::allPropertyNames() const { return m_properties.keys(); }
const QStringList TableSchema::allForeignKeyNames() const { return m_foreignKeys.keys(); }

const QStringList TableSchema::allColumnNames(Brewtarget::DBTypes type) const
{
   QStringList tmp;

   foreach( PropertySchema* prop, m_properties ) {
      tmp.append(prop->colName(type));
   }
   return tmp;
}
const QStringList TableSchema::allForeignKeyColumnNames(Brewtarget::DBTypes type) const
{
   QStringList tmp;

   foreach( PropertySchema* prop, m_foreignKeys ) {
      tmp.append(prop->colName(type));
   }
   return tmp;
}

const PropertySchema* TableSchema::property(QString prop) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop);
   }

   return nullptr;
}

const QString TableSchema::propertyToColumn(QString prop, Brewtarget::DBTypes type) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->colName(type);
   }
   else {
      return QString();
   }
}

const QString TableSchema::propertyToXml(QString prop) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->xmlName();
   }
   else {
      return QString();
   }
}

const QString TableSchema::xmlToProperty(QString xmlName) const
{
   QString retval;

   foreach ( PropertySchema* prop, m_properties ) {
      if ( prop->xmlName() == xmlName ) {
         retval = prop->propName();
         break;
      }
   }
   return retval;
}

const QString TableSchema::propertyColumnType(QString prop) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->colType();
   }
   else {
      return QString();
   }
}

const QVariant TableSchema::propertyColumnDefault(QString prop) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->defaultValue();
   }
   else {
      return QString();
   }
}

int TableSchema::propertyColumnSize(QString prop) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->colSize();
   }
   else {
      return 0;
   }
}

// This got long. Not sure if there's a better way to do it.
void TableSchema::defineTable()
{
   switch( m_dbTable ) {
      case Brewtarget::BREWNOTETABLE:
         defineBrewnoteTable();
         break;
      case Brewtarget::STYLETABLE:
         defineStyleTable();
         break;
      case Brewtarget::EQUIPTABLE:
         defineEquipmentTable();
         break;
      case Brewtarget::FERMTABLE:
         defineFermentableTable();
         break;
      case Brewtarget::HOPTABLE:
         defineHopTable();
         break;
      case Brewtarget::MASHTABLE:
         defineMashTable();
         break;
      case Brewtarget::MASHSTEPTABLE:
         defineMashstepTable();
         break;
      case Brewtarget::MISCTABLE:
         defineMiscTable();
         break;
      case Brewtarget::RECTABLE:
         defineRecipeTable();
         break;
      case Brewtarget::YEASTTABLE:
         defineYeastTable();
         break;
      case Brewtarget::BT_EQUIPTABLE:
         defineInRecipeTable(kcolEquipmentId, Brewtarget::EQUIPTABLE);
         break;
      case Brewtarget::BT_FERMTABLE:
         defineInRecipeTable(kcolFermentableId, Brewtarget::FERMTABLE);
         break;
      case Brewtarget::BT_HOPTABLE:
         defineInRecipeTable(kcolHopId, Brewtarget::HOPTABLE);
         break;
      case Brewtarget::BT_MISCTABLE:
         defineInRecipeTable(kcolMiscId, Brewtarget::MISCTABLE);
         break;
      case Brewtarget::BT_STYLETABLE:
         defineInRecipeTable(kcolStyleId, Brewtarget::STYLETABLE);
         break;
      case Brewtarget::BT_WATERTABLE:
         defineInRecipeTable(kcolWaterId, Brewtarget::WATERTABLE);
         break;
      case Brewtarget::BT_YEASTTABLE:
         defineInRecipeTable(kcolYeastId, Brewtarget::YEASTTABLE);
         break;
      case Brewtarget::EQUIPCHILDTABLE:
         defineChildTable(Brewtarget::EQUIPTABLE);
         break;
      case Brewtarget::FERMCHILDTABLE:
         defineChildTable(Brewtarget::FERMTABLE);
         break;
      case Brewtarget::HOPCHILDTABLE:
         defineChildTable(Brewtarget::HOPTABLE);
         break;
      case Brewtarget::MISCCHILDTABLE:
         defineChildTable(Brewtarget::MISCTABLE);
         break;
      case Brewtarget::RECIPECHILDTABLE:
         defineChildTable(Brewtarget::RECTABLE);
         break;
      case Brewtarget::STYLECHILDTABLE:
         defineChildTable(Brewtarget::STYLETABLE);
         break;
      case Brewtarget::WATERCHILDTABLE:
         defineChildTable(Brewtarget::WATERTABLE);
         break;
      case Brewtarget::YEASTCHILDTABLE:
         defineChildTable(Brewtarget::YEASTTABLE);
         break;
      case Brewtarget::FERMINRECTABLE:
         defineInRecipeTable(kcolFermentableId, Brewtarget::FERMTABLE);
         break;
      case Brewtarget::HOPINRECTABLE:
         defineInRecipeTable(kcolHopId, Brewtarget::HOPTABLE);
         break;
      case Brewtarget::MISCINRECTABLE:
         defineInRecipeTable(kcolMiscId, Brewtarget::MISCTABLE);
         break;
      case Brewtarget::WATERINRECTABLE:
         defineInRecipeTable(kcolWaterId, Brewtarget::WATERTABLE);
         break;
      case Brewtarget::YEASTINRECTABLE:
         defineInRecipeTable(kcolYeastId, Brewtarget::YEASTTABLE);
         break;
      case Brewtarget::FERMINVTABLE:
         defineFermInventoryTable();
         break;
      case Brewtarget::HOPINVTABLE:
         defineHopInventoryTable();
         break;
      case Brewtarget::MISCINVTABLE:
         defineMiscInventoryTable();
         break;
      case Brewtarget::YEASTINVTABLE:
         defineYeastInventoryTable();
         break;
      default:
         break;
   }
}

// Finally, the methods to define the properties and foreign keys
void TableSchema::defineStyleTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kpropName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   // All db use "category" as the column name
   tmpNames[Brewtarget::NODB] = kcolStyleType;
   m_properties[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Ale"));

   tmpNames[Brewtarget::NODB] = kcolStyleCategoryNumber;
   m_properties[kpropCategoryNumber] = new PropertySchema( kpropCategoryNumber, tmpNames, kxmlPropCategoryNumber, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolStyleStyleLetter;
   m_properties[kpropStyleLetter] = new PropertySchema( kpropStyleLetter, tmpNames, kxmlPropStyleLetter, QString("text"), QString("") );

   tmpNames[Brewtarget::NODB] = kcolStyleStyleGuide;
   m_properties[kpropStyleGuide] = new PropertySchema( kpropStyleGuide, tmpNames, kxmlPropStyleGuide, QString("text"), QString("") );

   tmpNames[Brewtarget::NODB] = kcolStyleOGMin;
   m_properties[kpropOGMin] = new PropertySchema( kpropOGMin, tmpNames, kxmlPropOGMin);

   tmpNames[Brewtarget::NODB] = kcolStyleOGMax;
   m_properties[kpropOGMax] = new PropertySchema( kpropOGMax, tmpNames, kxmlPropOGMax);

   tmpNames[Brewtarget::NODB] = kcolStyleFGMin;
   m_properties[kpropFGMin] = new PropertySchema( kpropFGMin, tmpNames, kxmlPropFGMin);

   tmpNames[Brewtarget::NODB] = kcolStyleFGMax;
   m_properties[kpropFGMax] = new PropertySchema( kpropFGMax, tmpNames, kxmlPropFGMax);

   tmpNames[Brewtarget::NODB] = kcolStyleIBUMin;
   m_properties[kpropIBUMin] = new PropertySchema( kpropIBUMin, tmpNames, kxmlPropIBUMin);

   tmpNames[Brewtarget::NODB] = kcolStyleIBUMax;
   m_properties[kpropIBUMax] = new PropertySchema( kpropIBUMax, tmpNames, kxmlPropIBUMax);

   tmpNames[Brewtarget::NODB] = kcolStyleColorMin;
   m_properties[kpropColorMin] = new PropertySchema( kpropColorMin,tmpNames, kxmlPropColorMin);

   tmpNames[Brewtarget::NODB] = kcolStyleColorMax;
   m_properties[kpropColorMax] = new PropertySchema( kpropColorMax,tmpNames, kxmlPropColorMax);

   tmpNames[Brewtarget::NODB] = kcolStyleABVMin;
   m_properties[kpropABVMin] = new PropertySchema( kpropABVMin, tmpNames, kxmlPropABVMin);

   tmpNames[Brewtarget::NODB] = kcolStyleABVMax;
   m_properties[kpropABVMax] = new PropertySchema( kpropABVMax, tmpNames, kxmlPropABVMax);

   tmpNames[Brewtarget::NODB] = kcolStyleCarbMin;
   m_properties[kpropCarbMin] = new PropertySchema( kpropCarbMin, tmpNames, kxmlPropCarbMin);

   tmpNames[Brewtarget::NODB] = kcolStyleCarbMax;
   m_properties[kpropCarbMax] = new PropertySchema( kpropCarbMax, tmpNames, kxmlPropCarbMax);

   tmpNames[Brewtarget::NODB] = kcolNotes;
   m_properties[kpropNotes] = new PropertySchema( kpropNotes, tmpNames, kxmlPropNotes);

   tmpNames[Brewtarget::NODB] = kcolStyleProfile;
   m_properties[kpropProfile] = new PropertySchema( kpropProfile, tmpNames, kxmlPropProfile);

   tmpNames[Brewtarget::NODB] = kcolStyleIngredients;
   m_properties[kpropIngredients] = new PropertySchema( kpropIngredients, tmpNames, kxmlPropIngredients);

   tmpNames[Brewtarget::NODB] = kcolStyleExamples;
   m_properties[kpropExamples] = new PropertySchema( kpropExamples, tmpNames, kxmlPropExamples);
}

void TableSchema::defineEquipmentTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kpropName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolBoilSize;
   m_properties[kpropBoilSize] = new PropertySchema( kpropBoilSize, tmpNames , kxmlPropBoilSize, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBatchSize;
   m_properties[kpropBatchSize] = new PropertySchema( kpropBatchSize, tmpNames , kxmlPropBatchSize, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTunVolume;
   m_properties[kpropTunVolume] = new PropertySchema( kpropTunVolume, tmpNames , kxmlPropTunVolume, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTunWeight;
   m_properties[kpropTunWeight] = new PropertySchema( kpropTunWeight, tmpNames , kxmlPropTunWeight, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTunSpecificHeat;
   m_properties[kpropTunSpecificHeat] = new PropertySchema( kpropTunSpecificHeat, tmpNames , kxmlPropTunSpecificHeat, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTopUpWater;
   m_properties[kpropTopUpWater] = new PropertySchema( kpropTopUpWater, tmpNames , kxmlPropTopUpWater, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTrubChillerLoss;
   m_properties[kpropTrubChillerLoss] = new PropertySchema( kpropTrubChillerLoss, tmpNames , kxmlPropTrubChillerLoss, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolEvapRate;
   m_properties[kpropEvaporationRate] = new PropertySchema( kpropEvaporationRate, tmpNames , kxmlPropEvaporationRate, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBoilTime;
   m_properties[kpropBoilTime] = new PropertySchema( kpropBoilTime, tmpNames , kxmlPropBoilTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolCalcBoilVolume;
   m_properties[kpropCalcBoilVolume] = new PropertySchema( kpropCalcBoilVolume, tmpNames , kxmlPropCalcBoilVolume, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolLauterDeadspace;
   m_properties[kpropLauterDeadspace] = new PropertySchema( kpropLauterDeadspace, tmpNames , kxmlPropLauterDeadspace, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolTopUpKettle;
   m_properties[kpropTopUpKettle] = new PropertySchema( kpropTopUpKettle, tmpNames , kxmlPropTopUpKettle, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopUtilization;
   m_properties[kpropHopUtilization] = new PropertySchema( kpropHopUtilization, tmpNames , kxmlPropHopUtilization, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolNotes;
   m_properties[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString());

   tmpNames[Brewtarget::NODB] = kcolRealEvapRate;
   m_properties[kpropRealEvaporationRate] = new PropertySchema( kpropRealEvaporationRate, tmpNames , kxmlPropRealEvaporationRate, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBoilingPoint;
   m_properties[kpropBoilingPoint] = new PropertySchema( kpropBoilingPoint, tmpNames , kxmlPropBoilingPoint, QString("real"), QVariant(100.0));

   tmpNames[Brewtarget::NODB] = kcolAbsorption;
   m_properties[kpropAbsorption] = new PropertySchema( kpropAbsorption, tmpNames , kxmlPropGrainAbsorption, QString("real"), QVariant(1.085));

}

void TableSchema::defineFermentableTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolNotes;
   m_properties[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   // Apparently, there is no equipment type in the beerXML spec
   tmpNames[Brewtarget::NODB] = kcolFermType;
   m_properties[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Grain"));

   tmpNames[Brewtarget::NODB] = kcolAmount;
   m_properties[kpropAmountKg] = new PropertySchema( kpropAmountKg, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   // NOTE: We skip inventory, because that's a weird one and always calculated.

   tmpNames[Brewtarget::NODB] = kcolFermYield;
   m_properties[kpropYield] = new PropertySchema( kpropYield, tmpNames , kxmlPropYield, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermColor;
   m_properties[kpropColor] = new PropertySchema( kpropColor, tmpNames , kxmlPropColor, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermAddAfterBoil;
   m_properties[kpropAddAfterBoil] = new PropertySchema( kpropAddAfterBoil, tmpNames , kxmlPropAddAfterBoil, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolFermOrigin;
   m_properties[kpropOrigin] = new PropertySchema( kpropOrigin, tmpNames , kxmlPropOrigin, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolFermSupplier;
   m_properties[kpropSupplier] = new PropertySchema( kpropSupplier, tmpNames , kxmlPropSupplier, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolFermCoarseFineDiff;
   m_properties[kpropCoarseFineDiff] = new PropertySchema( kpropCoarseFineDiff, tmpNames , kxmlPropCoarseFineDiff, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermMoisture;
   m_properties[kpropMoisture] = new PropertySchema( kpropMoisture, tmpNames , kxmlPropMoisture, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermDiastaticPower;
   m_properties[kpropDiastaticPower] = new PropertySchema( kpropDiastaticPower, tmpNames , kxmlPropDiastaticPower, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermProtein;
   m_properties[kpropProtein] = new PropertySchema( kpropProtein, tmpNames , kxmlPropProtein, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermMaxInBatch;
   m_properties[kpropMaxInBatch] = new PropertySchema( kpropMaxInBatch, tmpNames , kxmlPropMaxInBatch, QString("real"), QVariant(100.0));

   tmpNames[Brewtarget::NODB] = kcolFermRecommendMash;
   m_properties[kpropRecommendMash] = new PropertySchema( kpropRecommendMash, tmpNames, kxmlPropRecommendMash, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolFermIsMashed;
   m_properties[kpropIsMashed] = new PropertySchema( kpropIsMashed, tmpNames , kxmlPropIsMashed, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolFermIBUGalPerLb;
   m_properties[kpropIBUGalPerLb] = new PropertySchema( kpropIBUGalPerLb, tmpNames , kxmlPropIBUGalPerLb, QString("real"), QVariant(0.0));

}

void TableSchema::defineHopTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file.
   tmpNames[Brewtarget::NODB] = kcolName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolNotes;
   m_properties[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolAmount;
   m_properties[kpropAmountKg] = new PropertySchema( kpropAmountKg, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolUse;
   m_properties[kpropUse] = new PropertySchema( kpropUse, tmpNames , QString(""), QString("text"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolTime;
   m_properties[kpropTime] = new PropertySchema( kpropTime, tmpNames , kxmlPropTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolOrigin;
   m_properties[kpropOrigin] = new PropertySchema( kpropOrigin, tmpNames , QString(""), QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolSubstitutes;
   m_properties[kpropSubstitutes] = new PropertySchema( kpropSubstitutes, tmpNames , QString(""), QString("text"), QString(""));

   // NOTE: We skip inventory, because that's a weird one and always calculated.

   tmpNames[Brewtarget::NODB] = kcolHopAlpha;
   m_properties[kpropAlpha] = new PropertySchema( kpropAlpha, tmpNames , kxmlPropAlpha, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopType;
   m_properties[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolHopForm;
   m_properties[kpropForm] = new PropertySchema( kpropForm, tmpNames , QString(""), QString("text"), QString("Pellet"));

   tmpNames[Brewtarget::NODB] = kcolHopBeta;
   m_properties[kpropBeta] = new PropertySchema( kpropBeta, tmpNames , kxmlPropBeta, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopHSI;
   m_properties[kpropHSI] = new PropertySchema( kpropHSI, tmpNames , kxmlPropHSI, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopHumulene;
   m_properties[kpropHumulene] = new PropertySchema( kpropHumulene, tmpNames , kxmlPropHumulene, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopCaryophyllene;
   m_properties[kpropCaryophyllene] = new PropertySchema( kpropCaryophyllene, tmpNames , kxmlPropCaryophyllene, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopCohumulone;
   m_properties[kpropCohumulone] = new PropertySchema( kpropCohumulone, tmpNames , kxmlPropCohumulone, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopMyrcene;
   m_properties[kpropMyrcene] = new PropertySchema( kpropMyrcene, tmpNames , kxmlPropMyrcene, QString("real"), QVariant(0.0));

}

void TableSchema::defineMashTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file.
   tmpNames[Brewtarget::NODB] = kcolName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolNotes;
   m_properties[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolMashGrainTemp;
   m_properties[kpropGrainTemp] = new PropertySchema( kpropGrainTemp, tmpNames , kxmlPropGrainTemp, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashTunTemp;
   m_properties[kpropTunTemp] = new PropertySchema( kpropTunTemp, tmpNames , kxmlPropTunTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolMashSpargeTemp;
   m_properties[kpropSpargeTemp] = new PropertySchema( kpropSpargeTemp, tmpNames , kxmlPropSpargeTemp, QString("real"), QVariant(74.0));

   tmpNames[Brewtarget::NODB] = kcolMashPH;
   m_properties[kpropPH] = new PropertySchema( kpropPH, tmpNames , kxmlPropPH, QString("real"), QVariant(7.0));

   tmpNames[Brewtarget::NODB] = kcolMashTunWeight;
   m_properties[kpropTunWeight] = new PropertySchema( kpropTunWeight, tmpNames , kxmlPropTunWeight, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashTunSpecificHeat;
   m_properties[kpropTunSpecificHeat] = new PropertySchema( kpropTunSpecificHeat, tmpNames , kxmlPropTunSpecificHeat, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashEquipAdjust;
   m_properties[kpropEquipAdjust] = new PropertySchema( kpropEquipAdjust, tmpNames , kxmlPropEquipAdjust, QString("boolean"), QVariant(true));

}

void TableSchema::defineMashstepTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   // type is always weird, and I wish we hadn't done it this way
   tmpNames[Brewtarget::NODB] = kcolMashstepType;
   m_properties[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Infusion"));

   tmpNames[Brewtarget::NODB] = kcolMashstepInfuseAmount;
   m_properties[kpropInfuseAmount] = new PropertySchema( kpropInfuseAmount, tmpNames , kxmlPropInfuseAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepStepTemp;
   m_properties[kpropStepTemp] = new PropertySchema( kpropStepTemp, tmpNames , kxmlPropStepTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepStepTime;
   m_properties[kpropStepTime] = new PropertySchema( kpropStepTime, tmpNames , kxmlPropStepTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepRampTime;
   m_properties[kpropRampTime] = new PropertySchema( kpropRampTime, tmpNames , kxmlPropRampTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepEndTemp;
   m_properties[kpropEndTemp] = new PropertySchema( kpropEndTemp, tmpNames , kxmlPropEndTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepInfuseTemp;
   m_properties[kpropInfuseTemp] = new PropertySchema( kpropInfuseTemp, tmpNames , kxmlPropInfuseTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolMashstepDecoctionAmount;
   m_properties[kpropDecoctionAmount] = new PropertySchema( kpropDecoctionAmount, tmpNames , kxmlPropDecoctionAmount, QString("real"), QVariant(67.0));

}

void TableSchema::defineMiscTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file.
   tmpNames[Brewtarget::NODB] = kcolName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolNotes;
   m_properties[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolAmount;
   m_properties[kpropAmountKg] = new PropertySchema( kpropAmountKg, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolUse;
   m_properties[kpropUse] = new PropertySchema( kpropUse, tmpNames , QString(""), QString("text"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolTime;
   m_properties[kpropTime] = new PropertySchema( kpropTime, tmpNames , kxmlPropTime, QString("real"), QVariant(0.0));

   // type is always weird, and I wish we hadn't done it this way
   tmpNames[Brewtarget::NODB] = kcolMiscType;
   m_properties[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Other"));

   tmpNames[Brewtarget::NODB] = kcolMiscAmountIsWeight;
   m_properties[kpropAmountIsWeight] = new PropertySchema( kpropAmountIsWeight, tmpNames , kxmlPropAmountIsWeight, QString("boolean"), QString("Boil"));

   tmpNames[Brewtarget::NODB] = kcolMiscUseFor;
   m_properties[kpropUseFor] = new PropertySchema( kpropUseFor, tmpNames , kxmlPropUseFor, QString("text"), QString(""));

}

void TableSchema::defineRecipeTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file.
   tmpNames[Brewtarget::NODB] = kcolName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolNotes;
   m_properties[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolRecipeType;
   m_properties[kpropType] = new PropertySchema( kpropType, tmpNames , kxmlPropType, QString("text"), QString("All Grain"));

   tmpNames[Brewtarget::NODB] = kcolRecipeBrewer;
   m_properties[kpropBrewer] = new PropertySchema( kpropBrewer, tmpNames , kxmlPropBrewer, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolRecipeAsstBrewer;
   m_properties[kpropAsstBrewer] = new PropertySchema( kpropAsstBrewer, tmpNames , kxmlPropAsstBrewer, QString("text"), QString("Brewtarget"));

   tmpNames[Brewtarget::NODB] = kcolRecipeBatchSize;
   m_properties[kpropBatchSize] = new PropertySchema( kpropBatchSize, tmpNames , kxmlPropBatchSize, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeBoilSize;
   m_properties[kpropBoilSize] = new PropertySchema( kpropBoilSize, tmpNames , kxmlPropBoilSize, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeBoilTime;
   m_properties[kpropBoilTime] = new PropertySchema( kpropBoilTime, tmpNames , kxmlPropBoilTime, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeEfficiency;
   m_properties[kpropEfficiency] = new PropertySchema( kpropEfficiency, tmpNames , kxmlPropEfficiency, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeOG;
   m_properties[kpropOG] = new PropertySchema( kpropOG, tmpNames , kxmlPropOG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeFG;
   m_properties[kpropFG] = new PropertySchema( kpropFG, tmpNames , kxmlPropFG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeFermentationStages;
   m_properties[kpropFermentationStages] = new PropertySchema( kpropFermentationStages, tmpNames , kxmlPropFermentationStages, QString("int"), QVariant(0));

   tmpNames[Brewtarget::NODB] = kcolRecipePrimaryAgeDays;
   m_properties[kpropPrimaryAgeDays] = new PropertySchema( kpropPrimaryAgeDays, tmpNames , kxmlPropPrimaryAgeDays, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipePrimaryTemp;
   m_properties[kpropPrimaryTemp] = new PropertySchema( kpropPrimaryTemp, tmpNames , kxmlPropPrimaryTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeSecondaryAgeDays;
   m_properties[kpropSecondaryAgeDays] = new PropertySchema( kpropSecondaryAgeDays, tmpNames , kxmlPropSecondaryAgeDays, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeSecondaryTemp;
   m_properties[kpropSecondaryTemp] = new PropertySchema( kpropSecondaryTemp, tmpNames , kxmlPropSecondaryTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeTertiaryAgeDays;
   m_properties[kpropTertiaryAgeDays] = new PropertySchema( kpropTertiaryAgeDays, tmpNames , kxmlPropTertiaryAgeDays, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeTertiaryTemp;
   m_properties[kpropTertiaryTemp] = new PropertySchema( kpropTertiaryTemp, tmpNames , kxmlPropTertiaryTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeAge;
   m_properties[kpropAge] = new PropertySchema( kpropAge, tmpNames , kxmlPropAge, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeAgeTemp;
   m_properties[kpropAgeTemp] = new PropertySchema( kpropAgeTemp, tmpNames , kxmlPropAgeTemp, QString("real"), QVariant(20.0));

   // This one is hard. Not sure about the default value, but I think I need
   // it to be a string?
   tmpNames[Brewtarget::NODB] = kcolRecipeDate;
   m_properties[kpropDate] = new PropertySchema( kpropDate, tmpNames , kxmlPropDate, QString("date"), QString("now()"));

   tmpNames[Brewtarget::NODB] = kcolRecipeCarbonationVols;
   m_properties[kpropCarbonationVols] = new PropertySchema( kpropCarbonationVols, tmpNames , kxmlPropCarbonationVols, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeForcedCarbonation;
   m_properties[kpropForcedCarbonation] = new PropertySchema( kpropForcedCarbonation, tmpNames , kxmlPropForcedCarbonation, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolRecipePrimingSugarName;
   m_properties[kpropPrimingSugarName] = new PropertySchema( kpropPrimingSugarName, tmpNames , kxmlPropPrimingSugarName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolRecipeCarbonationTemp;
   m_properties[kpropCarbonationTemp] = new PropertySchema( kpropCarbonationTemp, tmpNames , kxmlPropCarbonationTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolRecipePrimingSugarEquiv;
   m_properties[kpropPrimingSugarEquiv] = new PropertySchema( kpropPrimingSugarEquiv, tmpNames , kxmlPropPrimingSugarEquiv, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeKegPrimingFactor;
   m_properties[kpropKegPrimingFactor] = new PropertySchema( kpropKegPrimingFactor, tmpNames , kxmlPropKegPrimingFactor, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeTasteNotes;
   m_properties[kpropTasteNotes] = new PropertySchema( kpropTasteNotes, tmpNames , kxmlPropTasteNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolRecipeTasteRating;
   m_properties[kpropTasteRating] = new PropertySchema( kpropTasteRating, tmpNames , kxmlPropTasteRating, QString("real"), QVariant(20.0));

   // enough properties, now some foreign keys
   tmpNames[Brewtarget::NODB] = kcolRecipeEquipmentId;
   m_foreignKeys[kpropEquipmentId] = new PropertySchema( kpropEquipmentId, tmpNames , Brewtarget::EQUIPTABLE);

   tmpNames[Brewtarget::NODB] = kcolRecipeMashId;
   m_foreignKeys[kpropMashId] = new PropertySchema( kpropMashId, tmpNames , Brewtarget::MASHTABLE);

   tmpNames[Brewtarget::NODB] = kcolRecipeStyleId;
   m_foreignKeys[kpropStyleId] = new PropertySchema( kpropStyleId, tmpNames , Brewtarget::STYLETABLE);

   tmpNames[Brewtarget::NODB] = kcolRecipeAncestorId;
   m_foreignKeys[kpropAncestorId] = new PropertySchema( kpropAncestorId, tmpNames , Brewtarget::RECTABLE);
}

void TableSchema::defineYeastTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // These are defined in the global file.
   tmpNames[Brewtarget::NODB] = kcolName;
   m_properties[kpropName] = new PropertySchema( kpropName, tmpNames , kxmlPropName, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolNotes;
   m_properties[kpropNotes] = new PropertySchema( kpropNotes, tmpNames , kxmlPropNotes, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolYeastType;
   m_properties[kpropType] = new PropertySchema( kpropType, tmpNames , QString(), QString("text"), QObject::tr("Ale"));

   tmpNames[Brewtarget::NODB] = kcolYeastForm;
   m_properties[kpropForm] = new PropertySchema( kpropForm, tmpNames , QString(), QString("text"), QObject::tr("Liquid"));

   tmpNames[Brewtarget::NODB] = kcolYeastAmount;
   m_properties[kpropAmount] = new PropertySchema( kpropAmount, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolYeastAmountIsWeight;
   m_properties[kpropAmountIsWeight] = new PropertySchema( kpropAmountIsWeight, tmpNames , kxmlPropAmountIsWeight, QString("boolean"), QVariant(false));

   tmpNames[Brewtarget::NODB] = kcolYeastLab;
   m_properties[kpropLab] = new PropertySchema( kpropLab, tmpNames , kxmlPropLab, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolYeastProductID;
   m_properties[kpropProductID] = new PropertySchema( kpropProductID, tmpNames , kxmlPropProductID, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolYeastMinTemp;
   m_properties[kpropMinTemp] = new PropertySchema( kpropMinTemp, tmpNames , kxmlPropMinTemp, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolYeastMaxTemp;
   m_properties[kpropMaxTemp] = new PropertySchema( kpropMaxTemp, tmpNames , kxmlPropMaxTemp, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolYeastFlocculation;
   m_properties[kpropFlocculation] = new PropertySchema( kpropFlocculation, tmpNames , QString(), QString("text"), QObject::tr("Medium"));

   tmpNames[Brewtarget::NODB] = kcolYeastAttenuation;
   m_properties[kpropAttenuation] = new PropertySchema( kpropAttenuationPct, tmpNames , kxmlPropAttenuation, QString("real"), QVariant(75.0));

   tmpNames[Brewtarget::NODB] = kcolYeastBestFor;
   m_properties[kpropBestFor] = new PropertySchema( kpropBestFor, tmpNames , kxmlPropBestFor, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolYeastTimesCultured;
   m_properties[kpropTimesCultured] = new PropertySchema( kpropTimesCultured, tmpNames , kxmlPropTimesCultured, QString("int"), QVariant(0));

   tmpNames[Brewtarget::NODB] = kcolYeastMaxReuse;
   m_properties[kpropMaxReuse] = new PropertySchema( kpropMaxReuse, tmpNames , kxmlPropMaxReuse, QString("int"), QVariant(10));

   tmpNames[Brewtarget::NODB] = kcolYeastAddToSecondary;
   m_properties[kpropAddToSecondary] = new PropertySchema( kpropAddToSecondary, tmpNames , kxmlPropAddToSecondary, QString("boolean"), QVariant(false));

}

void TableSchema::defineBrewnoteTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolBrewnoteBrewDate;
   m_properties[kpropBrewDate] = new PropertySchema( kpropBrewDate, tmpNames , kxmlPropBrewDate, QString("timestamp"), QDateTime());

   tmpNames[Brewtarget::NODB] = kcolBrewnoteFermentDate;
   m_properties[kpropFermentDate] = new PropertySchema( kpropFermentDate, tmpNames , kxmlPropFermentDate, QString("timestamp"), QDateTime());

   tmpNames[Brewtarget::NODB] = kcolBrewnoteSpecificGravity;
   m_properties[kpropSpecificGravity] = new PropertySchema( kpropSpecificGravity, tmpNames , kxmlPropSpecificGravity, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteVolumeIntoBoil;
   m_properties[kpropVolumeIntoBoil] = new PropertySchema( kpropVolumeIntoBoil, tmpNames , kxmlPropVolumeIntoBoil, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteStrikeTemp;
   m_properties[kpropStrikeTemp] = new PropertySchema( kpropStrikeTemp, tmpNames , kxmlPropStrikeTemp, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteMashFinalTemp;
   m_properties[kpropMashFinalTemp] = new PropertySchema( kpropMashFinalTemp, tmpNames , kxmlPropMashFinalTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteOriginalGravity;
   m_properties[kpropOriginalGravity] = new PropertySchema( kpropOriginalGravity, tmpNames , kxmlPropOriginalGravity, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnotePostBoilVolume;
   m_properties[kpropPostBoilVolume] = new PropertySchema( kpropPostBoilVolume, tmpNames , kxmlPropPostBoilVolume, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteVolumeIntoFermenter;
   m_properties[kpropVolumeIntoFermenter] = new PropertySchema( kpropVolumeIntoFermenter, tmpNames , kxmlPropVolumeIntoFermenter, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnotePitchTemp;
   m_properties[kpropPitchTemp] = new PropertySchema( kpropPitchTemp, tmpNames , kxmlPropPitchTemp, QString("real"), QVariant(20.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteFinalGravity;
   m_properties[kpropFinalGravity] = new PropertySchema( kpropFinalGravity, tmpNames , kxmlPropFinalGravity, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteEfficiencyIntoBoil;
   m_properties[kpropEfficiencyIntoBoil] = new PropertySchema( kpropEfficiencyIntoBoil, tmpNames , kxmlPropEfficiencyIntoBoil, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteABV;
   m_properties[kpropABV] = new PropertySchema( kpropABV, tmpNames , kxmlPropABV, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedOG;
   m_properties[kpropProjectedOG] = new PropertySchema( kpropProjectedOG, tmpNames , kxmlPropProjectedOG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteBrewhouseEfficiency;
   m_properties[kpropBrewhouseEfficiency] = new PropertySchema( kpropBrewhouseEfficiency, tmpNames , kxmlPropBrewhouseEfficiency, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedBoilGravity;
   m_properties[kpropProjectedBoilGravity] = new PropertySchema( kpropProjectedBoilGravity, tmpNames , kxmlPropProjectedBoilGravity, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedStrikeTemp;
   m_properties[kpropProjectedStrikeTemp] = new PropertySchema( kpropProjectedStrikeTemp, tmpNames , kxmlPropProjectedStrikeTemp, QString("real"), QVariant(70.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedMashFinishTemp;
   m_properties[kpropProjectedMashFinishTemp] = new PropertySchema( kpropProjectedMashFinishTemp, tmpNames , kxmlPropProjectedMashFinishTemp, QString("real"), QVariant(67.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedVolumeIntoBoil;
   m_properties[kpropProjectedVolumeIntoBoil] = new PropertySchema( kpropProjectedVolumeIntoBoil, tmpNames , kxmlPropProjectedVolumeIntoBoil, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedOG;
   m_properties[kpropProjectedOG] = new PropertySchema( kpropProjectedOG, tmpNames , kxmlPropProjectedOG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedVolumeIntoFermenter;
   m_properties[kpropProjectedVolumeIntoFermenter] = new PropertySchema( kpropProjectedVolumeIntoFermenter, tmpNames , kxmlPropProjectedVolumeIntoFermenter, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedFG;
   m_properties[kpropProjectedFG] = new PropertySchema( kpropProjectedFG, tmpNames , kxmlPropProjectedFG, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedEfficiency;
   m_properties[kpropProjectedEfficiency] = new PropertySchema( kpropProjectedEfficiency, tmpNames , kxmlPropProjectedEfficiency, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedABV;
   m_properties[kpropProjectedABV] = new PropertySchema( kpropProjectedABV, tmpNames , kxmlPropProjectedABV, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedAttenuation;
   m_properties[kpropProjectedAttenuation] = new PropertySchema( kpropProjectedAttenuation, tmpNames , kxmlPropProjectedAttenuation, QString("real"), QVariant(75.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedPoints;
   m_properties[kpropProjectedPoints] = new PropertySchema( kpropProjectedPoints, tmpNames , kxmlPropProjectedPoints, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteProjectedFermentationPoints;
   m_properties[kpropProjectedFermentationPoints] = new PropertySchema( kpropProjectedFermentationPoints, tmpNames , kxmlPropProjectedFermentationPoints, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteBoilOff;
   m_properties[kpropBoilOff] = new PropertySchema( kpropBoilOff, tmpNames , kxmlPropBoilOff, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteFinalVolume;
   m_properties[kpropFinalVolume] = new PropertySchema( kpropFinalVolume, tmpNames , kxmlPropFinalVolume, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolBrewnoteAttenuation;
   m_properties[kpropAttenuation] = new PropertySchema( kpropAttenuation, tmpNames , kxmlPropAttenuation, QString("real"), QVariant(1.0));

   tmpNames[Brewtarget::NODB] = kcolRecipeId;
   m_foreignKeys[kpropRecipeId] = new PropertySchema( kpropRecipeId, tmpNames , Brewtarget::RECTABLE);

}

void TableSchema::defineChildTable(Brewtarget::DBTable table)
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolParentId;
   m_foreignKeys[kpropParentId] = new PropertySchema( kpropParentId, tmpNames , table);

   tmpNames[Brewtarget::NODB] = kcolChildId;
   m_foreignKeys[kpropChildId] = new PropertySchema( kpropChildId, tmpNames , table);

}

void TableSchema::defineInRecipeTable(QString childIdx, Brewtarget::DBTable table)
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolRecipeId;
   m_foreignKeys[kpropRecipeId] = new PropertySchema( kpropRecipeId, tmpNames , Brewtarget::RECTABLE);

   // this sort of breaks the rule -- I prefer to have a kcol and a kprop. But
   // these aren't in any object, so they no property and I can reuse
   // the kcol value
   tmpNames[Brewtarget::NODB] = childIdx;
   m_foreignKeys[childIdx] = new PropertySchema( childIdx, tmpNames , table);

}

void TableSchema::defineBtTable(QString childIdx, Brewtarget::DBTable table)
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   // What good is a rule followed to well?
   tmpNames[Brewtarget::NODB] = childIdx;
   m_foreignKeys[childIdx] = new PropertySchema( childIdx, tmpNames , table);

}

void TableSchema::defineFermInventoryTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolAmount;
   m_properties[kpropAmount] = new PropertySchema( kpropAmount, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolFermentableId;
   m_foreignKeys[kcolFermentableId] = new PropertySchema( kcolFermentableId, tmpNames , Brewtarget::FERMTABLE);

}

void TableSchema::defineHopInventoryTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolAmount;
   m_properties[kpropAmount] = new PropertySchema( kpropAmount, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolHopId;
   m_foreignKeys[kcolHopId] = new PropertySchema( kcolHopId, tmpNames , Brewtarget::HOPTABLE);

}

void TableSchema::defineMiscInventoryTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolAmount;
   m_properties[kpropAmount] = new PropertySchema( kpropAmount, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolMiscId;
   m_foreignKeys[kcolMiscId] = new PropertySchema( kcolMiscId, tmpNames , Brewtarget::MISCTABLE);

}

void TableSchema::defineYeastInventoryTable()
{
   QHash<Brewtarget::DBTypes,QString> tmpNames;

   tmpNames[Brewtarget::NODB] = kcolYeastQuanta;
   m_properties[kpropQuanta] = new PropertySchema( kpropQuanta, tmpNames , kxmlPropAmount, QString("real"), QVariant(0.0));

   tmpNames[Brewtarget::NODB] = kcolYeastId;
   m_foreignKeys[kcolYeastId] = new PropertySchema( kcolYeastId, tmpNames , Brewtarget::YEASTTABLE);

}
