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
#include "WaterSchema.h"
#include "SaltSchema.h"
#include "BrewnoteSchema.h"
#include "SettingsSchema.h"

static const QString kDefault("DEFAULT");

TableSchema::TableSchema(Brewtarget::DBTable table)
    : QObject(nullptr),
      m_tableName( Brewtarget::dbTableToName[ static_cast<int>(table) ] ),
      m_dbTable(table),
      m_childTable(Brewtarget::NOTABLE),
      m_inRecTable(Brewtarget::NOTABLE),
      m_invTable(Brewtarget::NOTABLE),
      m_btTable(Brewtarget::NOTABLE),
      m_trigger(QString()),
      m_defType(Brewtarget::dbType())
{
    // for this bit of ugly, I gain a lot of utility.
    defineTable();
}

// almost everything is a get. The initialization is expected all the parameters

const QString TableSchema::tableName() const { return m_tableName; }
const QString TableSchema::className() const { return m_className; }
Brewtarget::DBTable TableSchema::dbTable() const { return m_dbTable; }
Brewtarget::DBTable TableSchema::childTable() const  { return m_childTable; }
Brewtarget::DBTable TableSchema::inRecTable() const { return m_inRecTable; }
Brewtarget::DBTable TableSchema::invTable() const { return m_invTable; }
Brewtarget::DBTable TableSchema::btTable() const { return m_btTable; }
const QString TableSchema::triggerProperty() const { return m_trigger; }

const QMap<QString,PropertySchema*> TableSchema::properties() const { return m_properties; }
const QMap<QString,PropertySchema*> TableSchema::foreignKeys() const { return m_foreignKeys; }
const PropertySchema* TableSchema::key() const { return m_key; }
Brewtarget::DBTypes TableSchema::defType() const { return m_defType; }

const QString TableSchema::keyName( Brewtarget::DBTypes type ) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;

   return m_key->colName(selected);
}

const QStringList TableSchema::allPropertyNames(Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;

   QMapIterator<QString,PropertySchema*> i(m_properties);
   QStringList retval;
   while ( i.hasNext() ) {
      i.next();
      retval.append( i.value()->propName(selected));
   }
   return retval;
}

const QStringList TableSchema::allForeignKeyNames(Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QMapIterator<QString,PropertySchema*> i(m_foreignKeys);
   QStringList retval;
   while ( i.hasNext() ) {
      i.next();
      retval.append( i.value()->colName(selected));
   }
   return retval;
}

const QStringList TableSchema::allColumnNames(Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QStringList tmp;
   QMapIterator<QString,PropertySchema*> i(m_properties);

   while ( i.hasNext() ) {
      i.next();
      tmp.append(i.value()->colName(selected));
   }
   return tmp;
}

const QStringList TableSchema::allForeignKeyColumnNames(Brewtarget::DBTypes type) const
{
   QStringList tmp;
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;

   QMapIterator<QString,PropertySchema*> i(m_foreignKeys);

   while ( i.hasNext() ) {
      i.next();
      tmp.append(i.value()->colName(selected));
   } return tmp;
}

const PropertySchema* TableSchema::property(QString prop) const
{
   PropertySchema* retval = nullptr;
   if ( m_properties.contains(prop) ) {
      retval = m_properties.value(prop);
   }
   return retval;
}

const QString TableSchema::propertyName(QString prop, Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString retval;
   if ( m_properties.contains(prop) ) {
      retval =  m_properties.value(prop)->propName(selected);
   }
   return retval;

}
const QString TableSchema::propertyToColumn(QString prop, Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString retval;
   if ( m_properties.contains(prop) ) {
      retval =  m_properties.value(prop)->colName(selected);
   }
   return retval;
}

const QString TableSchema::foreignKeyToColumn(QString fkey, Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString retval;
   if ( m_foreignKeys.contains(fkey) ) {
      retval =  m_foreignKeys.value(fkey)->colName(selected);
   }
   return retval;
}

const QString TableSchema::foreignKeyToColumn(Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString retval;

   if ( m_foreignKeys.size() == 1 ) {
      retval = m_foreignKeys.first()->colName(selected);
   }
   return retval;
}

const QString TableSchema::propertyToXml(QString prop, Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString retval;
   if ( m_properties.contains(prop) ) {
      retval = m_properties.value(prop)->xmlName(selected);
   }
   if ( retval.isEmpty() ) {
      foreach( PropertySchema* p, m_properties.values() ) {
         if ( p->propName(selected) == prop ) {
            retval = p->xmlName(selected);
         }
      }
   }

   return retval;
}

const QString TableSchema::xmlToProperty(QString xmlName, Brewtarget::DBTypes type) const
{
   QString retval;
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;

   QMapIterator<QString,PropertySchema*> i(m_properties);

   while ( i.hasNext() ) {
      i.next();
      if ( i.value()->xmlName() == xmlName ) {
         retval = i.value()->propName(selected);
         break;
      }
   }
   return retval;
}

const QString TableSchema::propertyColumnType(QString prop, Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->colType(selected);
   }
   else {
      return QString();
   }
}

const QVariant TableSchema::propertyColumnDefault(QString prop, Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QVariant retval = QString();
   if ( m_properties.contains(prop) ) {
      retval = m_properties.value(prop)->defaultValue(selected);
   }
   return retval;
}

int TableSchema::propertyColumnSize(QString prop, Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   if ( m_properties.contains(prop) ) {
      return m_properties.value(prop)->colSize(selected);
   }
   else {
      return 0;
   }
}

Brewtarget::DBTable TableSchema::foreignTable(QString fkey, Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   Brewtarget::DBTable retval = Brewtarget::NOTABLE;

   if ( m_foreignKeys.contains(fkey) ) {
      retval =  m_foreignKeys.value(fkey)->fTable(selected);
   }
   return retval;

}

Brewtarget::DBTable TableSchema::foreignTable(Brewtarget::DBTypes type) const
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   Brewtarget::DBTable retval = Brewtarget::NOTABLE;

   if ( m_foreignKeys.size() == 1 ) {
      retval =  m_foreignKeys.first()->fTable(selected);
   }
   return retval;
}

bool TableSchema::isInventoryTable() { return m_type == INV; }
bool TableSchema::isBaseTable()      { return m_type == BASE; }
bool TableSchema::isChildTable()     { return m_type == CHILD; }
bool TableSchema::isInRecTable()     { return m_type == INREC; }
bool TableSchema::isBtTable()        { return m_type == BT; }
bool TableSchema::isMetaTable()      { return m_type == META; }

