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
   tmpNames[Brewtarget::NODB] = kcolType;
   tmp[kpropType] = new PropertySchema( kpropType, tmpNames , QString(""), QString("text"), QString("Ale"));
  
   tmpNames[Brewtarget::NODB] = kcolCategoryNumber;
   tmp[kpropCategoryNumber] = new PropertySchema( kpropCategoryNumber, tmpNames, kxmlPropCategoryNumber, QString("text"), QString(""));

   tmpNames[Brewtarget::NODB] = kcolStyleLetter;
   tmp[kpropStyleLetter] = new PropertySchema( kpropStyleLetter, tmpNames, kxmlPropStyleLetter, QString("text"), QString("") );

   tmpNames[Brewtarget::NODB] = kcolStyleGuide;
   tmp[kpropStyleGuide] = new PropertySchema( kpropStyleGuide, tmpNames, kxmlPropStyleGuide, QString("text"), QString("") );

   tmpNames[Brewtarget::NODB] = kcolOGMin;
   tmp[kpropOGMin] = new PropertySchema( kpropOGMin, tmpNames, kxmlPropOGMin);

   tmpNames[Brewtarget::NODB] = kcolOGMax;
   tmp[kpropOGMax] = new PropertySchema( kpropOGMax, tmpNames, kxmlPropOGMax);

   tmpNames[Brewtarget::NODB] = kcolFGMin;
   tmp[kpropFGMin] = new PropertySchema( kpropFGMin, tmpNames, kxmlPropFGMin);

   tmpNames[Brewtarget::NODB] = kcolFGMax;
   tmp[kpropFGMax] = new PropertySchema( kpropFGMax, tmpNames, kxmlPropFGMax);

   tmpNames[Brewtarget::NODB] = kcolIBUMin;
   tmp[kpropIBUMin] = new PropertySchema( kpropIBUMin, tmpNames, kxmlPropIBUMin);

   tmpNames[Brewtarget::NODB] = kcolIBUMax;
   tmp[kpropIBUMax] = new PropertySchema( kpropIBUMax, tmpNames, kxmlPropIBUMax);

   tmpNames[Brewtarget::NODB] = kcolColorMin;
   tmp[kpropColorMin] = new PropertySchema( kpropColorMin,tmpNames, kxmlPropColorMin);

   tmpNames[Brewtarget::NODB] = kcolColorMax;
   tmp[kpropColorMax] = new PropertySchema( kpropColorMax,tmpNames, kxmlPropColorMax);

   tmpNames[Brewtarget::NODB] = kcolABVMin;
   tmp[kpropABVMin] = new PropertySchema( kpropABVMin, tmpNames, kxmlPropABVMin);

   tmpNames[Brewtarget::NODB] = kcolABVMax;
   tmp[kpropABVMax] = new PropertySchema( kpropABVMax, tmpNames, kxmlPropABVMax);

   tmpNames[Brewtarget::NODB] = kcolCarbMin;
   tmp[kpropCarbMin] = new PropertySchema( kpropCarbMin, tmpNames, kxmlPropCarbMin);

   tmpNames[Brewtarget::NODB] = kcolCarbMax;
   tmp[kpropCarbMax] = new PropertySchema( kpropCarbMax, tmpNames, kxmlPropCarbMax);

   tmpNames[Brewtarget::NODB] = kcolNotes;
   tmp[kpropNotes] = new PropertySchema( kpropNotes, tmpNames, kxmlPropNotes);

   tmpNames[Brewtarget::NODB] = kcolProfile;
   tmp[kpropProfile] = new PropertySchema( kpropProfile, tmpNames, kxmlPropProfile);

   tmpNames[Brewtarget::NODB] = kcolIngredients;
   tmp[kpropIngredients] = new PropertySchema( kpropIngredients, tmpNames, kxmlPropIngredients);

   tmpNames[Brewtarget::NODB] = kcolExamples;
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
