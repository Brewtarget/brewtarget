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
#include "water.h"
#include "yeast.h"
#include "mashstep.h"
#include "mash.h"
#include "instruction.h"
#include "fermentable.h"
#include "equipment.h"
#include "style.h"

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

const QStringList TableSchema::allProperties() const
{
   return m_properties.keys();
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

const QStringList TableSchema::allForeignKeys() const 
{
   return m_foreignKeys.keys();
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
   QString retVal = QString("CREATE TABLE %1 (\n%2 %3\n")
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
         retVal.append( QString(",\n%1 %2 %3 %4 %5")
                           .arg( prop->colName() ).arg( prop->colType() )
                           .arg( prop->constraint() ).arg( kDefault ).arg( tmp )
         );
      }
      else {
         retVal.append( QString("%1 %2 %3,\n")
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

      retVal.append( QString(",\n%1 %2").arg( key->colName(selected) ).arg( key->colType(selected) ));

      retKeys.append( QString(",\nFOREIGN KEY(%1) REFERENCES %2(id)")
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
      binding += QString(",:%1").arg( i.key());
   }

   QMapIterator<QString, PropertySchema*> j(m_foreignKeys);
   while ( j.hasNext() ) {
      j.next();
      PropertySchema* key = j.value();

      columns += QString(",%1").arg(key->colName(selected));
      binding += QString(",:%1").arg( i.key());
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

      if ( prop->colName(selected) == keyName(selected) ) {
         continue;
      }

      if ( columns.isEmpty() ) {
         columns = QString("%1").arg(prop->colName(selected));
         binding = QString(":%1").arg(i.key());
      }
      else {
         columns += QString(",%1").arg(prop->colName(selected));
         binding += QString(",:%1").arg(i.key());
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
         columns += QString(",%1=:%2")
                        .arg( prop->colName(selected))
                        .arg( i.key());
      }
      else {
         columns = QString("%1=:%2")
                       .arg( prop->colName(selected))
                       .arg( i.key());
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
         columns += QString(",%1=:%2")
                        .arg( prop->colName(selected))
                        .arg( i.key());
      }
      else {
         columns = QString("%1=:%2")
                       .arg( prop->colName(selected))
                       .arg( i.key());
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

   m_properties[PropertyNames::NamedEntity::name]      = new PropertySchema( PropertyNames::NamedEntity::name,       PropertyNames::NamedEntity::name,         kxmlPropName,     QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Style::type]      = new PropertySchema( PropertyNames::Style::typeString, kcolStyleType,     kxmlPropType,     QString("text"), QString("'Ale'"));
   m_properties[PropertyNames::Style::category]       = new PropertySchema( PropertyNames::Style::category,        kcolStyleCat,      kxmlPropCat,      QString("text"), QString("''"));
   m_properties[PropertyNames::Style::categoryNumber]    = new PropertySchema( PropertyNames::Style::categoryNumber,     kcolStyleCatNum,   kxmlPropCatNum,   QString("text"), QString("''"));
   m_properties[PropertyNames::Style::styleLetter]    = new PropertySchema( PropertyNames::Style::styleLetter,     kcolStyleLetter,   kxmlPropLetter,   QString("text"), QString("''"));
   m_properties[PropertyNames::Style::styleGuide]     = new PropertySchema( PropertyNames::Style::styleGuide,      kcolStyleGuide,    kxmlPropGuide,    QString("text"), QString("''"));
   m_properties[PropertyNames::Style::ogMin]     = new PropertySchema( PropertyNames::Style::ogMin,      kcolStyleOGMin,    kxmlPropOGMin,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::ogMax]     = new PropertySchema( PropertyNames::Style::ogMax,      kcolStyleOGMax,    kxmlPropOGMax,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::fgMin]     = new PropertySchema( PropertyNames::Style::fgMin,      kcolStyleFGMin,    kxmlPropFGMin,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::fgMax]     = new PropertySchema( PropertyNames::Style::fgMax,      kcolStyleFGMax,    kxmlPropFGMax,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::ibuMin]    = new PropertySchema( PropertyNames::Style::ibuMin,     kcolStyleIBUMin,   kxmlPropIBUMin,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::ibuMax]    = new PropertySchema( PropertyNames::Style::ibuMax,     kcolStyleIBUMax,   kxmlPropIBUMax,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::colorMin_srm]  = new PropertySchema( PropertyNames::Style::colorMin_srm,   kcolStyleColorMin, kxmlPropColorMin, QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::colorMax_srm]  = new PropertySchema( PropertyNames::Style::colorMax_srm,   kcolStyleColorMax, kxmlPropColorMax, QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::abvMin_pct]    = new PropertySchema( PropertyNames::Style::abvMin_pct,     kcolStyleABVMin,   kxmlPropABVMin,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::abvMax_pct]    = new PropertySchema( PropertyNames::Style::abvMax_pct,     kcolStyleABVMax,   kxmlPropABVMax,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::carbMin_vol]   = new PropertySchema( PropertyNames::Style::carbMin_vol,    kcolStyleCarbMin,  kxmlPropCarbMin,  QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::carbMax_vol]   = new PropertySchema( PropertyNames::Style::carbMax_vol,    kcolStyleCarbMax,  kxmlPropCarbMax,  QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Style::notes]     = new PropertySchema( PropertyNames::Style::notes,      kcolNotes,         kxmlPropNotes,    QString("text"), QString("''"));
   m_properties[PropertyNames::Style::profile]   = new PropertySchema( PropertyNames::Style::profile,    kcolStyleProfile,  kxmlPropProfile,  QString("text"), QString("''"));
   m_properties[PropertyNames::Style::ingredients]   = new PropertySchema( PropertyNames::Style::ingredients,    kcolStyleIngreds,  kxmlPropIngreds,  QString("text"), QString("''"));
   m_properties[PropertyNames::Style::examples]  = new PropertySchema( PropertyNames::Style::examples,   kcolStyleExamples, kxmlPropExamples, QString("text"), QString("''"));

   // not sure about these, but I think I'm gonna need them anyway
   m_properties[PropertyNames::NamedEntity::display]   = new PropertySchema(PropertyNames::NamedEntity::display,   kcolDisplay,       QString(),        QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]   = new PropertySchema(PropertyNames::NamedEntity::deleted,   kcolDeleted,       QString(),        QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]    = new PropertySchema(PropertyNames::NamedEntity::folder,    kcolFolder,        QString(),        QString("text"),    QString("''"));
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

   m_properties[PropertyNames::NamedEntity::name]          = new PropertySchema( PropertyNames::NamedEntity::name,          PropertyNames::NamedEntity::name,         kxmlPropName,            QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Equipment::boilSize_l]      = new PropertySchema( PropertyNames::Equipment::boilSize_l,      kcolEquipBoilSize,      kxmlPropBoilSize,        QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::batchSize_l]     = new PropertySchema( PropertyNames::Equipment::batchSize_l,     kcolEquipBatchSize,     kxmlPropBatchSize,       QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::tunVolume_l]     = new PropertySchema( PropertyNames::Equipment::tunVolume_l,     kcolEquipTunVolume,     kxmlPropTunVolume,       QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::tunWeight_kg]     = new PropertySchema( PropertyNames::Equipment::tunWeight_kg,     kcolEquipTunWeight,     kxmlPropTunWeight,       QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::tunSpecificHeat_calGC]   = new PropertySchema( PropertyNames::Equipment::tunSpecificHeat_calGC,   kcolEquipTunSpecHeat,   kxmlPropTunSpecHeat,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::topUpWater_l]    = new PropertySchema( PropertyNames::Equipment::topUpWater_l,    kcolEquipTopUpWater,    kxmlPropTopUpWater,      QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::trubChillerLoss_l] = new PropertySchema( PropertyNames::Equipment::trubChillerLoss_l, kcolEquipTrubChillLoss, kxmlPropTrubChillLoss,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::evapRate_pctHr]      = new PropertySchema( PropertyNames::Equipment::evapRate_pctHr,      kcolEquipEvapRate,      kxmlPropEvapRate,        QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::boilTime_min]      = new PropertySchema( PropertyNames::Equipment::boilTime_min,      kcolEquipBoilTime,      kxmlPropBoilTime,        QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::calcBoilVolume]   = new PropertySchema( PropertyNames::Equipment::calcBoilVolume,   kcolEquipCalcBoilVol,   kxmlPropCalcBoilVol,     QString("boolean"), QVariant(false));
   m_properties[PropertyNames::Equipment::lauterDeadspace_l]   = new PropertySchema( PropertyNames::Equipment::lauterDeadspace_l,   kcolEquipLauterSpace,   kxmlPropLauterSpace,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::topUpKettle_l]   = new PropertySchema( PropertyNames::Equipment::topUpKettle_l,   kcolEquipTopUpKettle,   kxmlPropTopUpKettle,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::hopUtilization_pct]       = new PropertySchema( PropertyNames::Equipment::hopUtilization_pct,       kcolEquipHopUtil,       kxmlPropHopUtil,         QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::notes]         = new PropertySchema( PropertyNames::Equipment::notes,         kcolNotes,              kxmlPropNotes,           QString("text"), QString("''"));
   m_properties[PropertyNames::Equipment::evapRate_lHr]  = new PropertySchema( PropertyNames::Equipment::evapRate_lHr,  kcolEquipRealEvapRate,  kxmlPropRealEvapRate,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Equipment::boilingPoint_c]  = new PropertySchema( PropertyNames::Equipment::boilingPoint_c,  kcolEquipBoilingPoint,  kxmlPropBoilingPoint,    QString("real"), QVariant(100.0));
   m_properties[PropertyNames::Equipment::grainAbsorption_LKg]    = new PropertySchema( PropertyNames::Equipment::grainAbsorption_LKg,    kcolEquipAbsorption,    kxmlPropGrainAbsorption, QString("real"), QVariant(1.085));

   m_properties[PropertyNames::NamedEntity::display]       = new PropertySchema( PropertyNames::NamedEntity::display,       kcolDisplay,       QString(),               QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]       = new PropertySchema( PropertyNames::NamedEntity::deleted,       kcolDeleted,       QString(),               QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]        = new PropertySchema( PropertyNames::NamedEntity::folder,        kcolFolder,       QString(),               QString("text"), QString("''"));

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

   m_properties[PropertyNames::NamedEntity::name]           = new PropertySchema( PropertyNames::NamedEntity::name,           kcolName,               kxmlPropName,           QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Fermentable::notes]          = new PropertySchema( PropertyNames::Fermentable::notes,          kcolNotes,              kxmlPropNotes,          QString("text"), QString("''"));
   m_properties[PropertyNames::Fermentable::type]           = new PropertySchema( PropertyNames::Fermentable::typeString,     kcolFermType,           kxmlPropType,           QString("text"), QString("'Grain'"));
   m_properties[PropertyNames::Fermentable::amount_kg]       = new PropertySchema( PropertyNames::Fermentable::amount_kg,       kcolAmount,             kxmlPropAmount,         QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Fermentable::yield_pct]          = new PropertySchema( PropertyNames::Fermentable::yield_pct,          kcolFermYield,          kxmlPropYield,          QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Fermentable::color_srm]          = new PropertySchema( PropertyNames::Fermentable::color_srm,          kcolFermColor,          kxmlPropColor,          QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Fermentable::addAfterBoil]   = new PropertySchema( PropertyNames::Fermentable::addAfterBoil,   kcolFermAddAfterBoil,   kxmlPropAddAfterBoil,   QString("boolean"), QVariant(false));
   m_properties[PropertyNames::Fermentable::origin]         = new PropertySchema( PropertyNames::Fermentable::origin,         kcolFermOrigin,         kxmlPropOrigin,         QString("text"), QString("''"));
   m_properties[PropertyNames::Fermentable::supplier]       = new PropertySchema( PropertyNames::Fermentable::supplier,       kcolFermSupplier,       kxmlPropSupplier,       QString("text"), QString("''"));
   m_properties[PropertyNames::Fermentable::coarseFineDiff_pct] = new PropertySchema( PropertyNames::Fermentable::coarseFineDiff_pct, kcolFermCoarseFineDiff, kxmlPropCoarseFineDiff, QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Fermentable::moisture_pct]       = new PropertySchema( PropertyNames::Fermentable::moisture_pct,       kcolFermMoisture,       kxmlPropMoisture,       QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Fermentable::diastaticPower_lintner] = new PropertySchema( PropertyNames::Fermentable::diastaticPower_lintner, kcolFermDiastaticPower, kxmlPropDiastaticPower, QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Fermentable::protein_pct]        = new PropertySchema( PropertyNames::Fermentable::protein_pct,        kcolFermProtein,        kxmlPropProtein,        QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Fermentable::maxInBatch_pct]     = new PropertySchema( PropertyNames::Fermentable::maxInBatch_pct,     kcolFermMaxInBatch,     kxmlPropMaxInBatch,     QString("real"), QVariant(100.0));
   m_properties[PropertyNames::Fermentable::recommendMash]  = new PropertySchema( PropertyNames::Fermentable::recommendMash,  kcolFermRecommendMash,  kxmlPropRecommendMash,  QString("boolean"), QVariant(false));
   m_properties[PropertyNames::Fermentable::isMashed]       = new PropertySchema( PropertyNames::Fermentable::isMashed,       kcolFermIsMashed,       kxmlPropIsMashed,       QString("boolean"), QVariant(false));
   m_properties[PropertyNames::Fermentable::ibuGalPerLb]    = new PropertySchema( PropertyNames::Fermentable::ibuGalPerLb,    kcolFermIBUGalPerLb,    kxmlPropIBUGalPerLb,    QString("real"), QVariant(0.0));

   m_properties[PropertyNames::NamedEntity::display]        = new PropertySchema( PropertyNames::NamedEntity::display,        kcolDisplay,            QString(),              QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]        = new PropertySchema( PropertyNames::NamedEntity::deleted,        kcolDeleted,            QString(),              QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]         = new PropertySchema( PropertyNames::NamedEntity::folder,         kcolFolder,             QString(),              QString("text"), QString("''"));

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
   m_properties[PropertyNames::NamedEntity::name]          = new PropertySchema( PropertyNames::NamedEntity::name,          kcolName,             kxmlPropName,          QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Hop::notes]         = new PropertySchema( PropertyNames::Hop::notes,         kcolNotes,            kxmlPropNotes,         QString("text"), QString("''"));
   m_properties[PropertyNames::Hop::amount_kg]      = new PropertySchema( PropertyNames::Hop::amount_kg,      kcolAmount,           kxmlPropAmount,        QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Hop::use]           = new PropertySchema( PropertyNames::Hop::useString,     kcolUse,              kxmlPropUse,           QString("text"), QString("'Boil'"));
   m_properties[PropertyNames::Hop::time_min]          = new PropertySchema( PropertyNames::Hop::time_min,          kcolTime,             kxmlPropTime,          QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Hop::origin]        = new PropertySchema( PropertyNames::Hop::origin,        kcolOrigin,           kxmlPropOrigin,        QString("text"), QString("''"));
   m_properties[PropertyNames::Hop::substitutes]   = new PropertySchema( PropertyNames::Hop::substitutes,   kcolSubstitutes,      kxmlPropSubstitutes,   QString("text"), QString("''"));
   m_properties[PropertyNames::Hop::alpha_pct]         = new PropertySchema( PropertyNames::Hop::alpha_pct,         kcolHopAlpha,         kxmlPropAlpha,         QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Hop::type]          = new PropertySchema( PropertyNames::Hop::typeString,    kcolHopType,          kxmlPropType,          QString("text"), QString("'Boil'"));
   m_properties[PropertyNames::Hop::form]          = new PropertySchema( PropertyNames::Hop::formString,    kcolHopForm,          kxmlPropForm,          QString("text"), QString("'Pellet'"));
   m_properties[PropertyNames::Hop::beta_pct]          = new PropertySchema( PropertyNames::Hop::beta_pct,          kcolHopBeta,          kxmlPropBeta,          QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Hop::hsi_pct]           = new PropertySchema( PropertyNames::Hop::hsi_pct,           kcolHopHSI,           kxmlPropHSI,           QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Hop::humulene_pct]      = new PropertySchema( PropertyNames::Hop::humulene_pct,      kcolHopHumulene,      kxmlPropHumulene,      QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Hop::caryophyllene_pct] = new PropertySchema( PropertyNames::Hop::caryophyllene_pct, kcolHopCaryophyllene, kxmlPropCaryophyllene, QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Hop::cohumulone_pct]    = new PropertySchema( PropertyNames::Hop::cohumulone_pct,    kcolHopCohumulone,    kxmlPropCohumulone,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Hop::myrcene_pct]       = new PropertySchema( PropertyNames::Hop::myrcene_pct,       kcolHopMyrcene,       kxmlPropMyrcene,       QString("real"), QVariant(0.0));

   m_properties[PropertyNames::NamedEntity::display]       = new PropertySchema( PropertyNames::NamedEntity::display,       kcolDisplay,          QString(),             QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]       = new PropertySchema( PropertyNames::NamedEntity::deleted,       kcolDeleted,          QString(),             QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]        = new PropertySchema( PropertyNames::NamedEntity::folder,        kcolFolder,           QString(),             QString("text"), QString("''"));

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
   m_properties[PropertyNames::NamedEntity::name]          = new PropertySchema( PropertyNames::NamedEntity::name,          kcolName,                  kxmlPropName,       QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Instruction::directions]    = new PropertySchema( PropertyNames::Instruction::directions,    kcolInstructionDirections, kxmlPropDirections, QString("text"), QString("''"));
   m_properties[PropertyNames::Instruction::hasTimer]      = new PropertySchema( PropertyNames::Instruction::hasTimer,      kcolInstructionHasTimer,   kxmlPropHasTimer,   QString("boolean"), QVariant(false));
   m_properties[PropertyNames::Instruction::timerValue]    = new PropertySchema( PropertyNames::Instruction::timerValue,    kcolInstructionTimerValue, kxmlPropTimerValue, QString("text"), QVariant("'00:00:00'"));
   m_properties[PropertyNames::Instruction::completed]     = new PropertySchema( PropertyNames::Instruction::completed,     kcolInstructionCompleted,  kxmlPropCompleted,  QString("boolean"), QVariant(false));
   m_properties[PropertyNames::Instruction::interval]      = new PropertySchema( PropertyNames::Instruction::interval,      kcolInstructionInterval,   kxmlPropInterval,   QString("real"), QVariant(0.0));

   m_properties[PropertyNames::NamedEntity::display]       = new PropertySchema( PropertyNames::NamedEntity::display,       kcolDisplay,               QString(),          QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]       = new PropertySchema( PropertyNames::NamedEntity::deleted,       kcolDeleted,               QString(),          QString("boolean"), QVariant(false));
}

void TableSchema::defineMashTable()
{
   m_type = BASE;
   m_className = QString("Mash");

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   // These are defined in the global file.
   m_properties[PropertyNames::NamedEntity::name]        = new PropertySchema( PropertyNames::NamedEntity::name,        kcolName,            kxmlPropName,        QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Mash::notes]       = new PropertySchema( PropertyNames::Mash::notes,       kcolNotes,           kxmlPropNotes,       QString("text"), QString("''"));
   m_properties[PropertyNames::Mash::grainTemp_c]   = new PropertySchema( PropertyNames::Mash::grainTemp_c,   kcolMashGrainTemp,   kxmlPropGrainTemp,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Mash::tunTemp_c]     = new PropertySchema( PropertyNames::Mash::tunTemp_c,     kcolMashTunTemp,     kxmlPropTunTemp,     QString("real"), QVariant(20.0));
   m_properties[PropertyNames::Mash::spargeTemp_c]  = new PropertySchema( PropertyNames::Mash::spargeTemp_c,  kcolMashSpargeTemp,  kxmlPropSpargeTemp,  QString("real"), QVariant(74.0));
   m_properties[PropertyNames::Mash::ph]          = new PropertySchema( PropertyNames::Mash::ph,          kcolPH,              kxmlPropPH,          QString("real"), QVariant(7.0));
   m_properties[PropertyNames::Mash::tunWeight_kg]   = new PropertySchema( PropertyNames::Mash::tunWeight_kg,   kcolMashTunWeight,   kxmlPropTunWeight,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Mash::tunSpecificHeat_calGC] = new PropertySchema( PropertyNames::Mash::tunSpecificHeat_calGC, kcolMashTunSpecHeat, kxmlPropTunSpecHeat, QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Mash::equipAdjust] = new PropertySchema( PropertyNames::Mash::equipAdjust, kcolMashEquipAdjust, kxmlPropEquipAdjust, QString("boolean"), QVariant(true));

   m_properties[PropertyNames::NamedEntity::display]     = new PropertySchema( PropertyNames::NamedEntity::display,     kcolDisplay,         QString(),           QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]     = new PropertySchema( PropertyNames::NamedEntity::deleted,     kcolDeleted,         QString(),           QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]      = new PropertySchema( PropertyNames::NamedEntity::folder,      kcolFolder,         QString(),           QString("text"), QString("''"));
}

// property name, column name, xml property name, column type, column default, column constraint
void TableSchema::defineMashstepTable()
{
   m_type = BASE;
   m_className = QString("MashStep");

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[PropertyNames::NamedEntity::name]       = new PropertySchema( PropertyNames::NamedEntity::name,       kcolName,               kxmlPropName,       QString("text"), QString("''"),QString("not null"));
   m_properties[PropertyNames::MashStep::type]       = new PropertySchema( PropertyNames::MashStep::typeString, kcolMashstepType,       kxmlPropType,       QString("text"), QString("'Infusion'"));
   m_properties[PropertyNames::MashStep::infuseAmount_l]  = new PropertySchema( PropertyNames::MashStep::infuseAmount_l,  kcolMashstepInfuseAmt,  kxmlPropInfuseAmt,  QString("real"), QVariant(0.0));
   m_properties[PropertyNames::MashStep::stepTemp_c]   = new PropertySchema( PropertyNames::MashStep::stepTemp_c,   kcolMashstepStepTemp,   kxmlPropStepTemp,   QString("real"), QVariant(67.0));
   m_properties[PropertyNames::MashStep::stepTime_min]   = new PropertySchema( PropertyNames::MashStep::stepTime_min,   kcolMashstepStepTime,   kxmlPropStepTime,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::MashStep::rampTime_min]   = new PropertySchema( PropertyNames::MashStep::rampTime_min,   kcolMashstepRampTime,   kxmlPropRampTime,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::MashStep::endTemp_c]    = new PropertySchema( PropertyNames::MashStep::endTemp_c,    kcolMashstepEndTemp,    kxmlPropEndTemp,    QString("real"), QVariant(67.0));
   m_properties[PropertyNames::MashStep::infuseTemp_c] = new PropertySchema( PropertyNames::MashStep::infuseTemp_c, kcolMashstepInfuseTemp, kxmlPropInfuseTemp, QString("real"), QVariant(67.0));
   m_properties[PropertyNames::MashStep::decoctionAmount_l]  = new PropertySchema( PropertyNames::MashStep::decoctionAmount_l,  kcolMashstepDecoctAmt,  kxmlPropDecoctAmt,  QString("real"), QVariant(67.0));
   m_properties[PropertyNames::MashStep::stepNumber] = new PropertySchema( PropertyNames::MashStep::stepNumber, kcolMashstepStepNumber, QString(),          QString("integer"), QVariant(0));

   m_properties[PropertyNames::NamedEntity::display]    = new PropertySchema( PropertyNames::NamedEntity::display,    kcolDisplay,            QString(),          QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]    = new PropertySchema( PropertyNames::NamedEntity::deleted,    kcolDeleted,            QString(),          QString("boolean"), QVariant(false));

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
   m_properties[PropertyNames::NamedEntity::name]     = new PropertySchema( PropertyNames::NamedEntity::name,       kcolName,         kxmlPropName,     QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Misc::notes]    = new PropertySchema( PropertyNames::Misc::notes,      kcolNotes,        kxmlPropNotes,    QString("text"), QString("''"));
   m_properties[PropertyNames::Misc::amount]   = new PropertySchema( PropertyNames::Misc::amount,     kcolAmount,       kxmlPropAmount,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Misc::use]      = new PropertySchema( PropertyNames::Misc::useString,  kcolUse,          kxmlPropUse,      QString("text"), QString("'Boil'"));
   m_properties[PropertyNames::Hop::time_min]     = new PropertySchema( PropertyNames::Hop::time_min,       kcolTime,         kxmlPropTime,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Misc::type]     = new PropertySchema( PropertyNames::Misc::typeString, kcolMiscType,     kxmlPropType,     QString("text"), QString("'Other'"));
   m_properties[PropertyNames::Misc::amountIsWeight] = new PropertySchema( PropertyNames::Misc::amountIsWeight,   kcolMiscAmtIsWgt, kxmlPropAmtIsWgt, QString("boolean"), QVariant(true));
   m_properties[PropertyNames::Misc::useFor]   = new PropertySchema( PropertyNames::Misc::useFor,     kcolMiscUseFor,   kxmlPropUseFor,   QString("text"), QString("''"));

   m_properties[PropertyNames::NamedEntity::display]  = new PropertySchema( PropertyNames::NamedEntity::display,  kcolDisplay,      QString(),        QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]  = new PropertySchema( PropertyNames::NamedEntity::deleted,  kcolDeleted,      QString(),        QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]   = new PropertySchema( PropertyNames::NamedEntity::folder,   kcolFolder,      QString(),        QString("text"), QString("''"));

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

   m_properties[PropertyNames::NamedEntity::name]        = new PropertySchema( PropertyNames::NamedEntity::name,        kcolName,               kxmlPropName,         QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Recipe::notes]       = new PropertySchema( PropertyNames::Recipe::notes,       kcolNotes,              kxmlPropNotes,        QString("text"), QString("''"));
   m_properties[PropertyNames::Recipe::type]        = new PropertySchema( PropertyNames::Recipe::type,        kcolRecipeType,         kxmlPropType,         QString("text"), QString("'All Grain'"));
   m_properties[PropertyNames::Recipe::brewer]      = new PropertySchema( PropertyNames::Recipe::brewer,      kcolRecipeBrewer,       kxmlPropBrewer,       QString("text"), QString("''"));
   m_properties[PropertyNames::Recipe::asstBrewer]  = new PropertySchema( PropertyNames::Recipe::asstBrewer,  kcolRecipeAsstBrewer,   kxmlPropAsstBrewer,   QString("text"), QString("'Brewtarget'"));
   m_properties[PropertyNames::Recipe::batchSize_l]   = new PropertySchema( PropertyNames::Recipe::batchSize_l,   kcolRecipeBatchSize,    kxmlPropBatchSize,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Recipe::boilSize_l]    = new PropertySchema( PropertyNames::Recipe::boilSize_l,    kcolRecipeBoilSize,     kxmlPropBoilSize,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Recipe::boilTime_min]    = new PropertySchema( PropertyNames::Recipe::boilTime_min,    kcolRecipeBoilTime,     kxmlPropBoilTime,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Recipe::efficiency_pct]      = new PropertySchema( PropertyNames::Recipe::efficiency_pct,      kcolRecipeEff,          kxmlPropEff,          QString("real"), QVariant(70.0));
   m_properties[PropertyNames::Recipe::og]          = new PropertySchema( PropertyNames::Recipe::og,          kcolRecipeOG,           kxmlPropOG,           QString("real"), QVariant(1.0));
   m_properties[PropertyNames::Recipe::fg]          = new PropertySchema( PropertyNames::Recipe::fg,          kcolRecipeFG,           kxmlPropFG,           QString("real"), QVariant(1.0));
   m_properties[PropertyNames::Recipe::fermentationStages]  = new PropertySchema( PropertyNames::Recipe::fermentationStages,  kcolRecipeFermStages,   kxmlPropFermStages,   QString("int"), QVariant(0));
   m_properties[PropertyNames::Recipe::primaryAge_days] = new PropertySchema( PropertyNames::Recipe::primaryAge_days, kcolRecipePrimAgeDays,  kxmlPropPrimAgeDays,  QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Recipe::primaryTemp_c]    = new PropertySchema( PropertyNames::Recipe::primaryTemp_c,    kcolRecipePrimTemp,     kxmlPropPrimTemp,     QString("real"), QVariant(20.0));
   m_properties[PropertyNames::Recipe::secondaryAge_days]  = new PropertySchema( PropertyNames::Recipe::secondaryAge_days,  kcolRecipeSecAgeDays,   kxmlPropSecAgeDays,   QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Recipe::secondaryTemp_c]     = new PropertySchema( PropertyNames::Recipe::secondaryTemp_c,     kcolRecipeSecTemp,      kxmlPropSecTemp,      QString("real"), QVariant(20.0));
   m_properties[PropertyNames::Recipe::tertiaryAge_days] = new PropertySchema( PropertyNames::Recipe::tertiaryAge_days, kcolRecipeTertAgeDays,  kxmlPropTertAgeDays,  QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Recipe::tertiaryTemp_c]    = new PropertySchema( PropertyNames::Recipe::tertiaryTemp_c,    kcolRecipeTertTemp,     kxmlPropTertTemp,     QString("real"), QVariant(20.0));
   m_properties[PropertyNames::Recipe::age]         = new PropertySchema( PropertyNames::Recipe::age,         kcolRecipeAge,          kxmlPropAge,          QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Recipe::ageTemp_c]     = new PropertySchema( PropertyNames::Recipe::ageTemp_c,     kcolRecipeAgeTemp,      kxmlPropAgeTemp,      QString("real"), QVariant(20.0));
   m_properties[PropertyNames::Recipe::date]        = new PropertySchema( PropertyNames::Recipe::date,        kcolRecipeDate,         kxmlPropDate,         QString(PropertyNames::Recipe::date), QString("CURRENT_TIMESTAMP"));
   m_properties[PropertyNames::Recipe::carbonation_vols]    = new PropertySchema( PropertyNames::Recipe::carbonation_vols,    kcolRecipeCarbVols,     kxmlPropCarbVols,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Recipe::forcedCarbonation]  = new PropertySchema( PropertyNames::Recipe::forcedCarbonation,  kcolRecipeForcedCarb,   kxmlPropForcedCarb,   QString("boolean"), QVariant(false));
   m_properties[PropertyNames::Recipe::primingSugarName] = new PropertySchema( PropertyNames::Recipe::primingSugarName, kcolRecipePrimSugName,  kxmlPropPrimSugName,  QString("text"), QString("''"));
   m_properties[PropertyNames::Recipe::carbonationTemp_c]    = new PropertySchema( PropertyNames::Recipe::carbonationTemp_c,    kcolRecipeCarbTemp,     kxmlPropCarbTemp,     QString("real"), QVariant(20.0));
   m_properties[PropertyNames::Recipe::primingSugarEquiv]= new PropertySchema( PropertyNames::Recipe::primingSugarEquiv,kcolRecipePrimSugEquiv, kxmlPropPrimSugEquiv, QString("real"), QVariant(1.0));
   m_properties[PropertyNames::Recipe::kegPrimingFactor] = new PropertySchema( PropertyNames::Recipe::kegPrimingFactor, kcolRecipeKegPrimFact,  kxmlPropKegPrimFact,  QString("real"), QVariant(1.0));
   m_properties[PropertyNames::Recipe::tasteNotes]  = new PropertySchema( PropertyNames::Recipe::tasteNotes,  kcolRecipeTasteNotes,   kxmlPropTasteNotes,   QString("text"), QString("''"));
   m_properties[PropertyNames::Recipe::tasteRating] = new PropertySchema( PropertyNames::Recipe::tasteRating, kcolRecipeTasteRating,  kxmlPropTasteRating,  QString("real"), QVariant(20.0));

   m_properties[PropertyNames::NamedEntity::display]     = new PropertySchema( PropertyNames::NamedEntity::display,     kcolDisplay,            QString(),            QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]     = new PropertySchema( PropertyNames::NamedEntity::deleted,     kcolDeleted,            QString(),            QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]      = new PropertySchema( PropertyNames::NamedEntity::folder,      kcolFolder,            QString(),            QString("text"), QString("''"));
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
   m_properties[PropertyNames::NamedEntity::name]       = new PropertySchema( PropertyNames::NamedEntity::name,       kcolName,            kxmlPropName,       QString("text"), QString("''"), QString("not null"));
   m_properties[PropertyNames::Yeast::notes]      = new PropertySchema( PropertyNames::Yeast::notes,      kcolNotes,           kxmlPropNotes,      QString("text"), QString("''"));
   m_properties[PropertyNames::Yeast::type]       = new PropertySchema( PropertyNames::Yeast::typeString, kcolYeastType,       kxmlPropType,       QString("text"), QObject::tr("'Ale'"));
   m_properties[PropertyNames::Yeast::form]       = new PropertySchema( PropertyNames::Yeast::formString, kcolYeastForm,       kxmlPropForm,       QString("text"), QObject::tr("'Liquid'"));
   m_properties[PropertyNames::Yeast::amount]     = new PropertySchema( PropertyNames::Yeast::amount,     kcolYeastAmount,     kxmlPropAmount,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Yeast::amountIsWeight]   = new PropertySchema( PropertyNames::Yeast::amountIsWeight,   kcolYeastAmtIsWgt,   kxmlPropAmtIsWgt,   QString("boolean"), QVariant(false));
   m_properties[PropertyNames::Yeast::laboratory]        = new PropertySchema( PropertyNames::Yeast::laboratory,        kcolYeastLab,        kxmlPropLab,        QString("text"), QString("''"));
   m_properties[PropertyNames::Yeast::productID]  = new PropertySchema( PropertyNames::Yeast::productID,  kcolYeastProductID,  kxmlPropProductID,  QString("text"), QString("''"));
   m_properties[PropertyNames::Yeast::minTemperature_c]    = new PropertySchema( PropertyNames::Yeast::minTemperature_c,    kcolYeastMinTemp,    kxmlPropMinTemp,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Yeast::maxTemperature_c]    = new PropertySchema( PropertyNames::Yeast::maxTemperature_c,    kcolYeastMaxTemp,    kxmlPropMaxTemp,    QString("real"), QVariant(0.0));
   m_properties[PropertyNames::Yeast::flocculation]       = new PropertySchema( PropertyNames::Yeast::flocculationString, kcolYeastFloc,       kxmlPropFloc,       QString("text"), QObject::tr("'Medium'"));
   m_properties[PropertyNames::Yeast::attenuation_pct]   = new PropertySchema( PropertyNames::Yeast::attenuation_pct,   kcolYeastAtten,      kxmlPropAtten,      QString("real"), QVariant(75.0));
   m_properties[PropertyNames::Yeast::bestFor]    = new PropertySchema( PropertyNames::Yeast::bestFor,    kcolYeastBestFor,    kxmlPropBestFor,    QString("text"), QString("''"));
   m_properties[PropertyNames::Yeast::timesCultured] = new PropertySchema( PropertyNames::Yeast::timesCultured, kcolYeastTimesCultd, kxmlPropTimesCultd, QString("int"), QVariant(0));
   m_properties[PropertyNames::Yeast::maxReuse]   = new PropertySchema( PropertyNames::Yeast::maxReuse,   kcolYeastMaxReuse,   kxmlPropMaxReuse,   QString("int"), QVariant(10));
   m_properties[PropertyNames::Yeast::addToSecondary]   = new PropertySchema( PropertyNames::Yeast::addToSecondary,   kcolYeastAddToSec,   kxmlPropAddToSec,   QString("boolean"), QVariant(false));

   m_properties[PropertyNames::NamedEntity::display]    = new PropertySchema( PropertyNames::NamedEntity::display,    kcolDisplay,         QString(),          QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]    = new PropertySchema( PropertyNames::NamedEntity::deleted,    kcolDeleted,         QString(),          QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]     = new PropertySchema( PropertyNames::NamedEntity::folder,     kcolFolder,          QString(),          QString("text"),    QString("''"));

   m_foreignKeys[kpropInventoryId]  = new PropertySchema( kpropInventoryId,   kcolInventoryId,      QString("integer"),    m_invTable);
}

void TableSchema::defineBrewnoteTable()
{
   m_type = BASE;
   m_className = QString("BrewNote");

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[PropertyNames::BrewNote::notes]           = new PropertySchema( PropertyNames::BrewNote::notes,           kcolNotes,                kxmlPropNotes,           QString("text"),    QString("''"));

   m_properties[PropertyNames::BrewNote::brewDate]        = new PropertySchema( PropertyNames::BrewNote::brewDate,        kcolBNoteBrewDate,        kxmlPropBrewDate,        QString("timestamp"), QString("CURRENT_TIMESTAMP"));
   m_properties[PropertyNames::BrewNote::fermentDate]        = new PropertySchema( PropertyNames::BrewNote::fermentDate,        kcolBNoteFermDate,        kxmlPropFermDate,        QString("timestamp"), QString("CURRENT_TIMESTAMP"));
   m_properties[PropertyNames::BrewNote::sg]              = new PropertySchema( PropertyNames::BrewNote::sg,              kcolBNoteSG,              kxmlPropSG,              QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::volumeIntoBK_l]     = new PropertySchema( PropertyNames::BrewNote::volumeIntoBK_l,     kcolBNoteVolIntoBoil,     kxmlPropVolIntoBoil,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::BrewNote::strikeTemp_c]      = new PropertySchema( PropertyNames::BrewNote::strikeTemp_c,      kcolBNoteStrikeTemp,      kxmlPropStrikeTemp,      QString("real"), QVariant(70.0));
   m_properties[PropertyNames::BrewNote::mashFinTemp_c]     = new PropertySchema( PropertyNames::BrewNote::mashFinTemp_c,     kcolBNoteMashFinTemp,     kxmlPropMashFinTemp,     QString("real"), QVariant(67.0));
   m_properties[PropertyNames::BrewNote::og]              = new PropertySchema( PropertyNames::BrewNote::og,              kcolBNoteOG,              kxmlPropOG,              QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::postBoilVolume_l]     = new PropertySchema( PropertyNames::BrewNote::postBoilVolume_l,     kcolBNotePostBoilVol,     kxmlPropPostBoilVol,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::BrewNote::volumeIntoFerm_l]     = new PropertySchema( PropertyNames::BrewNote::volumeIntoFerm_l,     kcolBNoteVolIntoFerm,     kxmlPropVolIntoFerm,     QString("real"), QVariant(0.0));
   m_properties[PropertyNames::BrewNote::pitchTemp_c]       = new PropertySchema( PropertyNames::BrewNote::pitchTemp_c,       kcolBNotePitchTemp,       kxmlPropPitchTemp,       QString("real"), QVariant(20.0));
   m_properties[PropertyNames::BrewNote::fg]              = new PropertySchema( PropertyNames::BrewNote::fg,              kcolBNoteFG,              kxmlPropFG,              QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::effIntoBK_pct]     = new PropertySchema( PropertyNames::BrewNote::effIntoBK_pct,     kcolBNoteEffIntoBoil,     kxmlPropEffIntoBoil,     QString("real"), QVariant(70.0));
   m_properties[PropertyNames::BrewNote::abv]             = new PropertySchema( PropertyNames::BrewNote::abv,             kcolBNoteABV,             kxmlPropABV,             QString("real"), QVariant(0.0));
   m_properties[PropertyNames::BrewNote::projOg]          = new PropertySchema( PropertyNames::BrewNote::projOg,          kcolBNoteProjOG,          kxmlPropProjOG,          QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::brewhouseEff_pct]       = new PropertySchema( PropertyNames::BrewNote::brewhouseEff_pct,       kcolBNoteBrewhsEff,       kxmlPropBrewhsEff,       QString("real"), QVariant(70.0));
   m_properties[PropertyNames::BrewNote::projBoilGrav]    = new PropertySchema( PropertyNames::BrewNote::projBoilGrav,    kcolBNoteProjBoilGrav,    kxmlPropProjBoilGrav,    QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::projStrikeTemp_c]  = new PropertySchema( PropertyNames::BrewNote::projStrikeTemp_c,  kcolBNoteProjStrikeTemp,  kxmlPropProjStrikeTemp,  QString("real"), QVariant(70.0));
   m_properties[PropertyNames::BrewNote::projMashFinTemp_c] = new PropertySchema( PropertyNames::BrewNote::projMashFinTemp_c, kcolBNoteProjMashFinTemp, kxmlPropProjMashFinTemp, QString("real"), QVariant(67.0));
   m_properties[PropertyNames::BrewNote::projVolIntoBK_l] = new PropertySchema( PropertyNames::BrewNote::projVolIntoBK_l, kcolBNoteProjVolIntoBoil, kxmlPropProjVolIntoBoil, QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::projOg]          = new PropertySchema( PropertyNames::BrewNote::projOg,          kcolBNoteProjOG,          kxmlPropProjOG,          QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::projVolIntoFerm_l] = new PropertySchema( PropertyNames::BrewNote::projVolIntoFerm_l, kcolBNoteProjVolIntoFerm, kxmlPropProjVolIntoFerm, QString("real"), QVariant(0.0));
   m_properties[PropertyNames::BrewNote::projFg]          = new PropertySchema( PropertyNames::BrewNote::projFg,          kcolBNoteProjFG,          kxmlPropProjFG,          QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::projEff_pct]         = new PropertySchema( PropertyNames::BrewNote::projEff_pct,         kcolBNoteProjEff,         kxmlPropProjEff,         QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::projABV_pct]         = new PropertySchema( PropertyNames::BrewNote::projABV_pct,         kcolBNoteProjABV,         kxmlPropProjABV,         QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::projAtten]       = new PropertySchema( PropertyNames::BrewNote::projAtten,       kcolBNoteProjAtten,       kxmlPropProjAtten,       QString("real"), QVariant(75.0));
   m_properties[PropertyNames::BrewNote::projPoints]        = new PropertySchema( PropertyNames::BrewNote::projPoints,        kcolBNoteProjPnts,        kxmlPropProjPnts,        QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::projFermPoints]    = new PropertySchema( PropertyNames::BrewNote::projFermPoints,    kcolBNoteProjFermPnts,    kxmlPropProjFermPnts,    QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::boilOff_l]         = new PropertySchema( PropertyNames::BrewNote::boilOff_l,         kcolBNoteBoilOff,         kxmlPropBoilOff,         QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::finalVolume_l]          = new PropertySchema( PropertyNames::BrewNote::finalVolume_l,          kcolBNoteFinVol,          kxmlPropFinVol,          QString("real"), QVariant(1.0));
   m_properties[PropertyNames::BrewNote::attenuation]           = new PropertySchema( PropertyNames::BrewNote::attenuation,           kcolBNoteAtten,           kxmlPropAtten,           QString("real"), QVariant(1.0));

   m_properties[PropertyNames::NamedEntity::display]         = new PropertySchema( PropertyNames::NamedEntity::display,         kcolDisplay,              QString(),               QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]         = new PropertySchema( PropertyNames::NamedEntity::deleted,         kcolDeleted,              QString(),               QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]          = new PropertySchema( PropertyNames::NamedEntity::folder,          kcolFolder,              QString(),               QString("text"), QString("''"));

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
   m_properties[PropertyNames::NamedEntity::name]        = new PropertySchema( PropertyNames::NamedEntity::name,        kcolName,             kxmlPropName,        QString("text"),    QString("''"), QString("not null"));
   m_properties[PropertyNames::Water::notes]       = new PropertySchema( PropertyNames::Water::notes,       kcolNotes,            kxmlPropNotes,       QString("text"),    QString("''"));
   m_properties[PropertyNames::Water::amount]      = new PropertySchema( PropertyNames::Water::amount,      kcolAmount,           kxmlPropAmount,      QString("real"),    QVariant(0.0));

   m_properties[PropertyNames::Water::calcium_ppm]     = new PropertySchema( PropertyNames::Water::calcium_ppm,     kcolWaterCalcium,     kxmlPropCalcium,     QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::bicarbonate_ppm] = new PropertySchema( PropertyNames::Water::bicarbonate_ppm, kcolWaterBiCarbonate, kxmlPropBiCarbonate, QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::sulfate_ppm]     = new PropertySchema( PropertyNames::Water::sulfate_ppm,     kcolWaterSulfate,     kxmlPropSulfate,     QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::sodium_ppm]      = new PropertySchema( PropertyNames::Water::sodium_ppm,      kcolWaterSodium,      kxmlPropSodium,      QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::chloride_ppm]    = new PropertySchema( PropertyNames::Water::chloride_ppm,    kcolWaterChloride,    kxmlPropChloride,    QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::magnesium_ppm]   = new PropertySchema( PropertyNames::Water::magnesium_ppm,   kcolWaterMagnesium,   kxmlPropMagnesium,   QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::ph]          = new PropertySchema( PropertyNames::Water::ph,          kcolPH,               kxmlPropPH,          QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::alkalinity]  = new PropertySchema( PropertyNames::Water::alkalinity,  kcolWaterAlkalinity,  QString(),           QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::type]        = new PropertySchema( PropertyNames::Water::type,        kcolWaterType,        QString(),           QString("int"),     QVariant(0));
   m_properties[PropertyNames::Water::mashRO]      = new PropertySchema( PropertyNames::Water::mashRO,      kcolWaterMashRO,      QString(),           QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::spargeRO]    = new PropertySchema( PropertyNames::Water::spargeRO,    kcolWaterSpargeRO,    QString(),           QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Water::alkalinityAsHCO3]      = new PropertySchema( PropertyNames::Water::alkalinityAsHCO3,      kcolWaterAsHCO3,      QString(),           QString("boolean"), QVariant(true));

   m_properties[PropertyNames::NamedEntity::display]     = new PropertySchema( PropertyNames::NamedEntity::display,     kcolDisplay,          QString(),           QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]     = new PropertySchema( PropertyNames::NamedEntity::deleted,     kcolDeleted,          QString(),           QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]      = new PropertySchema( PropertyNames::NamedEntity::folder,      kcolFolder,           QString(),           QString("text"),    QString("''"));

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
   m_properties[PropertyNames::NamedEntity::name]     = new PropertySchema( PropertyNames::NamedEntity::name,     kcolName,         QString(), QString("text"),    QString("''"), QString("not null"));
   m_properties[PropertyNames::Salt::amount]   = new PropertySchema( PropertyNames::Salt::amount,   kcolAmount,       QString(), QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Salt::amountIsWeight] = new PropertySchema( PropertyNames::Salt::amountIsWeight, kcolSaltAmtIsWgt, QString(), QString("boolean"), QVariant(true));
   m_properties[PropertyNames::Salt::percentAcid]  = new PropertySchema( PropertyNames::Salt::percentAcid,  kcolSaltPctAcid,  QString(), QString("real"),    QVariant(0.0));
   m_properties[PropertyNames::Salt::isAcid]   = new PropertySchema( PropertyNames::Salt::isAcid,   kcolSaltIsAcid,   QString(), QString("boolean"), QVariant(false));

   m_properties[PropertyNames::Salt::type]     = new PropertySchema( PropertyNames::Salt::type,    kcolSaltType,  QString(), QString("int"),     QVariant(0));
   m_properties[PropertyNames::Salt::addTo]    = new PropertySchema( PropertyNames::Salt::addTo,   kcolSaltAddTo, QString(), QString("int"),     QVariant(0));

   m_properties[PropertyNames::NamedEntity::display]  = new PropertySchema( PropertyNames::NamedEntity::display, kcolDisplay,   QString(), QString("boolean"), QVariant(true));
   m_properties[PropertyNames::NamedEntity::deleted]  = new PropertySchema( PropertyNames::NamedEntity::deleted, kcolDeleted,   QString(), QString("boolean"), QVariant(false));
   m_properties[PropertyNames::NamedEntity::folder]   = new PropertySchema( PropertyNames::NamedEntity::folder,  kcolFolder,    QString(), QString("text"),    QString("''"));

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

   m_properties[PropertyNames::Fermentable::inventory]      = new PropertySchema( PropertyNames::Fermentable::inventory,     kcolAmount,        kxmlPropAmount, QString("real"), QVariant(0.0));
}

void TableSchema::defineHopInventoryTable()
{
   m_type = INV;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[PropertyNames::Hop::inventory] = new PropertySchema( PropertyNames::Hop::inventory, kcolAmount, kxmlPropAmount, QString("real"), QVariant(0.0));
}

void TableSchema::defineMiscInventoryTable()
{
   m_type = INV;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[PropertyNames::Misc::inventory] = new PropertySchema( PropertyNames::Misc::inventory, kcolAmount, kxmlPropAmount, QString("real"), QVariant(0.0));

}

void TableSchema::defineYeastInventoryTable()
{
   m_type = INV;

   m_key                        = new PropertySchema();
   m_key->addProperty(kpropKey, Brewtarget::PGSQL,  kcolKey, QString(""), QString("integer"), QVariant(0), 0, kPgSQLConstraint);
   m_key->addProperty(kpropKey, Brewtarget::SQLITE, kcolKey, QString(""), QString("integer"), QVariant(0), 0, kSQLiteConstraint);

   m_properties[PropertyNames::Yeast::inventory] = new PropertySchema( kpropQuanta,  kcolYeastQuanta, kxmlPropAmount, QString("real"), QVariant(0.0));

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
