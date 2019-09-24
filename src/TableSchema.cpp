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
#include "InstructionSchema.h"
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
      m_dbTable(table),
      m_childTable(Brewtarget::NOTABLE),
      m_inRecTable(Brewtarget::NOTABLE),
      m_invTable(Brewtarget::NOTABLE)
{
    // for this bit of ugly, I gain a lot of utility.
    defineTable();
}

const QString TableSchema::tableName() const { return m_tableName; }
const QString TableSchema::className() const { return m_className; }
Brewtarget::DBTable TableSchema::dbTable() const { return m_dbTable; }
Brewtarget::DBTable TableSchema::childTable() const  { return m_childTable; }
Brewtarget::DBTable TableSchema::inRecTable() const { return m_inRecTable; }
Brewtarget::DBTable TableSchema::invTable() const { return m_invTable; }

const QMap<QString,PropertySchema*> TableSchema::properties() const { return m_properties; }
const QMap<QString,PropertySchema*> TableSchema::foreignKeys() const { return m_foreignKeys; }

const QStringList TableSchema::allPropertyNames(Brewtarget::DBTypes type) const
{
    QMapIterator<QString,PropertySchema*> i(m_properties);
    QStringList retval;
    while ( i.hasNext() ) {
        i.next();
        retval.append( i.value()->colName(type));
    }
    return retval;
}

const QStringList TableSchema::allForeignKeyNames(Brewtarget::DBTypes type) const
{
    QMapIterator<QString,PropertySchema*> i(m_foreignKeys);
    QStringList retval;
    while ( i.hasNext() ) {
        i.next();
        retval.append( i.value()->colName(type));
    }
    return retval;
}

const QStringList TableSchema::allColumnNames(Brewtarget::DBTypes type) const
{
   QStringList tmp;
   QMapIterator<QString,PropertySchema*> i(m_properties);

   while ( i.hasNext() ) {
      i.next();
      tmp.append(i.value()->colName(type));
   }
   return tmp;
}

const QStringList TableSchema::allForeignKeyColumnNames(Brewtarget::DBTypes type) const
{
   QStringList tmp;

   QMapIterator<QString,PropertySchema*> i(m_properties);

   while ( i.hasNext() ) {
      i.next();
      tmp.append(i.value()->colName(type));
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

const QString TableSchema::propertyToXml(QString prop, Brewtarget::DBTypes type) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->xmlName(type);
   }
   else {
      return QString();
   }
}

const QString TableSchema::xmlToProperty(QString xmlName, Brewtarget::DBTypes type) const
{
   QString retval;

   QMapIterator<QString,PropertySchema*> i(m_properties);

   while ( i.hasNext() ) {
      i.next();
      if ( i.value()->xmlName() == xmlName ) {
         retval = i.value()->propName(type);
         break;
      }
   }
   return retval;
}

const QString TableSchema::propertyColumnType(QString prop, Brewtarget::DBTypes type) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->colType(type);
   }
   else {
      return QString();
   }
}

const QVariant TableSchema::propertyColumnDefault(QString prop, Brewtarget::DBTypes type) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->defaultValue(type);
   }
   else {
      return QString();
   }
}

int TableSchema::propertyColumnSize(QString prop, Brewtarget::DBTypes type) const
{
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->colSize(type);
   }
   else {
      return 0;
   }
}

bool TableSchema::isInventoryTable() { return m_type == INV; }
bool TableSchema::isBaseTable()      { return m_type == BASE; }
bool TableSchema::isChildTable()     { return m_type == CHILD; }
bool TableSchema::isInRecTable()     { return m_type == INREC; }

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
      case Brewtarget::INSTRUCTIONTABLE:
         defineInstructionTable();
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
   m_type = BASE;
   m_className = QString("Style");
   m_childTable = Brewtarget::STYLECHILDTABLE;

   m_properties[kpropName]      = new PropertySchema( kpropName,     kpropName,         kxmlPropName,     QString("text"), QString("''"), QString("not null"));
   m_properties[kpropType]      = new PropertySchema( kpropType,     kcolStyleType,     QString(""),      QString("text"), QString("'Ale'"));
   m_properties[kpropCat]       = new PropertySchema( kpropCat,      kcolStyleCat,      kxmlPropCat,      QString("text"), QString("''"));
   m_properties[kpropLetter]    = new PropertySchema( kpropLetter,   kcolStyleLetter,   kxmlPropLetter,   QString("text"), QString("''"));
   m_properties[kpropGuide]     = new PropertySchema( kpropGuide,    kcolStyleGuide,    kxmlPropGuide,    QString("text"), QString("''"));
   m_properties[kpropOGMin]     = new PropertySchema( kpropOGMin,    kcolStyleOGMin,    kxmlPropOGMin,    QString("real"), QVariant(0.0));
   m_properties[kpropOGMax]     = new PropertySchema( kpropOGMax,    kcolStyleOGMax,    kxmlPropOGMax,    QString("real"), QVariant(0.0));
   m_properties[kpropFGMin]     = new PropertySchema( kpropFGMin,    kcolStyleFGMin,    kxmlPropFGMin,    QString("real"), QVariant(0.0));
   m_properties[kpropFGMax]     = new PropertySchema( kpropFGMax,    kcolStyleFGMax,    kxmlPropFGMax,    QString("real"), QVariant(0.0));
   m_properties[kpropIBUMin]    = new PropertySchema( kpropIBUMin,   kcolStyleIBUMin,   kxmlPropIBUMin,   QString("real"), QVariant(0.0));
   m_properties[kpropIBUMax]    = new PropertySchema( kpropIBUMax,   kcolStyleIBUMax,   kxmlPropIBUMax,   QString("real"), QVariant(0.0));
   m_properties[kpropColorMin]  = new PropertySchema( kpropColorMin, kcolStyleColorMin, kxmlPropColorMin, QString("real"), QVariant(0.0));
   m_properties[kpropColorMax]  = new PropertySchema( kpropColorMax, kcolStyleColorMax, kxmlPropColorMax, QString("real"), QVariant(0.0));
   m_properties[kpropABVMin]    = new PropertySchema( kpropABVMin,   kcolStyleABVMin,   kxmlPropABVMin,   QString("real"), QVariant(0.0));
   m_properties[kpropABVMax]    = new PropertySchema( kpropABVMax,   kcolStyleABVMax,   kxmlPropABVMax,   QString("real"), QVariant(0.0));
   m_properties[kpropCarbMin]   = new PropertySchema( kpropCarbMin,  kcolStyleCarbMin,  kxmlPropCarbMin,  QString("real"), QVariant(0.0));
   m_properties[kpropCarbMax]   = new PropertySchema( kpropCarbMax,  kcolStyleCarbMax,  kxmlPropCarbMax,  QString("real"), QVariant(0.0));
   m_properties[kpropNotes]     = new PropertySchema( kpropNotes,    kcolNotes,         kxmlPropNotes,    QString("text"), QString("''"));
   m_properties[kpropProfile]   = new PropertySchema( kpropProfile,  kcolStyleProfile,  kxmlPropProfile,  QString("text"), QString("''"));
   m_properties[kpropIngreds]   = new PropertySchema( kpropIngreds,  kcolStyleIngreds,  kxmlPropIngreds,  QString("text"), QString("''"));
   m_properties[kpropExamples]  = new PropertySchema( kpropExamples, kcolStyleExamples, kxmlPropExamples, QString("text"), QString("''"));

   // not sure about these, but I think I'm gonna need them anyway
   m_properties[kpropDisplay]   = new PropertySchema(kpropDisplay,   kcolDisplay,       QString(),        QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]   = new PropertySchema(kpropDeleted,   kcolDeleted,       QString(),        QString("boolean"), QVariant(false));
   m_properties[kpropFolder]    = new PropertySchema(kpropFolder,    kpropFolder,       QString(),        QString("text"), QString("''"));
}