const QString TableSchema::childIndexName(Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString cname;

   if ( m_type == CHILD || m_type == BT ) {
      QMapIterator<QString,PropertySchema*> i(m_foreignKeys);

      while ( i.hasNext() ) {
         i.next();
         if ( i.value()->colName(selected) != kpropRecipeId ) {
            cname = i.value()->colName(selected);
            break;
         }
      }
   }
   return cname;
}

const QString TableSchema::inRecIndexName(Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString cname;

   if ( m_type == INREC ) {
      QMapIterator<QString,PropertySchema*> i(m_foreignKeys);

      while ( i.hasNext() ) {
         i.next();
         if ( i.value()->colName(selected) != kpropRecipeId ) {
            cname = i.value()->colName(selected);
            break;
         }
      }
   }
   return cname;
}

const QString TableSchema::recipeIndexName(Brewtarget::DBTypes type)
{
   QString cname;
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;

   if ( m_foreignKeys.contains(kpropRecipeId) ) {
      cname = m_foreignKeys.value(kpropRecipeId)->colName(selected);
   }

   return cname;
}

const QString TableSchema::parentIndexName(Brewtarget::DBTypes type)
{
   QString cname;
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;

   if ( m_foreignKeys.contains(kpropParentId) ) {
      cname = m_foreignKeys.value(kpropParentId)->colName(selected);
   }

   return cname;
}

const QString TableSchema::generateCreateTable(Brewtarget::DBTypes type, QString tmpName)
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString tname = tmpName.isEmpty() ? m_tableName : tmpName;
   QString retVal = QString("CREATE TABLE %1 (%2 %3 ")
                     .arg( tname )
                     .arg( m_key->colName(selected) )
                     .arg( m_key->constraint(selected)
   );

   QString retKeys;
   QMapIterator<QString, PropertySchema*> i(m_properties);
   while ( i.hasNext() ) {
      i.next();
      PropertySchema* prop = i.value();

      // based on the different way a boolean is handled between sqlite and
      // pgsql, I need to single them out.
      QVariant defVal = prop->defaultValue(selected);
      if ( defVal.isValid() ) {
         QString tmp = defVal.toString();
         if ( prop->colType() == "boolean" ) {
            tmp = Brewtarget::dbBoolean(defVal.toBool(),type);
         }

         // this isn't quite perfect, as you will get two spaces between the type
         // and DEFAULT if there are no constraints. On the other hand, nobody
         // will know that but me and the person reading this comment.
         retVal.append( QString(", %1 %2 %3 %4 %5")
                           .arg( prop->colName() ).arg( prop->colType() )
                           .arg( prop->constraint() ).arg( kDefault ).arg( tmp )
         );
      }
      else {
         retVal.append( QString("%1 %2 %3, ")
               .arg( prop->colName() ).arg( prop->colType() ).arg( prop->constraint() ));
      }
   }

   // SQLITE wants the foreign key declarations go at the end, and they cannot
   // be intermixed with other column defs. This is an ugly hack to make it
   // work
   QMapIterator<QString, PropertySchema*> j(m_foreignKeys);
   while ( j.hasNext() ) {
      j.next();
      PropertySchema* key = j.value();

      retVal.append( QString(", %1 %2").arg( key->colName(selected) ).arg( key->colType(selected) ));

      retKeys.append( QString(", FOREIGN KEY(%1) REFERENCES %2(id)")
                       .arg( key->colName(selected) )
                       .arg( Brewtarget::dbTableToName[ key->fTable() ] )
      );
   }

   if ( ! retKeys.isEmpty() ) {
      retVal.append( retKeys );
   }
   retVal.append(");");

   return retVal;
}

const QString TableSchema::generateInsertRow(Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString columns = keyName(selected);
   QString binding = QString(":%1").arg(keyName(selected));

   QMapIterator<QString, PropertySchema*> i(m_properties);
   while ( i.hasNext() ) {
      i.next();
      PropertySchema* prop = i.value();

      columns += QString(",%1").arg( prop->colName(selected));
      binding += QString(",:%1").arg( prop->colName(selected));
   }

   QMapIterator<QString, PropertySchema*> j(m_foreignKeys);
   while ( j.hasNext() ) {
      j.next();
      PropertySchema* key = j.value();

      columns += QString(",%1").arg(key->colName(selected));
      binding += QString(",:%1").arg(key->colName(selected));
   }
   return QString("INSERT INTO %1 (%2) VALUES(%3)").arg(m_tableName).arg(columns).arg(binding);
}

// NOTE: This does NOT deal with foreign keys nor the primary key for the table. It assumes
// any calling method will handle those relationships. In my rough design ideas, a table knows
// of itself and foreign key *values* are part of the database.
// To make other parts of the code easier, I am making certain that the bound values use the property name
// and not the column name. It saves a call later.
const QString TableSchema::generateInsertProperties(Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString columns;
   QString binding;

   QMapIterator<QString, PropertySchema*> i(m_properties);
   while ( i.hasNext() ) {
      i.next();
      PropertySchema* prop = i.value();

      if ( columns.isEmpty() ) {
         columns = QString("%1").arg( prop->colName(selected));
         binding = QString(":%1").arg( prop->propName(selected));
      }
      else {
         columns += QString(",%1").arg( prop->colName(selected));
         binding += QString(",:%1").arg( prop->propName(selected));
      }
   }

   return QString("INSERT INTO %1 (%2) VALUES(%3)").arg(m_tableName).arg(columns).arg(binding);

}

// note: this does not do anything with foreign keys. It is up to the calling code to handle those problems
const QString TableSchema::generateUpdateRow(int key, Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString columns;

   QMapIterator<QString, PropertySchema*> i(m_properties);
   while ( i.hasNext() ) {
      i.next();
      PropertySchema* prop = i.value();
      if ( ! columns.isEmpty() ) {
         columns += QString(",%1=:%1").arg( prop->colName(selected));
      }
      else {
         columns = QString("%1=:%1").arg( prop->colName(selected) );
      }
   }

   return QString("UPDATE %1 SET %2 where %3=%4")
           .arg(m_tableName)
           .arg(columns)
           .arg(keyName(selected))
           .arg(key);
}

// note: this does not do anything with foreign keys. It is up to the calling code to handle those problems
// unlike the previous method, this one uses a bind named ":id" for the key value.
const QString TableSchema::generateUpdateRow(Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString columns;

   QMapIterator<QString, PropertySchema*> i(m_properties);
   while ( i.hasNext() ) {
      i.next();
      PropertySchema* prop = i.value();
      if ( ! columns.isEmpty() ) {
         columns += QString(",%1=:%1").arg( prop->colName(selected));
      }
      else {
         columns = QString("%1=:%1").arg( prop->colName(selected) );
      }
   }

   return QString("UPDATE %1 SET %2 where %3=:id")
           .arg(m_tableName)
           .arg(columns)
           .arg(keyName(selected));
}