void TableSchema::defineEquipmentTable()
{
   m_type = BASE;
   m_className = QString("Equipment");
   m_childTable = Brewtarget::EQUIPCHILDTABLE;

   m_properties[kpropName]          = new PropertySchema( kpropName,          kpropName,         kxmlPropName,            QString("text"), QString("''"), QString("not null"));
   m_properties[kpropBoilSize]      = new PropertySchema( kpropBoilSize,      kcolEquipBoilSize,      kxmlPropBoilSize,        QString("real"), QVariant(0.0));
   m_properties[kpropBatchSize]     = new PropertySchema( kpropBatchSize,     kcolEquipBatchSize,     kxmlPropBatchSize,       QString("real"), QVariant(0.0));
   m_properties[kpropTunVolume]     = new PropertySchema( kpropTunVolume,     kcolEquipTunVolume,     kxmlPropTunVolume,       QString("real"), QVariant(0.0));
   m_properties[kpropTunWeight]     = new PropertySchema( kpropTunWeight,     kcolEquipTunWeight,     kxmlPropTunWeight,       QString("real"), QVariant(0.0));
   m_properties[kpropTunSpecHeat]   = new PropertySchema( kpropTunSpecHeat,   kcolEquipTunSpecHeat,   kxmlPropTunSpecHeat,     QString("real"), QVariant(0.0));
   m_properties[kpropTopUpWater]    = new PropertySchema( kpropTopUpWater,    kcolEquipTopUpWater,    kxmlPropTopUpWater,      QString("real"), QVariant(0.0));
   m_properties[kpropTrubChillLoss] = new PropertySchema( kpropTrubChillLoss, kcolEquipTrubChillLoss, kxmlPropTrubChillLoss,   QString("real"), QVariant(0.0));
   m_properties[kpropEvapRate]      = new PropertySchema( kpropEvapRate,      kcolEquipEvapRate,      kxmlPropEvapRate,        QString("real"), QVariant(0.0));
   m_properties[kpropBoilTime]      = new PropertySchema( kpropBoilTime,      kcolEquipBoilTime,      kxmlPropBoilTime,        QString("real"), QVariant(0.0));
   m_properties[kpropCalcBoilVol]   = new PropertySchema( kpropCalcBoilVol,   kcolEquipCalcBoilVol,   kxmlPropCalcBoilVol,     QString("boolean"), QVariant(false));
   m_properties[kpropLauterSpace]   = new PropertySchema( kpropLauterSpace,   kcolEquipLauterSpace,   kxmlPropLauterSpace,     QString("real"), QVariant(0.0));
   m_properties[kpropTopUpKettle]   = new PropertySchema( kpropTopUpKettle,   kcolEquipTopUpKettle,   kxmlPropTopUpKettle,     QString("real"), QVariant(0.0));
   m_properties[kpropHopUtil]       = new PropertySchema( kpropHopUtil,       kcolEquipHopUtil,       kxmlPropHopUtil,         QString("real"), QVariant(0.0));
   m_properties[kpropNotes]         = new PropertySchema( kpropNotes,         kcolNotes,              kxmlPropNotes,           QString("text"), QString("''"));
   m_properties[kpropRealEvapRate]  = new PropertySchema( kpropRealEvapRate,  kcolEquipRealEvapRate,  kxmlPropRealEvapRate,    QString("real"), QVariant(0.0));
   m_properties[kpropBoilingPoint]  = new PropertySchema( kpropBoilingPoint,  kcolEquipBoilingPoint,  kxmlPropBoilingPoint,    QString("real"), QVariant(100.0));
   m_properties[kpropAbsorption]    = new PropertySchema( kpropAbsorption,    kcolEquipAbsorption,    kxmlPropGrainAbsorption, QString("real"), QVariant(1.085));

   m_properties[kpropDisplay]       = new PropertySchema( kpropDisplay,       kcolDisplay,       QString(),               QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]       = new PropertySchema( kpropDeleted,       kcolDeleted,       QString(),               QString("boolean"), QVariant(false));
   m_properties[kpropFolder]        = new PropertySchema( kpropFolder,        kpropFolder,       QString(),               QString("text"), QString("''"));

}

void TableSchema::defineFermentableTable()
{
   m_type = BASE;
   m_className = QString("Fermentable");
   m_childTable = Brewtarget::FERMCHILDTABLE;
   m_inRecTable = Brewtarget::FERMINRECTABLE;
   m_invTable   = Brewtarget::FERMINVTABLE;

   m_properties[kpropName]           = new PropertySchema( kpropName,           kcolName,               kxmlPropName,           QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]          = new PropertySchema( kpropNotes,          kcolNotes,              kxmlPropNotes,          QString("text"), QString("''"));
   m_properties[kpropType]           = new PropertySchema( kpropType,           kcolFermType,           QString(""),            QString("text"), QString("'Grain'"));
   m_properties[kpropAmountKg]       = new PropertySchema( kpropAmountKg,       kcolAmount,             kxmlPropAmount,         QString("real"), QVariant(0.0));
   m_properties[kpropYield]          = new PropertySchema( kpropYield,          kcolFermYield,          kxmlPropYield,          QString("real"), QVariant(0.0));
   m_properties[kpropColor]          = new PropertySchema( kpropColor,          kcolFermColor,          kxmlPropColor,          QString("real"), QVariant(0.0));
   m_properties[kpropAddAfterBoil]   = new PropertySchema( kpropAddAfterBoil,   kcolFermAddAfterBoil,   kxmlPropAddAfterBoil,   QString("boolean"), QVariant(false));
   m_properties[kpropOrigin]         = new PropertySchema( kpropOrigin,         kcolFermOrigin,         kxmlPropOrigin,         QString("text"), QString(""));
   m_properties[kpropSupplier]       = new PropertySchema( kpropSupplier,       kcolFermSupplier,       kxmlPropSupplier,       QString("text"), QString(""));
   m_properties[kpropCoarseFineDiff] = new PropertySchema( kpropCoarseFineDiff, kcolFermCoarseFineDiff, kxmlPropCoarseFineDiff, QString("real"), QVariant(0.0));
   m_properties[kpropMoisture]       = new PropertySchema( kpropMoisture,       kcolFermMoisture,       kxmlPropMoisture,       QString("real"), QVariant(0.0));
   m_properties[kpropDiastaticPower] = new PropertySchema( kpropDiastaticPower, kcolFermDiastaticPower, kxmlPropDiastaticPower, QString("real"), QVariant(0.0));
   m_properties[kpropProtein]        = new PropertySchema( kpropProtein,        kcolFermProtein,        kxmlPropProtein,        QString("real"), QVariant(0.0));
   m_properties[kpropMaxInBatch]     = new PropertySchema( kpropMaxInBatch,     kcolFermMaxInBatch,     kxmlPropMaxInBatch,     QString("real"), QVariant(100.0));
   m_properties[kpropRecommendMash]  = new PropertySchema( kpropRecommendMash,  kcolFermRecommendMash,  kxmlPropRecommendMash,  QString("boolean"), QVariant(false));
   m_properties[kpropIsMashed]       = new PropertySchema( kpropIsMashed,       kcolFermIsMashed,       kxmlPropIsMashed,       QString("boolean"), QVariant(false));
   m_properties[kpropIBUGalPerLb]    = new PropertySchema( kpropIBUGalPerLb,    kcolFermIBUGalPerLb,    kxmlPropIBUGalPerLb,    QString("real"), QVariant(0.0));

   m_properties[kpropDisplay]        = new PropertySchema( kpropDisplay,        kcolDisplay,            QString(),              QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]        = new PropertySchema( kpropDeleted,        kcolDeleted,            QString(),              QString("boolean"), QVariant(false));
   m_properties[kpropFolder]         = new PropertySchema( kpropFolder,         kpropFolder,            QString(),              QString("text"), QString("''"));
}

void TableSchema::defineHopTable()
{
   m_type = BASE;
   m_className = QString("Hop");
   m_childTable = Brewtarget::HOPCHILDTABLE;
   m_inRecTable = Brewtarget::HOPINRECTABLE;
   m_invTable   = Brewtarget::HOPINVTABLE;

   // These are defined in the global file.
   m_properties[kpropName]          = new PropertySchema( kpropName,          kcolName,             kxmlPropName,          QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]         = new PropertySchema( kpropNotes,         kcolNotes,            kxmlPropNotes,         QString("text"), QString("''"));
   m_properties[kpropAmountKg]      = new PropertySchema( kpropAmountKg,      kcolAmount,           kxmlPropAmount,        QString("real"), QVariant(0.0));
   m_properties[kpropUse]           = new PropertySchema( kpropUse,           kcolUse,              QString(""),           QString("text"), QString("'Boil'"));
   m_properties[kpropTime]          = new PropertySchema( kpropTime,          kcolTime,             kxmlPropTime,          QString("real"), QVariant(0.0));
   m_properties[kpropOrigin]        = new PropertySchema( kpropOrigin,        kcolOrigin,           QString(""),           QString("text"), QString("''"));
   m_properties[kpropSubstitutes]   = new PropertySchema( kpropSubstitutes,   kcolSubstitutes,      QString(""),           QString("text"), QString("''"));
   m_properties[kpropAlpha]         = new PropertySchema( kpropAlpha,         kcolHopAlpha,         kxmlPropAlpha,         QString("real"), QVariant(0.0));
   m_properties[kpropType]          = new PropertySchema( kpropType,          kcolHopType,          QString(""),           QString("text"), QString("'Boil'"));
   m_properties[kpropForm]          = new PropertySchema( kpropForm,          kcolHopForm,          QString(""),           QString("text"), QString("'Pellet'"));
   m_properties[kpropBeta]          = new PropertySchema( kpropBeta,          kcolHopBeta,          kxmlPropBeta,          QString("real"), QVariant(0.0));
   m_properties[kpropHSI]           = new PropertySchema( kpropHSI,           kcolHopHSI,           kxmlPropHSI,           QString("real"), QVariant(0.0));
   m_properties[kpropHumulene]      = new PropertySchema( kpropHumulene,      kcolHopHumulene,      kxmlPropHumulene,      QString("real"), QVariant(0.0));
   m_properties[kpropCaryophyllene] = new PropertySchema( kpropCaryophyllene, kcolHopCaryophyllene, kxmlPropCaryophyllene, QString("real"), QVariant(0.0));
   m_properties[kpropCohumulone]    = new PropertySchema( kpropCohumulone,    kcolHopCohumulone,    kxmlPropCohumulone,    QString("real"), QVariant(0.0));
   m_properties[kpropMyrcene]       = new PropertySchema( kpropMyrcene,       kcolHopMyrcene,       kxmlPropMyrcene,       QString("real"), QVariant(0.0));

   m_properties[kpropDisplay]       = new PropertySchema( kpropDisplay,       kcolDisplay,          QString(),             QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]       = new PropertySchema( kpropDeleted,       kcolDeleted,          QString(),             QString("boolean"), QVariant(false));
   m_properties[kpropFolder]        = new PropertySchema( kpropFolder,        kpropFolder,          QString(),             QString("text"), QString("''"));
}