const QString TableSchema::generateCopyTable( QString dest, Brewtarget::DBTypes type )
{
   Brewtarget::DBTypes selected = type == Brewtarget::ALLDB ? m_defType : type;
   QString columns = keyName(selected);

   QMapIterator<QString, PropertySchema*> i(m_properties);
   while ( i.hasNext() ) {
      i.next();
      PropertySchema* prop = i.value();

      columns += QString(",%1").arg( prop->colName(selected));
   }

   QMapIterator<QString, PropertySchema*> j(m_foreignKeys);
   while ( j.hasNext() ) {
      j.next();
      PropertySchema* key = j.value();

      columns += QString(",%1").arg(key->colName(selected));
   }

   return QString("INSERT INTO %1 (%2) SELECT %2 FROM %3").arg(dest).arg(columns).arg(m_tableName);

}

// right now, only instruction_number has an increment (or decrement) trigger.
// if we invent others, the m_trigger property will need to be set for that table.
// this only handles one trigger per table. It could be made to handle a list, maybe.
const QString TableSchema::generateIncrementTrigger(Brewtarget::DBTypes type)
{
   QString retval;

   if ( m_trigger.isEmpty() )
      return retval;

   if ( type == Brewtarget::PGSQL ) {
      // create or replace function increment_instruction_num() returns trigger as $BODY$
      //   begin update instruction_in_recipe set instruction_number = (SELECT max(instruction_number) from instruction_in_recipe where recipe_id = new.recipe_id) + 1
      //         where id = NEW.id;
      //         return NULL;
      //   end;
      //   $BODY$ LANGUAGE plpgsql;
      retval = QString("CREATE OR REPLACE FUNCTION increment_instruction_num() RETURNS TRIGGER AS $BODY$ "
                       "BEGIN UPDATE %1 SET %2 = (SELECT max(%2) from %1 where %3 = NEW.%3) + 1 WHERE %4 = NEW.%4; "
                       "return NULL;"
                       "END;"
                       "$BODY$ LANGUAGE plpgsql;")
            .arg(m_tableName)
            .arg(propertyToColumn(m_trigger))
            .arg(recipeIndexName())
            .arg(keyName());
      // I do not like this, in that I am stringing these together in bad ways
      retval += QString("CREATE TRIGGER inc_ins_num AFTER INSERT ON %1 "
                        "FOR EACH ROW EXECUTE PROCEDURE increment_instruction_num();")
            .arg(m_tableName);
   }
   else {
     retval = QString("CREATE TRIGGER inc_ins_num AFTER INSERT ON %1 "
                      "BEGIN "
                         "UPDATE %1 SET %2 = (SELECT max(%2) from %1 where %3 = new.%3) + 1 "
                         "WHERE rowid = new.rowid;"
                      "END")
           .arg(m_tableName)
           .arg(propertyToColumn(m_trigger))
           .arg(recipeIndexName());
   }
   return retval;
}