void TableSchema::defineInstructionTable()
{
   m_type = BASE;
   m_className = QString("Instruction");
   m_inRecTable = Brewtarget::INSTINRECTABLE;

   // These are defined in the global file.
   m_properties[kpropName]          = new PropertySchema( kpropName,          kcolName,                  kxmlPropName,       QString("text"), QString("''"), QString("not null"));
   m_properties[kpropDirections]    = new PropertySchema( kpropDirections,    kcolInstructionDirections, kxmlPropDirections, QString("text"), QString("''"));
   m_properties[kpropHasTimer]      = new PropertySchema( kpropHasTimer,      kcolInstructionHasTimer,   kxmlPropHasTimer,   QString("boolean"), QVariant(false));
   m_properties[kpropTimerValue]    = new PropertySchema( kpropTimerValue,    kcolInstructionTimerValue, kxmlPropTimerValue, QString("text"), QVariant("'00:00:00'"));
   m_properties[kpropCompleted]     = new PropertySchema( kpropCompleted,     kcolInstructionCompleted,  kxmlPropCompleted,  QString("boolean"), QVariant(false));
   m_properties[kpropInterval]      = new PropertySchema( kpropInterval,      kcolInstructionInterval,   kxmlPropInterval,   QString("real"), QVariant(0.0));

   m_properties[kpropDisplay]       = new PropertySchema( kpropDisplay,       kcolDisplay,               QString(),          QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]       = new PropertySchema( kpropDeleted,       kcolDeleted,               QString(),          QString("boolean"), QVariant(false));
}

void TableSchema::defineMashTable()
{

   m_type = BASE;
   m_className = QString("Mash");

   // These are defined in the global file.
   m_properties[kpropName]        = new PropertySchema( kpropName,        kcolName,            kxmlPropName,        QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]       = new PropertySchema( kpropNotes,       kcolNotes,           kxmlPropNotes,       QString("text"), QString("''"));
   m_properties[kpropGrainTemp]   = new PropertySchema( kpropGrainTemp,   kcolMashGrainTemp,   kxmlPropGrainTemp,   QString("real"), QVariant(0.0));
   m_properties[kpropTunTemp]     = new PropertySchema( kpropTunTemp,     kcolMashTunTemp,     kxmlPropTunTemp,     QString("real"), QVariant(20.0));
   m_properties[kpropSpargeTemp]  = new PropertySchema( kpropSpargeTemp,  kcolMashSpargeTemp,  kxmlPropSpargeTemp,  QString("real"), QVariant(74.0));
   m_properties[kpropPH]          = new PropertySchema( kpropPH,          kcolMashPH,          kxmlPropPH,          QString("real"), QVariant(7.0));
   m_properties[kpropTunWeight]   = new PropertySchema( kpropTunWeight,   kcolMashTunWeight,   kxmlPropTunWeight,   QString("real"), QVariant(0.0));
   m_properties[kpropTunSpecHeat] = new PropertySchema( kpropTunSpecHeat, kcolMashTunSpecHeat, kxmlPropTunSpecHeat, QString("real"), QVariant(0.0));
   m_properties[kpropEquipAdjust] = new PropertySchema( kpropEquipAdjust, kcolMashEquipAdjust, kxmlPropEquipAdjust, QString("boolean"), QVariant(true));

   m_properties[kpropDisplay]     = new PropertySchema( kpropDisplay,     kcolDisplay,         QString(),           QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]     = new PropertySchema( kpropDeleted,     kcolDeleted,         QString(),           QString("boolean"), QVariant(false));
   m_properties[kpropFolder]      = new PropertySchema( kpropFolder,      kpropFolder,         QString(),           QString("text"), QString("''"));
}

void TableSchema::defineMashstepTable()
{

   m_type = BASE;
   m_className = QString("MashStep");

   m_properties[kpropName]       = new PropertySchema( kpropName,       kcolName,               kxmlPropName,       QString("text"), QString("''"),QString("not null"));
   m_properties[kpropType]       = new PropertySchema( kpropType,       kcolMashstepType,       QString(""),        QString("text"), QString("'Infusion'"));
   m_properties[kpropInfuseAmt]  = new PropertySchema( kpropInfuseAmt,  kcolMashstepInfuseAmt,  kxmlPropInfuseAmt,  QString("real"), QVariant(0.0));
   m_properties[kpropStepTemp]   = new PropertySchema( kpropStepTemp,   kcolMashstepStepTemp,   kxmlPropStepTemp,   QString("real"), QVariant(67.0));
   m_properties[kpropStepTime]   = new PropertySchema( kpropStepTime,   kcolMashstepStepTime,   kxmlPropStepTime,   QString("real"), QVariant(0.0));
   m_properties[kpropRampTime]   = new PropertySchema( kpropRampTime,   kcolMashstepRampTime,   kxmlPropRampTime,   QString("real"), QVariant(0.0));
   m_properties[kpropEndTemp]    = new PropertySchema( kpropEndTemp,    kcolMashstepEndTemp,    kxmlPropEndTemp,    QString("real"), QVariant(67.0));
   m_properties[kpropInfuseTemp] = new PropertySchema( kpropInfuseTemp, kcolMashstepInfuseTemp, kxmlPropInfuseTemp, QString("real"), QVariant(67.0));
   m_properties[kpropDecoctAmt]  = new PropertySchema( kpropDecoctAmt,  kcolMashstepDecoctAmt,  kxmlPropDecoctAmt,  QString("real"), QVariant(67.0));

   m_properties[kpropDisplay]    = new PropertySchema( kpropDisplay,    kcolDisplay,            QString(),          QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]    = new PropertySchema( kpropDeleted,    kcolDeleted,            QString(),          QString("boolean"), QVariant(false));
   m_properties[kpropFolder]     = new PropertySchema( kpropFolder,     kpropFolder,            QString(),          QString("text"), QString("''"));

   m_foreignKeys[kpropMashId]    = new PropertySchema( kpropMashId,     kcolRecipeMashId,       Brewtarget::MASHTABLE);

}

void TableSchema::defineMiscTable()
{

   m_type = BASE;
   m_className = QString("Misc");
   m_childTable = Brewtarget::MISCCHILDTABLE;
   m_inRecTable = Brewtarget::MISCINRECTABLE;
   m_invTable   = Brewtarget::MISCINVTABLE;

   // These are defined in the global file.
   m_properties[kpropName]     = new PropertySchema( kpropName,     kcolName,         kxmlPropName,     QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]    = new PropertySchema( kpropNotes,    kcolNotes,        kxmlPropNotes,    QString("text"), QString("''"));
   m_properties[kpropAmountKg] = new PropertySchema( kpropAmountKg, kcolAmount,       kxmlPropAmount,   QString("real"), QVariant(0.0));
   m_properties[kpropUse]      = new PropertySchema( kpropUse,      kcolUse,          QString(""),      QString("text"), QString("'Boil'"));
   m_properties[kpropTime]     = new PropertySchema( kpropTime,     kcolTime,         kxmlPropTime,     QString("real"), QVariant(0.0));
   m_properties[kpropType]     = new PropertySchema( kpropType,     kcolMiscType,     QString(""),      QString("text"), QString("'Other'"));
   m_properties[kpropAmtIsWgt] = new PropertySchema( kpropAmtIsWgt, kcolMiscAmtIsWgt, kxmlPropAmtIsWgt, QString("boolean"), QVariant(true));
   m_properties[kpropUseFor]   = new PropertySchema( kpropUseFor,   kcolMiscUseFor,   kxmlPropUseFor,   QString("text"), QString("''"));

   m_properties[kpropDisplay]  = new PropertySchema( kpropDisplay,  kcolDisplay,      QString(),        QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]  = new PropertySchema( kpropDeleted,  kcolDeleted,      QString(),        QString("boolean"), QVariant(false));
   m_properties[kpropFolder]   = new PropertySchema( kpropFolder,   kpropFolder,      QString(),        QString("text"), QString("''"));
}