const QString TableSchema::generateDecrementTrigger(Brewtarget::DBTypes type)
{
   QString retval;

   if ( m_trigger.isEmpty() )
      return retval;

   if ( type == Brewtarget::PGSQL ) {
      // create or replace function decrement_instruction_num() returns trigger as $BODY$
      //   begin update instruction_in_recipe set instruction_number = instruction_number - 1
      //         where recipe_id = OLD.recipe_id AND instruction_number > OLD.instruction_number;
      //         return NULL;
      //   end;
      //   $BODY$ LANGUAGE plpgsql;
      retval = QString("CREATE OR REPLACE FUNCTION decrement_instruction_num() RETURNS TRIGGER AS $BODY$ "
                       "BEGIN UPDATE %1 SET %2 = %2 - 1 "
                         "WHERE %3 = OLD.%3 AND %2 > OLD.%2;"
                         "return NULL;"
                       "END;"
                       "$BODY$ LANGUAGE plpgsql;")
            .arg(tableName())
            .arg(propertyToColumn(m_trigger))
            .arg(recipeIndexName());
      retval += QString("CREATE TRIGGER dec_ins_num AFTER DELETE ON %1 "
                        "FOR EACH ROW EXECUTE PROCEDURE decrement_instruction_num();")
            .arg(tableName());
   }
   else {
      // CREATE TRIGGER dec_ins_num after DELETE ON instruction_in_recipe
      // BEGIN
      //   UPDATE instuction_in_recipe SET instruction_number = instruction_number - 1
      //   WHERE recipe_id = OLD.recipe_id AND  instruction_number > OLD.instruction_number
      // END
     retval = QString("CREATE TRIGGER dec_ins_num AFTER DELETE ON %1 "
                      "BEGIN "
                        "UPDATE %1 SET %2 = %2 - 1 "
                        "WHERE %3 = OLD.%3 AND %2 > OLD.%2; "
                      "END")
           .arg( tableName() )
           .arg(propertyToColumn(m_trigger))
           .arg(recipeIndexName());
   }
   return retval;
}
// This got long. Not sure if there's a better way to do it.
void TableSchema::defineTable()
{
   switch( m_dbTable ) {
      case Brewtarget::SETTINGTABLE:
         defineSettingsTable();
         break;
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
      case Brewtarget::WATERTABLE:
         defineWaterTable();
         break;
      case Brewtarget::SALTTABLE:
         defineSaltTable();
         break;
      case Brewtarget::BT_EQUIPTABLE:
         defineBtTable(kcolEquipmentId, Brewtarget::EQUIPTABLE);
         break;
      case Brewtarget::BT_FERMTABLE:
         defineBtTable(kcolFermentableId, Brewtarget::FERMTABLE);
         break;
      case Brewtarget::BT_HOPTABLE:
         defineBtTable(kcolHopId, Brewtarget::HOPTABLE);
         break;
      case Brewtarget::BT_MISCTABLE:
         defineBtTable(kcolMiscId, Brewtarget::MISCTABLE);
         break;
      case Brewtarget::BT_STYLETABLE:
         defineBtTable(kcolStyleId, Brewtarget::STYLETABLE);
         break;
      case Brewtarget::BT_WATERTABLE:
         defineBtTable(kcolWaterId, Brewtarget::WATERTABLE);
         break;
      case Brewtarget::BT_YEASTTABLE:
         defineBtTable(kcolYeastId, Brewtarget::YEASTTABLE);
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
      case Brewtarget::INSTINRECTABLE:
         defineInstructionInRecipeTable( kcolInstructionId, Brewtarget::INSTRUCTIONTABLE);
         break;
      case Brewtarget::MISCINRECTABLE:
         defineInRecipeTable(kcolMiscId, Brewtarget::MISCTABLE);
         break;
      case Brewtarget::WATERINRECTABLE:
         defineInRecipeTable(kcolWaterId, Brewtarget::WATERTABLE);
         break;
      case Brewtarget::SALTINRECTABLE:
         defineInRecipeTable(kcolSaltId, Brewtarget::SALTTABLE);
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
static const QString kPgSQLConstraint("SERIAL PRIMARY KEY");
static const QString kSQLiteConstraint("INTEGER PRIMARY KEY autoincrement");

void TableSchema::defineStyleTable()
{
   m_type = BASE;
   m_className = QString("Style");
   m_childTable = Brewtarget::STYLECHILDTABLE;
   m_btTable = Brewtarget::BT_STYLETABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropName]      = new PropertySchema( kpropName,       kpropName,         kxmlPropName,     QString("text"), QString("''"), QString("not null"));
   m_properties[kpropType]      = new PropertySchema( kpropTypeString, kcolStyleType,     kxmlPropType,     QString("text"), QString("'Ale'"));
   m_properties[kpropCat]       = new PropertySchema( kpropCat,        kcolStyleCat,      kxmlPropCat,      QString("text"), QString("''"));
   m_properties[kpropCatNum]    = new PropertySchema( kpropCatNum,     kcolStyleCatNum,   kxmlPropCatNum,   QString("text"), QString("''"));
   m_properties[kpropLetter]    = new PropertySchema( kpropLetter,     kcolStyleLetter,   kxmlPropLetter,   QString("text"), QString("''"));
   m_properties[kpropGuide]     = new PropertySchema( kpropGuide,      kcolStyleGuide,    kxmlPropGuide,    QString("text"), QString("''"));
   m_properties[kpropOGMin]     = new PropertySchema( kpropOGMin,      kcolStyleOGMin,    kxmlPropOGMin,    QString("real"), QVariant(0.0));
   m_properties[kpropOGMax]     = new PropertySchema( kpropOGMax,      kcolStyleOGMax,    kxmlPropOGMax,    QString("real"), QVariant(0.0));
   m_properties[kpropFGMin]     = new PropertySchema( kpropFGMin,      kcolStyleFGMin,    kxmlPropFGMin,    QString("real"), QVariant(0.0));
   m_properties[kpropFGMax]     = new PropertySchema( kpropFGMax,      kcolStyleFGMax,    kxmlPropFGMax,    QString("real"), QVariant(0.0));
   m_properties[kpropIBUMin]    = new PropertySchema( kpropIBUMin,     kcolStyleIBUMin,   kxmlPropIBUMin,   QString("real"), QVariant(0.0));
   m_properties[kpropIBUMax]    = new PropertySchema( kpropIBUMax,     kcolStyleIBUMax,   kxmlPropIBUMax,   QString("real"), QVariant(0.0));
   m_properties[kpropColorMin]  = new PropertySchema( kpropColorMin,   kcolStyleColorMin, kxmlPropColorMin, QString("real"), QVariant(0.0));
   m_properties[kpropColorMax]  = new PropertySchema( kpropColorMax,   kcolStyleColorMax, kxmlPropColorMax, QString("real"), QVariant(0.0));
   m_properties[kpropABVMin]    = new PropertySchema( kpropABVMin,     kcolStyleABVMin,   kxmlPropABVMin,   QString("real"), QVariant(0.0));
   m_properties[kpropABVMax]    = new PropertySchema( kpropABVMax,     kcolStyleABVMax,   kxmlPropABVMax,   QString("real"), QVariant(0.0));
   m_properties[kpropCarbMin]   = new PropertySchema( kpropCarbMin,    kcolStyleCarbMin,  kxmlPropCarbMin,  QString("real"), QVariant(0.0));
   m_properties[kpropCarbMax]   = new PropertySchema( kpropCarbMax,    kcolStyleCarbMax,  kxmlPropCarbMax,  QString("real"), QVariant(0.0));
   m_properties[kpropNotes]     = new PropertySchema( kpropNotes,      kcolNotes,         kxmlPropNotes,    QString("text"), QString("''"));
   m_properties[kpropProfile]   = new PropertySchema( kpropProfile,    kcolStyleProfile,  kxmlPropProfile,  QString("text"), QString("''"));
   m_properties[kpropIngreds]   = new PropertySchema( kpropIngreds,    kcolStyleIngreds,  kxmlPropIngreds,  QString("text"), QString("''"));
   m_properties[kpropExamples]  = new PropertySchema( kpropExamples,   kcolStyleExamples, kxmlPropExamples, QString("text"), QString("''"));

   // not sure about these, but I think I'm gonna need them anyway
   m_properties[kpropDisplay]   = new PropertySchema(kpropDisplay,   kcolDisplay,       QString(),        QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]   = new PropertySchema(kpropDeleted,   kcolDeleted,       QString(),        QString("boolean"), QVariant(false));
   m_properties[kpropFolder]    = new PropertySchema(kpropFolder,    kcolFolder,        QString(),        QString("text"),    QString("''"));
}

void TableSchema::defineEquipmentTable()
{
   m_type = BASE;
   m_className = QString("Equipment");
   m_childTable = Brewtarget::EQUIPCHILDTABLE;
   m_btTable = Brewtarget::BT_EQUIPTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

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
   m_properties[kpropFolder]        = new PropertySchema( kpropFolder,        kcolFolder,       QString(),               QString("text"), QString("''"));

}

void TableSchema::defineFermentableTable()
{
   m_type = BASE;
   m_className = QString("Fermentable");
   m_childTable = Brewtarget::FERMCHILDTABLE;
   m_inRecTable = Brewtarget::FERMINRECTABLE;
   m_invTable   = Brewtarget::FERMINVTABLE;
   m_btTable    = Brewtarget::BT_FERMTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropName]           = new PropertySchema( kpropName,           kcolName,               kxmlPropName,           QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]          = new PropertySchema( kpropNotes,          kcolNotes,              kxmlPropNotes,          QString("text"), QString("''"));
   m_properties[kpropType]           = new PropertySchema( kpropTypeString,     kcolFermType,           kxmlPropType,           QString("text"), QString("'Grain'"));
   m_properties[kpropAmountKg]       = new PropertySchema( kpropAmountKg,       kcolAmount,             kxmlPropAmount,         QString("real"), QVariant(0.0));
   m_properties[kpropYield]          = new PropertySchema( kpropYield,          kcolFermYield,          kxmlPropYield,          QString("real"), QVariant(0.0));
   m_properties[kpropColor]          = new PropertySchema( kpropColor,          kcolFermColor,          kxmlPropColor,          QString("real"), QVariant(0.0));
   m_properties[kpropAddAfterBoil]   = new PropertySchema( kpropAddAfterBoil,   kcolFermAddAfterBoil,   kxmlPropAddAfterBoil,   QString("boolean"), QVariant(false));
   m_properties[kpropOrigin]         = new PropertySchema( kpropOrigin,         kcolFermOrigin,         kxmlPropOrigin,         QString("text"), QString("''"));
   m_properties[kpropSupplier]       = new PropertySchema( kpropSupplier,       kcolFermSupplier,       kxmlPropSupplier,       QString("text"), QString("''"));
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
   m_properties[kpropFolder]         = new PropertySchema( kpropFolder,         kcolFolder,             QString(),              QString("text"), QString("''"));

   // the inventory system is getting interesting
   m_foreignKeys[kpropInventoryId]   = new PropertySchema( kpropInventoryId,    kcolInventoryId,        QString("integer"),    m_invTable);

}