void TableSchema::defineRecipeTable()
{

   m_type = BASE;
   m_className = QString("Recipe");
   m_childTable = Brewtarget::RECIPECHILDTABLE;

   m_properties[kpropName]        = new PropertySchema( kpropName,        kcolName,               kxmlPropName,         QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]       = new PropertySchema( kpropNotes,       kcolNotes,              kxmlPropNotes,        QString("text"), QString("''"));
   m_properties[kpropType]        = new PropertySchema( kpropType,        kcolRecipeType,         kxmlPropType,         QString("text"), QString("'All Grain'"));
   m_properties[kpropBrewer]      = new PropertySchema( kpropBrewer,      kcolRecipeBrewer,       kxmlPropBrewer,       QString("text"), QString("''"));
   m_properties[kpropAsstBrewer]  = new PropertySchema( kpropAsstBrewer,  kcolRecipeAsstBrewer,   kxmlPropAsstBrewer,   QString("text"), QString("'Brewtarget'"));
   m_properties[kpropBatchSize]   = new PropertySchema( kpropBatchSize,   kcolRecipeBatchSize,    kxmlPropBatchSize,    QString("real"), QVariant(0.0));
   m_properties[kpropBoilSize]    = new PropertySchema( kpropBoilSize,    kcolRecipeBoilSize,     kxmlPropBoilSize,     QString("real"), QVariant(0.0));
   m_properties[kpropBoilTime]    = new PropertySchema( kpropBoilTime,    kcolRecipeBoilTime,     kxmlPropBoilTime,     QString("real"), QVariant(0.0));
   m_properties[kpropEffPct]      = new PropertySchema( kpropEffPct,      kcolRecipeEff,          kxmlPropEff,          QString("real"), QVariant(70.0));
   m_properties[kpropOG]          = new PropertySchema( kpropOG,          kcolRecipeOG,           kxmlPropOG,           QString("real"), QVariant(1.0));
   m_properties[kpropFG]          = new PropertySchema( kpropFG,          kcolRecipeFG,           kxmlPropFG,           QString("real"), QVariant(1.0));
   m_properties[kpropFermStages]  = new PropertySchema( kpropFermStages,  kcolRecipeFermStages,   kxmlPropFermStages,   QString("int"), QVariant(0));
   m_properties[kpropPrimAgeDays] = new PropertySchema( kpropPrimAgeDays, kcolRecipePrimAgeDays,  kxmlPropPrimAgeDays,  QString("real"), QVariant(0.0));
   m_properties[kpropPrimTemp]    = new PropertySchema( kpropPrimTemp,    kcolRecipePrimTemp,     kxmlPropPrimTemp,     QString("real"), QVariant(20.0));
   m_properties[kpropSecAgeDays]  = new PropertySchema( kpropSecAgeDays,  kcolRecipeSecAgeDays,   kxmlPropSecAgeDays,   QString("real"), QVariant(0.0));
   m_properties[kpropSecTemp]     = new PropertySchema( kpropSecTemp,     kcolRecipeSecTemp,      kxmlPropSecTemp,      QString("real"), QVariant(20.0));
   m_properties[kpropTertAgeDays] = new PropertySchema( kpropTertAgeDays, kcolRecipeTertAgeDays,  kxmlPropTertAgeDays,  QString("real"), QVariant(0.0));
   m_properties[kpropTertTemp]    = new PropertySchema( kpropTertTemp,    kcolRecipeTertTemp,     kxmlPropTertTemp,     QString("real"), QVariant(20.0));
   m_properties[kpropAge]         = new PropertySchema( kpropAge,         kcolRecipeAge,          kxmlPropAge,          QString("real"), QVariant(0.0));
   m_properties[kpropAgeTemp]     = new PropertySchema( kpropAgeTemp,     kcolRecipeAgeTemp,      kxmlPropAgeTemp,      QString("real"), QVariant(20.0));
   m_properties[kpropDate]        = new PropertySchema( kpropDate,        kcolRecipeDate,         kxmlPropDate,         QString("date"), QString("now()"));
   m_properties[kpropCarbVols]    = new PropertySchema( kpropCarbVols,    kcolRecipeCarbVols,     kxmlPropCarbVols,     QString("real"), QVariant(0.0));
   m_properties[kpropForcedCarb]  = new PropertySchema( kpropForcedCarb,  kcolRecipeForcedCarb,   kxmlPropForcedCarb,   QString("boolean"), QVariant(false));
   m_properties[kpropPrimSugName] = new PropertySchema( kpropPrimSugName, kcolRecipePrimSugName,  kxmlPropPrimSugName,  QString("text"), QString("''"));
   m_properties[kpropCarbTemp]    = new PropertySchema( kpropCarbTemp,    kcolRecipeCarbTemp,     kxmlPropCarbTemp,     QString("real"), QVariant(20.0));
   m_properties[kpropPrimSugEquiv]= new PropertySchema( kpropPrimSugEquiv,kcolRecipePrimSugEquiv, kxmlPropPrimSugEquiv, QString("real"), QVariant(1.0));
   m_properties[kpropKegPrimFact] = new PropertySchema( kpropKegPrimFact, kcolRecipeKegPrimFact,  kxmlPropKegPrimFact,  QString("real"), QVariant(1.0));
   m_properties[kpropTasteNotes]  = new PropertySchema( kpropTasteNotes,  kcolRecipeTasteNotes,   kxmlPropTasteNotes,   QString("text"), QString("''"));
   m_properties[kpropTasteRating] = new PropertySchema( kpropTasteRating, kcolRecipeTasteRating,  kxmlPropTasteRating,  QString("real"), QVariant(20.0));

   m_properties[kpropDisplay]     = new PropertySchema( kpropDisplay,     kcolDisplay,            QString(),            QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]     = new PropertySchema( kpropDeleted,     kcolDeleted,            QString(),            QString("boolean"), QVariant(false));
   m_properties[kpropFolder]      = new PropertySchema( kpropFolder,      kpropFolder,            QString(),            QString("text"), QString("''"));

   // enough properties, now some foreign keys
   m_foreignKeys[kpropEquipmentId] = new PropertySchema( kpropEquipmentId, kcolRecipeEquipmentId, Brewtarget::EQUIPTABLE);
   m_foreignKeys[kpropMashId]      = new PropertySchema( kpropMashId,      kcolRecipeMashId,      Brewtarget::MASHTABLE);
   m_foreignKeys[kpropStyleId]     = new PropertySchema( kpropStyleId,     kcolRecipeStyleId,     Brewtarget::STYLETABLE);
   m_foreignKeys[kpropAncestorId]  = new PropertySchema( kpropAncestorId,  kcolRecipeAncestorId,  Brewtarget::RECTABLE);
}

void TableSchema::defineYeastTable()
{
   m_type = BASE;
   m_className = QString("Yeast");
   m_childTable = Brewtarget::YEASTCHILDTABLE;
   m_inRecTable = Brewtarget::YEASTINRECTABLE;
   m_invTable   = Brewtarget::YEASTINVTABLE;

   // These are defined in the global file.
   m_properties[kpropName]       = new PropertySchema( kpropName,       kcolName,            kxmlPropName,       QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]      = new PropertySchema( kpropNotes,      kcolNotes,           kxmlPropNotes,      QString("text"), QString("''"));
   m_properties[kpropType]       = new PropertySchema( kpropType,       kcolYeastType,       QString(),          QString("text"), QObject::tr("'Ale'"));
   m_properties[kpropForm]       = new PropertySchema( kpropForm,       kcolYeastForm,       QString(),          QString("text"), QObject::tr("'Liquid'"));
   m_properties[kpropAmount]     = new PropertySchema( kpropAmount,     kcolYeastAmount,     kxmlPropAmount,     QString("real"), QVariant(0.0));
   m_properties[kpropAmtIsWgt]   = new PropertySchema( kpropAmtIsWgt,   kcolYeastAmtIsWgt,   kxmlPropAmtIsWgt,   QString("boolean"), QVariant(false));
   m_properties[kpropLab]        = new PropertySchema( kpropLab,        kcolYeastLab,        kxmlPropLab,        QString("text"), QString("''"));
   m_properties[kpropProductID]  = new PropertySchema( kpropProductID,  kcolYeastProductID,  kxmlPropProductID,  QString("text"), QString("''"));
   m_properties[kpropMinTemp]    = new PropertySchema( kpropMinTemp,    kcolYeastMinTemp,    kxmlPropMinTemp,    QString("real"), QVariant(0.0));
   m_properties[kpropMaxTemp]    = new PropertySchema( kpropMaxTemp,    kcolYeastMaxTemp,    kxmlPropMaxTemp,    QString("real"), QVariant(0.0));
   m_properties[kpropFloc]       = new PropertySchema( kpropFloc,       kcolYeastFloc,       QString(),          QString("text"), QObject::tr("'Medium'"));
   m_properties[kpropAtten]      = new PropertySchema( kpropAttenPct,   kcolYeastAtten,      kxmlPropAtten,      QString("real"), QVariant(75.0));
   m_properties[kpropBestFor]    = new PropertySchema( kpropBestFor,    kcolYeastBestFor,    kxmlPropBestFor,    QString("text"), QString(""));
   m_properties[kpropTimesCultd] = new PropertySchema( kpropTimesCultd, kcolYeastTimesCultd, kxmlPropTimesCultd, QString("int"), QVariant(0));
   m_properties[kpropMaxReuse]   = new PropertySchema( kpropMaxReuse,   kcolYeastMaxReuse,   kxmlPropMaxReuse,   QString("int"), QVariant(10));
   m_properties[kpropAddToSec]   = new PropertySchema( kpropAddToSec,   kcolYeastAddToSec,   kxmlPropAddToSec,   QString("boolean"), QVariant(false));

   m_properties[kpropDisplay]    = new PropertySchema( kpropDisplay,    kcolDisplay,         QString(),          QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]    = new PropertySchema( kpropDeleted,    kcolDeleted,         QString(),          QString("boolean"), QVariant(false));
   m_properties[kpropFolder]     = new PropertySchema( kpropFolder,     kpropFolder,         QString(),          QString("text"), QString("''"));

}

void TableSchema::defineBrewnoteTable()
{
   m_type = BASE;
   m_className = QString("BrewNote");

   m_properties[kpropBrewDate]        = new PropertySchema( kpropBrewDate,        kcolBNoteBrewDate,        kxmlPropBrewDate,        QString("timestamp"), QDateTime());
   m_properties[kpropFermDate]        = new PropertySchema( kpropFermDate,        kcolBNoteFermDate,        kxmlPropFermDate,        QString("timestamp"), QDateTime());
   m_properties[kpropSG]              = new PropertySchema( kpropSG,              kcolBNoteSG,              kxmlPropSG,              QString("real"), QVariant(1.0));
   m_properties[kpropVolIntoBoil]     = new PropertySchema( kpropVolIntoBoil,     kcolBNoteVolIntoBoil,     kxmlPropVolIntoBoil,     QString("real"), QVariant(0.0));
   m_properties[kpropStrikeTemp]      = new PropertySchema( kpropStrikeTemp,      kcolBNoteStrikeTemp,      kxmlPropStrikeTemp,      QString("real"), QVariant(70.0));
   m_properties[kpropMashFinTemp]     = new PropertySchema( kpropMashFinTemp,     kcolBNoteMashFinTemp,     kxmlPropMashFinTemp,     QString("real"), QVariant(67.0));
   m_properties[kpropOG]              = new PropertySchema( kpropOG,              kcolBNoteOG,              kxmlPropOG,              QString("real"), QVariant(1.0));
   m_properties[kpropPostBoilVol]     = new PropertySchema( kpropPostBoilVol,     kcolBNotePostBoilVol,     kxmlPropPostBoilVol,     QString("real"), QVariant(0.0));
   m_properties[kpropVolIntoFerm]     = new PropertySchema( kpropVolIntoFerm,     kcolBNoteVolIntoFerm,     kxmlPropVolIntoFerm,     QString("real"), QVariant(0.0));
   m_properties[kpropPitchTemp]       = new PropertySchema( kpropPitchTemp,       kcolBNotePitchTemp,       kxmlPropPitchTemp,       QString("real"), QVariant(20.0));
   m_properties[kpropFG]              = new PropertySchema( kpropFG,              kcolBNoteFG,              kxmlPropFG,              QString("real"), QVariant(1.0));
   m_properties[kpropEffIntoBoil]     = new PropertySchema( kpropEffIntoBoil,     kcolBNoteEffIntoBoil,     kxmlPropEffIntoBoil,     QString("real"), QVariant(70.0));
   m_properties[kpropABV]             = new PropertySchema( kpropABV,             kcolBNoteABV,             kxmlPropABV,             QString("real"), QVariant(0.0));
   m_properties[kpropProjOG]          = new PropertySchema( kpropProjOG,          kcolBNoteProjOG,          kxmlPropProjOG,          QString("real"), QVariant(1.0));
   m_properties[kpropBrewhsEff]       = new PropertySchema( kpropBrewhsEff,       kcolBNoteBrewhsEff,       kxmlPropBrewhsEff,       QString("real"), QVariant(70.0));
   m_properties[kpropProjBoilGrav]    = new PropertySchema( kpropProjBoilGrav,    kcolBNoteProjBoilGrav,    kxmlPropProjBoilGrav,    QString("real"), QVariant(1.0));
   m_properties[kpropProjStrikeTemp]  = new PropertySchema( kpropProjStrikeTemp,  kcolBNoteProjStrikeTemp,  kxmlPropProjStrikeTemp,  QString("real"), QVariant(70.0));
   m_properties[kpropProjMashFinTemp] = new PropertySchema( kpropProjMashFinTemp, kcolBNoteProjMashFinTemp, kxmlPropProjMashFinTemp, QString("real"), QVariant(67.0));
   m_properties[kpropProjVolIntoBoil] = new PropertySchema( kpropProjVolIntoBoil, kcolBNoteProjVolIntoBoil, kxmlPropProjVolIntoBoil, QString("real"), QVariant(1.0));
   m_properties[kpropProjOG]          = new PropertySchema( kpropProjOG,          kcolBNoteProjOG,          kxmlPropProjOG,          QString("real"), QVariant(1.0));
   m_properties[kpropProjVolIntoFerm] = new PropertySchema( kpropProjVolIntoFerm, kcolBNoteProjVolIntoFerm, kxmlPropProjVolIntoFerm, QString("real"), QVariant(0.0));
   m_properties[kpropProjFG]          = new PropertySchema( kpropProjFG,          kcolBNoteProjFG,          kxmlPropProjFG,          QString("real"), QVariant(1.0));
   m_properties[kpropProjEff]         = new PropertySchema( kpropProjEff,         kcolBNoteProjEff,         kxmlPropProjEff,         QString("real"), QVariant(1.0));
   m_properties[kpropProjABV]         = new PropertySchema( kpropProjABV,         kcolBNoteProjABV,         kxmlPropProjABV,         QString("real"), QVariant(1.0));
   m_properties[kpropProjAtten]       = new PropertySchema( kpropProjAtten,       kcolBNoteProjAtten,       kxmlPropProjAtten,       QString("real"), QVariant(75.0));
   m_properties[kpropProjPnts]        = new PropertySchema( kpropProjPnts,        kcolBNoteProjPnts,        kxmlPropProjPnts,        QString("real"), QVariant(1.0));
   m_properties[kpropProjFermPnts]    = new PropertySchema( kpropProjFermPnts,    kcolBNoteProjFermPnts,    kxmlPropProjFermPnts,    QString("real"), QVariant(1.0));
   m_properties[kpropBoilOff]         = new PropertySchema( kpropBoilOff,         kcolBNoteBoilOff,         kxmlPropBoilOff,         QString("real"), QVariant(1.0));
   m_properties[kpropFinVol]          = new PropertySchema( kpropFinVol,          kcolBNoteFinVol,          kxmlPropFinVol,          QString("real"), QVariant(1.0));
   m_properties[kpropAtten]           = new PropertySchema( kpropAtten,           kcolBNoteAtten,           kxmlPropAtten,           QString("real"), QVariant(1.0));

   m_properties[kpropDisplay]         = new PropertySchema( kpropDisplay,         kcolDisplay,              QString(),               QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]         = new PropertySchema( kpropDeleted,         kcolDeleted,              QString(),               QString("boolean"), QVariant(false));
   m_properties[kpropFolder]          = new PropertySchema( kpropFolder,          kpropFolder,              QString(),               QString("text"), QString("''"));

   m_foreignKeys[kpropRecipeId] = new PropertySchema( kpropRecipeId, kcolRecipeId, Brewtarget::RECTABLE);

}