void TableSchema::defineHopTable()
{
   m_type = BASE;
   m_className = QString("Hop");
   m_childTable = Brewtarget::HOPCHILDTABLE;
   m_inRecTable = Brewtarget::HOPINRECTABLE;
   m_invTable   = Brewtarget::HOPINVTABLE;
   m_btTable    = Brewtarget::BT_HOPTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // These are defined in the global file.
   m_properties[kpropName]          = new PropertySchema( kpropName,          kcolName,             kxmlPropName,          QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]         = new PropertySchema( kpropNotes,         kcolNotes,            kxmlPropNotes,         QString("text"), QString("''"));
   m_properties[kpropAmountKg]      = new PropertySchema( kpropAmountKg,      kcolAmount,           kxmlPropAmount,        QString("real"), QVariant(0.0));
   m_properties[kpropUse]           = new PropertySchema( kpropUseString,     kcolUse,              kxmlPropUse,           QString("text"), QString("'Boil'"));
   m_properties[kpropTime]          = new PropertySchema( kpropTime,          kcolTime,             kxmlPropTime,          QString("real"), QVariant(0.0));
   m_properties[kpropOrigin]        = new PropertySchema( kpropOrigin,        kcolOrigin,           kxmlPropOrigin,        QString("text"), QString("''"));
   m_properties[kpropSubstitutes]   = new PropertySchema( kpropSubstitutes,   kcolSubstitutes,      kxmlPropSubstitutes,   QString("text"), QString("''"));
   m_properties[kpropAlpha]         = new PropertySchema( kpropAlpha,         kcolHopAlpha,         kxmlPropAlpha,         QString("real"), QVariant(0.0));
   m_properties[kpropType]          = new PropertySchema( kpropTypeString,    kcolHopType,          kxmlPropType,          QString("text"), QString("'Boil'"));
   m_properties[kpropForm]          = new PropertySchema( kpropFormString,    kcolHopForm,          kxmlPropForm,          QString("text"), QString("'Pellet'"));
   m_properties[kpropBeta]          = new PropertySchema( kpropBeta,          kcolHopBeta,          kxmlPropBeta,          QString("real"), QVariant(0.0));
   m_properties[kpropHSI]           = new PropertySchema( kpropHSI,           kcolHopHSI,           kxmlPropHSI,           QString("real"), QVariant(0.0));
   m_properties[kpropHumulene]      = new PropertySchema( kpropHumulene,      kcolHopHumulene,      kxmlPropHumulene,      QString("real"), QVariant(0.0));
   m_properties[kpropCaryophyllene] = new PropertySchema( kpropCaryophyllene, kcolHopCaryophyllene, kxmlPropCaryophyllene, QString("real"), QVariant(0.0));
   m_properties[kpropCohumulone]    = new PropertySchema( kpropCohumulone,    kcolHopCohumulone,    kxmlPropCohumulone,    QString("real"), QVariant(0.0));
   m_properties[kpropMyrcene]       = new PropertySchema( kpropMyrcene,       kcolHopMyrcene,       kxmlPropMyrcene,       QString("real"), QVariant(0.0));

   m_properties[kpropDisplay]       = new PropertySchema( kpropDisplay,       kcolDisplay,          QString(),             QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]       = new PropertySchema( kpropDeleted,       kcolDeleted,          QString(),             QString("boolean"), QVariant(false));
   m_properties[kpropFolder]        = new PropertySchema( kpropFolder,        kcolFolder,           QString(),             QString("text"), QString("''"));

   m_foreignKeys[kpropInventoryId]  = new PropertySchema( kpropInventoryId,   kcolInventoryId,      QString("integer"),    m_invTable);
}

void TableSchema::defineInstructionTable()
{
   m_type = BASE;
   m_className = QString("Instruction");
   m_inRecTable = Brewtarget::INSTINRECTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

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

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // These are defined in the global file.
   m_properties[kpropName]        = new PropertySchema( kpropName,        kcolName,            kxmlPropName,        QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]       = new PropertySchema( kpropNotes,       kcolNotes,           kxmlPropNotes,       QString("text"), QString("''"));
   m_properties[kpropGrainTemp]   = new PropertySchema( kpropGrainTemp,   kcolMashGrainTemp,   kxmlPropGrainTemp,   QString("real"), QVariant(0.0));
   m_properties[kpropTunTemp]     = new PropertySchema( kpropTunTemp,     kcolMashTunTemp,     kxmlPropTunTemp,     QString("real"), QVariant(20.0));
   m_properties[kpropSpargeTemp]  = new PropertySchema( kpropSpargeTemp,  kcolMashSpargeTemp,  kxmlPropSpargeTemp,  QString("real"), QVariant(74.0));
   m_properties[kpropPH]          = new PropertySchema( kpropPH,          kcolPH,              kxmlPropPH,          QString("real"), QVariant(7.0));
   m_properties[kpropTunWeight]   = new PropertySchema( kpropTunWeight,   kcolMashTunWeight,   kxmlPropTunWeight,   QString("real"), QVariant(0.0));
   m_properties[kpropTunSpecHeat] = new PropertySchema( kpropTunSpecHeat, kcolMashTunSpecHeat, kxmlPropTunSpecHeat, QString("real"), QVariant(0.0));
   m_properties[kpropEquipAdjust] = new PropertySchema( kpropEquipAdjust, kcolMashEquipAdjust, kxmlPropEquipAdjust, QString("boolean"), QVariant(true));

   m_properties[kpropDisplay]     = new PropertySchema( kpropDisplay,     kcolDisplay,         QString(),           QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]     = new PropertySchema( kpropDeleted,     kcolDeleted,         QString(),           QString("boolean"), QVariant(false));
   m_properties[kpropFolder]      = new PropertySchema( kpropFolder,      kcolFolder,         QString(),           QString("text"), QString("''"));
}