void TableSchema::defineChildTable(Brewtarget::DBTable table)
{
   m_type = CHILD;

   m_foreignKeys[kpropParentId] = new PropertySchema( kpropParentId, kcolParentId, table);
   m_foreignKeys[kpropChildId]  = new PropertySchema( kpropChildId,  kcolChildId,  table);

}

void TableSchema::defineInRecipeTable(QString childIdx, Brewtarget::DBTable table)
{

   m_type = INREC;
   // this sort of breaks the rule -- I prefer to have a kcol and a kprop. But
   // these aren't in any object, so they no property and I can reuse
   // the kcol value
   m_foreignKeys[kpropRecipeId] = new PropertySchema( kpropRecipeId, kcolRecipeId, Brewtarget::RECTABLE);
   m_foreignKeys[childIdx]      = new PropertySchema( childIdx,      childIdx,     table);

}

void TableSchema::defineBtTable(QString childIdx, Brewtarget::DBTable table)
{
   m_type = BT;

   // What good is a rule followed to well?
   m_foreignKeys[childIdx] = new PropertySchema( childIdx, childIdx, table);

}

void TableSchema::defineFermInventoryTable()
{
   m_type = INV;

   m_properties[kpropAmount]        = new PropertySchema( kpropAmount,       kcolAmount,        kxmlPropAmount, QString("real"), QVariant(0.0));
   m_foreignKeys[kcolFermentableId] = new PropertySchema( kcolFermentableId, kcolFermentableId, Brewtarget::FERMTABLE);

}

void TableSchema::defineHopInventoryTable()
{

   m_type = INV;
   m_properties[kpropAmount] = new PropertySchema( kpropAmount, kcolAmount, kxmlPropAmount, QString("real"), QVariant(0.0));
   m_foreignKeys[kcolHopId]  = new PropertySchema( kcolHopId,   kcolHopId,  Brewtarget::HOPTABLE);

}

void TableSchema::defineMiscInventoryTable()
{

   m_type = INV;
   m_properties[kpropAmount] = new PropertySchema( kpropAmount, kcolAmount, kxmlPropAmount, QString("real"), QVariant(0.0));
   m_foreignKeys[kcolMiscId] = new PropertySchema( kcolMiscId,  kcolMiscId, Brewtarget::MISCTABLE);

}

void TableSchema::defineYeastInventoryTable()
{

   m_type = INV;
   m_properties[kpropQuanta]  = new PropertySchema( kpropQuanta, kcolYeastQuanta, kxmlPropAmount, QString("real"), QVariant(0.0));
   m_properties[kpropAmount]  = new PropertySchema( kpropQuanta, kcolYeastQuanta, kxmlPropAmount, QString("real"), QVariant(0.0));
   m_foreignKeys[kcolYeastId] = new PropertySchema( kcolYeastId, kcolYeastId,     Brewtarget::YEASTTABLE);

}