// property name, column name, xml property name, column type, column default, column constraint
void TableSchema::defineMashstepTable()
{
   m_type = BASE;
   m_className = QString("MashStep");

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropName]       = new PropertySchema( kpropName,       kcolName,               kxmlPropName,       QString("text"), QString("''"),QString("not null"));
   m_properties[kpropType]       = new PropertySchema( kpropTypeString, kcolMashstepType,       kxmlPropType,       QString("text"), QString("'Infusion'"));
   m_properties[kpropInfuseAmt]  = new PropertySchema( kpropInfuseAmt,  kcolMashstepInfuseAmt,  kxmlPropInfuseAmt,  QString("real"), QVariant(0.0));
   m_properties[kpropStepTemp]   = new PropertySchema( kpropStepTemp,   kcolMashstepStepTemp,   kxmlPropStepTemp,   QString("real"), QVariant(67.0));
   m_properties[kpropStepTime]   = new PropertySchema( kpropStepTime,   kcolMashstepStepTime,   kxmlPropStepTime,   QString("real"), QVariant(0.0));
   m_properties[kpropRampTime]   = new PropertySchema( kpropRampTime,   kcolMashstepRampTime,   kxmlPropRampTime,   QString("real"), QVariant(0.0));
   m_properties[kpropEndTemp]    = new PropertySchema( kpropEndTemp,    kcolMashstepEndTemp,    kxmlPropEndTemp,    QString("real"), QVariant(67.0));
   m_properties[kpropInfuseTemp] = new PropertySchema( kpropInfuseTemp, kcolMashstepInfuseTemp, kxmlPropInfuseTemp, QString("real"), QVariant(67.0));
   m_properties[kpropDecoctAmt]  = new PropertySchema( kpropDecoctAmt,  kcolMashstepDecoctAmt,  kxmlPropDecoctAmt,  QString("real"), QVariant(67.0));
   m_properties[kpropStepNumber] = new PropertySchema( kpropStepNumber, kcolMashstepStepNumber, QString(),          QString("integer"), QVariant(0));

   m_properties[kpropDisplay]    = new PropertySchema( kpropDisplay,    kcolDisplay,            QString(),          QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]    = new PropertySchema( kpropDeleted,    kcolDeleted,            QString(),          QString("boolean"), QVariant(false));

   m_foreignKeys[kpropMashId]    = new PropertySchema( kpropMashId,     kcolMashId,       QString("integer"), Brewtarget::MASHTABLE);

}

void TableSchema::defineMiscTable()
{
   m_type = BASE;
   m_className = QString("Misc");
   m_childTable = Brewtarget::MISCCHILDTABLE;
   m_inRecTable = Brewtarget::MISCINRECTABLE;
   m_invTable   = Brewtarget::MISCINVTABLE;
   m_btTable    = Brewtarget::BT_MISCTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // These are defined in the global file.
   m_properties[kpropName]     = new PropertySchema( kpropName,       kcolName,         kxmlPropName,     QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]    = new PropertySchema( kpropNotes,      kcolNotes,        kxmlPropNotes,    QString("text"), QString("''"));
   m_properties[kpropAmount]   = new PropertySchema( kpropAmount,     kcolAmount,       kxmlPropAmount,   QString("real"), QVariant(0.0));
   m_properties[kpropUse]      = new PropertySchema( kpropUseString,  kcolUse,          kxmlPropUse,      QString("text"), QString("'Boil'"));
   m_properties[kpropTime]     = new PropertySchema( kpropTime,       kcolTime,         kxmlPropTime,     QString("real"), QVariant(0.0));
   m_properties[kpropType]     = new PropertySchema( kpropTypeString, kcolMiscType,     kxmlPropType,     QString("text"), QString("'Other'"));
   m_properties[kpropAmtIsWgt] = new PropertySchema( kpropAmtIsWgt,   kcolMiscAmtIsWgt, kxmlPropAmtIsWgt, QString("boolean"), QVariant(true));
   m_properties[kpropUseFor]   = new PropertySchema( kpropUseFor,     kcolMiscUseFor,   kxmlPropUseFor,   QString("text"), QString("''"));

   m_properties[kpropDisplay]  = new PropertySchema( kpropDisplay,  kcolDisplay,      QString(),        QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]  = new PropertySchema( kpropDeleted,  kcolDeleted,      QString(),        QString("boolean"), QVariant(false));
   m_properties[kpropFolder]   = new PropertySchema( kpropFolder,   kcolFolder,      QString(),        QString("text"), QString("''"));

   m_foreignKeys[kpropInventoryId]  = new PropertySchema( kpropInventoryId,   kcolInventoryId,      QString("integer"),    m_invTable);
}

void TableSchema::defineRecipeTable()
{
   m_type = BASE;
   m_className = QString("Recipe");
   m_childTable = Brewtarget::RECIPECHILDTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

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
   m_properties[kpropDate]        = new PropertySchema( kpropDate,        kcolRecipeDate,         kxmlPropDate,         QString("date"), QString("CURRENT_TIMESTAMP"));
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
   m_properties[kpropFolder]      = new PropertySchema( kpropFolder,      kcolFolder,            QString(),            QString("text"), QString("''"));
   // m_properties[kpropLocked]      = new PropertySchema( kpropLocked,      kcolLocked,             QString(),            QString("boolean"), QVariant(false));

   // enough properties, now some foreign keys

   m_foreignKeys[kpropEquipmentId] = new PropertySchema( kpropEquipmentId, kcolRecipeEquipmentId, QString("integer"), Brewtarget::EQUIPTABLE);
   m_foreignKeys[kpropMashId]      = new PropertySchema( kpropMashId,      kcolMashId,            QString("integer"), Brewtarget::MASHTABLE);
   m_foreignKeys[kpropStyleId]     = new PropertySchema( kpropStyleId,     kcolStyleId,           QString("integer"), Brewtarget::STYLETABLE);
   m_foreignKeys[kpropAncestorId]  = new PropertySchema( kpropAncestorId,  kcolRecipeAncestorId,  QString("integer"), Brewtarget::RECTABLE);
}

void TableSchema::defineYeastTable()
{
   m_type = BASE;
   m_className = QString("Yeast");
   m_childTable = Brewtarget::YEASTCHILDTABLE;
   m_inRecTable = Brewtarget::YEASTINRECTABLE;
   m_invTable   = Brewtarget::YEASTINVTABLE;
   m_btTable    = Brewtarget::BT_YEASTTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // These are defined in the global file.
   m_properties[kpropName]       = new PropertySchema( kpropName,       kcolName,            kxmlPropName,       QString("text"), QString("''"), QString("not null"));
   m_properties[kpropNotes]      = new PropertySchema( kpropNotes,      kcolNotes,           kxmlPropNotes,      QString("text"), QString("''"));
   m_properties[kpropType]       = new PropertySchema( kpropTypeString, kcolYeastType,       kxmlPropType,       QString("text"), QObject::tr("'Ale'"));
   m_properties[kpropForm]       = new PropertySchema( kpropFormString, kcolYeastForm,       kxmlPropForm,       QString("text"), QObject::tr("'Liquid'"));
   m_properties[kpropAmount]     = new PropertySchema( kpropAmount,     kcolYeastAmount,     kxmlPropAmount,     QString("real"), QVariant(0.0));
   m_properties[kpropAmtIsWgt]   = new PropertySchema( kpropAmtIsWgt,   kcolYeastAmtIsWgt,   kxmlPropAmtIsWgt,   QString("boolean"), QVariant(false));
   m_properties[kpropLab]        = new PropertySchema( kpropLab,        kcolYeastLab,        kxmlPropLab,        QString("text"), QString("''"));
   m_properties[kpropProductID]  = new PropertySchema( kpropProductID,  kcolYeastProductID,  kxmlPropProductID,  QString("text"), QString("''"));
   m_properties[kpropMinTemp]    = new PropertySchema( kpropMinTemp,    kcolYeastMinTemp,    kxmlPropMinTemp,    QString("real"), QVariant(0.0));
   m_properties[kpropMaxTemp]    = new PropertySchema( kpropMaxTemp,    kcolYeastMaxTemp,    kxmlPropMaxTemp,    QString("real"), QVariant(0.0));
   m_properties[kpropFloc]       = new PropertySchema( kpropFlocString, kcolYeastFloc,       kxmlPropFloc,       QString("text"), QObject::tr("'Medium'"));
   m_properties[kpropAttenPct]   = new PropertySchema( kpropAttenPct,   kcolYeastAtten,      kxmlPropAtten,      QString("real"), QVariant(75.0));
   m_properties[kpropBestFor]    = new PropertySchema( kpropBestFor,    kcolYeastBestFor,    kxmlPropBestFor,    QString("text"), QString("''"));
   m_properties[kpropTimesCultd] = new PropertySchema( kpropTimesCultd, kcolYeastTimesCultd, kxmlPropTimesCultd, QString("int"), QVariant(0));
   m_properties[kpropMaxReuse]   = new PropertySchema( kpropMaxReuse,   kcolYeastMaxReuse,   kxmlPropMaxReuse,   QString("int"), QVariant(10));
   m_properties[kpropAddToSec]   = new PropertySchema( kpropAddToSec,   kcolYeastAddToSec,   kxmlPropAddToSec,   QString("boolean"), QVariant(false));

   m_properties[kpropDisplay]    = new PropertySchema( kpropDisplay,    kcolDisplay,         QString(),          QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]    = new PropertySchema( kpropDeleted,    kcolDeleted,         QString(),          QString("boolean"), QVariant(false));
   m_properties[kpropFolder]     = new PropertySchema( kpropFolder,     kcolFolder,          QString(),          QString("text"),    QString("''"));

   m_foreignKeys[kpropInventoryId]  = new PropertySchema( kpropInventoryId,   kcolInventoryId,      QString("integer"),    m_invTable);
}

void TableSchema::defineBrewnoteTable()
{
   m_type = BASE;
   m_className = QString("BrewNote");

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropNotes]           = new PropertySchema( kpropNotes,           kcolNotes,                kxmlPropNotes,           QString("text"),    QString("''"));

   m_properties[kpropBrewDate]        = new PropertySchema( kpropBrewDate,        kcolBNoteBrewDate,        kxmlPropBrewDate,        QString("timestamp"), QString("CURRENT_TIMESTAMP"));
   m_properties[kpropFermDate]        = new PropertySchema( kpropFermDate,        kcolBNoteFermDate,        kxmlPropFermDate,        QString("timestamp"), QString("CURRENT_TIMESTAMP"));
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
   m_properties[kpropFolder]          = new PropertySchema( kpropFolder,          kcolFolder,              QString(),               QString("text"), QString("''"));

   m_foreignKeys[kpropRecipeId] = new PropertySchema( kpropRecipeId, kcolRecipeId, QString("integer"), Brewtarget::RECTABLE);

}

void TableSchema::defineWaterTable()
{
   m_type = BASE;
   m_className = QString("Water");
   m_childTable = Brewtarget::WATERCHILDTABLE;
   m_inRecTable = Brewtarget::WATERINRECTABLE;
   m_btTable    = Brewtarget::BT_WATERTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // These are defined in the global file.
   m_properties[kpropName]        = new PropertySchema( kpropName,        kcolName,             kxmlPropName,        QString("text"),    QString("''"), QString("not null"));
   m_properties[kpropNotes]       = new PropertySchema( kpropNotes,       kcolNotes,            kxmlPropNotes,       QString("text"),    QString("''"));
   m_properties[kpropAmount]      = new PropertySchema( kpropAmount,      kcolAmount,           kxmlPropAmount,      QString("real"),    QVariant(0.0));

   m_properties[kpropCalcium]     = new PropertySchema( kpropCalcium,     kcolWaterCalcium,     kxmlPropCalcium,     QString("real"),    QVariant(0.0));
   m_properties[kpropBiCarbonate] = new PropertySchema( kpropBiCarbonate, kcolWaterBiCarbonate, kxmlPropBiCarbonate, QString("real"),    QVariant(0.0));
   m_properties[kpropSulfate]     = new PropertySchema( kpropSulfate,     kcolWaterSulfate,     kxmlPropSulfate,     QString("real"),    QVariant(0.0));
   m_properties[kpropSodium]      = new PropertySchema( kpropSodium,      kcolWaterSodium,      kxmlPropSodium,      QString("real"),    QVariant(0.0));
   m_properties[kpropChloride]    = new PropertySchema( kpropChloride,    kcolWaterChloride,    kxmlPropChloride,    QString("real"),    QVariant(0.0));
   m_properties[kpropMagnesium]   = new PropertySchema( kpropMagnesium,   kcolWaterMagnesium,   kxmlPropMagnesium,   QString("real"),    QVariant(0.0));
   m_properties[kpropPH]          = new PropertySchema( kpropPH,          kcolPH,               kxmlPropPH,          QString("real"),    QVariant(0.0));
   m_properties[kpropAlkalinity]  = new PropertySchema( kpropAlkalinity,  kcolWaterAlkalinity,  QString(),           QString("real"),    QVariant(0.0));
   m_properties[kpropType]        = new PropertySchema( kpropType,        kcolWaterType,        QString(),           QString("int"),     QVariant(0));
   m_properties[kpropMashRO]      = new PropertySchema( kpropMashRO,      kcolWaterMashRO,      QString(),           QString("real"),    QVariant(0.0));
   m_properties[kpropSpargeRO]    = new PropertySchema( kpropSpargeRO,    kcolWaterSpargeRO,    QString(),           QString("real"),    QVariant(0.0));
   m_properties[kpropAsHCO3]      = new PropertySchema( kpropAsHCO3,      kcolWaterAsHCO3,      QString(),           QString("boolean"), QVariant(true));

   m_properties[kpropDisplay]     = new PropertySchema( kpropDisplay,     kcolDisplay,          QString(),           QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]     = new PropertySchema( kpropDeleted,     kcolDeleted,          QString(),           QString("boolean"), QVariant(false));
   m_properties[kpropFolder]      = new PropertySchema( kpropFolder,      kcolFolder,           QString(),           QString("text"),    QString("''"));

}

void TableSchema::defineSaltTable()
{
   m_type = BASE;
   m_className = QString("Salt");
   m_inRecTable = Brewtarget::SALTINRECTABLE;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // These are defined in the global file.
   m_properties[kpropName]     = new PropertySchema( kpropName,     kcolName,         QString(), QString("text"),    QString("''"), QString("not null"));
   m_properties[kpropAmount]   = new PropertySchema( kpropAmount,   kcolAmount,       QString(), QString("real"),    QVariant(0.0));
   m_properties[kpropAmtIsWgt] = new PropertySchema( kpropAmtIsWgt, kcolSaltAmtIsWgt, QString(), QString("boolean"), QVariant(true));
   m_properties[kpropPctAcid]  = new PropertySchema( kpropPctAcid,  kcolSaltPctAcid,  QString(), QString("real"),    QVariant(0.0));
   m_properties[kpropIsAcid]   = new PropertySchema( kpropIsAcid,   kcolSaltIsAcid,   QString(), QString("boolean"), QVariant(false));

   m_properties[kpropType]     = new PropertySchema( kpropType,    kcolSaltType,  QString(), QString("int"),     QVariant(0));
   m_properties[kpropAddTo]    = new PropertySchema( kpropAddTo,   kcolSaltAddTo, QString(), QString("int"),     QVariant(0));

   m_properties[kpropDisplay]  = new PropertySchema( kpropDisplay, kcolDisplay,   QString(), QString("boolean"), QVariant(true));
   m_properties[kpropDeleted]  = new PropertySchema( kpropDeleted, kcolDeleted,   QString(), QString("boolean"), QVariant(false));
   m_properties[kpropFolder]   = new PropertySchema( kpropFolder,  kcolFolder,    QString(), QString("text"),    QString("''"));

   m_foreignKeys[kpropMiscId]  = new PropertySchema( kpropMiscId, kcolMiscId, QString("integer"), Brewtarget::MISCTABLE);
}

void TableSchema::defineChildTable(Brewtarget::DBTable table)
{
   m_type = CHILD;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_foreignKeys[kpropChildId]  = new PropertySchema( kpropChildId,  kcolChildId,  QString("integer"), table);
   m_foreignKeys[kpropParentId] = new PropertySchema( kpropParentId, kcolParentId, QString("integer"), table);

}

void TableSchema::defineInRecipeTable(QString childIdx, Brewtarget::DBTable table)
{
   m_type = INREC;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_foreignKeys[kpropRecipeId] = new PropertySchema( kpropRecipeId, kcolRecipeId, QString("integer"), Brewtarget::RECTABLE);
   m_foreignKeys[childIdx]      = new PropertySchema( childIdx,      childIdx,     QString("integer"), table);

}

// instruction in rec has an extra field. I could have cheated, but we will try
// playing it straight first.
void TableSchema::defineInstructionInRecipeTable(QString childIdx, Brewtarget::DBTable table)
{
   m_type = INREC;
   m_trigger = kpropInstructionNumber;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // I am not breaking these rules any more. It makes it too annoying in the calling code to know when to use a kcol or kprop
   // so it is now kprop all the time
   m_properties[kpropInstructionNumber] = new PropertySchema( kpropInstructionNumber, kcolInstructionNumber, QString(""), QString("int"), QVariant(0));

   m_foreignKeys[kpropRecipeId] = new PropertySchema( kpropRecipeId, kcolRecipeId, QString("integer"), Brewtarget::RECTABLE);
   m_foreignKeys[childIdx]      = new PropertySchema( childIdx,      childIdx,     QString("integer"), table);

}

void TableSchema::defineBtTable(QString childIdx, Brewtarget::DBTable table)
{
   m_type = BT;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // What good is a rule followed to well?
   m_foreignKeys[childIdx] = new PropertySchema( childIdx, childIdx, QString("integer"), table);

}

void TableSchema::defineFermInventoryTable()
{
   m_type = INV;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropInventory]      = new PropertySchema( kpropInventory,     kcolAmount,        kxmlPropAmount, QString("real"), QVariant(0.0));
}

void TableSchema::defineHopInventoryTable()
{
   m_type = INV;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropInventory] = new PropertySchema( kpropInventory, kcolAmount, kxmlPropAmount, QString("real"), QVariant(0.0));
}

void TableSchema::defineMiscInventoryTable()
{
   m_type = INV;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropInventory] = new PropertySchema( kpropInventory, kcolAmount, kxmlPropAmount, QString("real"), QVariant(0.0));

}

void TableSchema::defineYeastInventoryTable()
{
   m_type = INV;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropInventory] = new PropertySchema( kpropQuanta,  kcolYeastQuanta, kxmlPropAmount, QString("real"), QVariant(0.0));

}

void TableSchema::defineSettingsTable()
{
   m_type = META;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[kpropSettingsVersion]    = new PropertySchema( QString(), kcolSettingsVersion,    QString(), QString("integer"), QVariant(0));
   m_properties[kpropSettingsRepopulate] = new PropertySchema( QString(), kcolSettingsRepopulate, QString(), QString("integer"), QVariant(0));
}
