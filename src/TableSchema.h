/*
 * TableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
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
#ifndef _TABLESCHEMA_H
#define _TABLESCHEMA_H

#include "PropertySchema.h"
#include "brewtarget.h"
#include <QString>

class TableSchema : QObject
{

   Q_OBJECT

   friend class DatabaseSchemaHelper;
   friend class DatabaseSchema;
   friend class Database;
   friend class BeerXML;

public:

   enum TableType {
      BASE,
      INV,
      CHILD,
      INREC,
      BT,
      META
   };

   const QString tableName() const;
   const QString className() const;
   Brewtarget::DBTable dbTable() const;
   Brewtarget::DBTable childTable() const;
   Brewtarget::DBTable inRecTable() const;
   Brewtarget::DBTable invTable() const;
   Brewtarget::DBTable btTable() const;
   const QMap<QString, PropertySchema*> properties() const;
   const QMap<QString, PropertySchema*> foreignKeys() const;
   const PropertySchema* key() const;

   // Things to do for properties

   // Get the property object. Try not to use this?
   const PropertySchema* property(QString prop) const;
   // some properties may be named differently (like inventory v quanta)
   const QString propertyName(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // get the database column name for this property
   const QString propertyToColumn(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // get the database column type
   const QString propertyColumnType(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // get the XML tag for this column
   const QString propertyToXml(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // get the default value for this column
   const QVariant propertyColumnDefault(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // get the column size of the property's column
   int propertyColumnSize(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // given an XML tag, get the associated property name
   const QString xmlToProperty(QString xmlName, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // returns the property to be used for the increment/decrement triggers
   const QString triggerProperty() const;

   const QStringList allPropertyNames(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   const QStringList allColumnNames(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;

   // things to do on foreign keys
   // get a specific foreign key column name
   const QString foreignKeyToColumn(QString fkey, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // a lot of tables have one foreign key. This is a nice shortcut for that
   const QString foreignKeyToColumn(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;

   // which table does this foreign key point to
   Brewtarget::DBTable foreignTable(QString fkey, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   // a lot of tables have one foreign key. This is a nice shortcut for that
   Brewtarget::DBTable foreignTable(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;

   const QStringList allForeignKeyNames(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   const QStringList allForeignKeyColumnNames(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;

   //!brief Use this to get the not recipe_id index from an inrec table
   const QString inRecIndexName(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief Use this to get the child_id index from a children table
   const QString childIndexName(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief Use this to get the recipe_id from a inrec table
   const QString recipeIndexName(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief Use this to get the parent_id from a child table
   const QString parentIndexName(Brewtarget::DBTypes type = Brewtarget::ALLDB);

   // Not sure these belong here yet, but maybe
   const QString generateCreateTable(Brewtarget::DBTypes type = Brewtarget::ALLDB, QString tmpName = QString("") );
   const QString generateUpdateRow(int key, Brewtarget::DBTypes type = Brewtarget::ALLDB);
   const QString generateUpdateRow(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   // this one includes the foreign keys and is really only suitable for copying databases
   const QString generateInsertRow(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   // this one ignores the foreign keys and is more generally useful
   const QString generateInsertProperties(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   // when dropping columns, we have to copy tables in sqlite. This does that.
   const QString generateCopyTable( QString dest, Brewtarget::DBTypes type = Brewtarget::ALLDB);

   const QString generateDecrementTrigger(Brewtarget::DBTypes type);
   const QString generateIncrementTrigger(Brewtarget::DBTypes type);

   bool isInventoryTable();
   bool isBaseTable();
   bool isChildTable();
   bool isInRecTable();
   bool isBtTable();
   bool isMetaTable();

   const QString keyName(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;

private:

   // I only allow table schema to be made with a DBTable constant
   // It saves a lot of work, and I think the name to constant
   // mapping doesn't belong here -- it belongs in DatabaseSchema
   TableSchema(Brewtarget::DBTable dbTable);

   QString m_tableName;
   QString m_className;
   Brewtarget::DBTable m_dbTable;
   TableType m_type;

   // these are only set by the base tables
   Brewtarget::DBTable m_childTable;
   Brewtarget::DBTable m_inRecTable;
   Brewtarget::DBTable m_invTable;
   Brewtarget::DBTable m_btTable;

   QString m_trigger;

   PropertySchema* m_key;
   QMap<QString,PropertySchema*> m_properties;
   QMap<QString,PropertySchema*> m_foreignKeys;
   // It all depends on the call I want to make. I can require the type on
   // every call to a TableSchema object which is dull, repetitive and makes
   // some already difficult to read calls harder to read. Or I can cache the
   // default in the table and use that if ALLDB is sent, which breaks the
   // metaphor.
   Brewtarget::DBTypes m_defType;

   // getter only. But this is private because only my dearest,
   // closest friends can do this
   Brewtarget::DBTypes defType() const;

   void defineTable();
   void defineStyleTable();
   void defineEquipmentTable();
   void defineFermentableTable();
   void defineHopTable();
   void defineInstructionTable();
   void defineMashTable();
   void defineMashstepTable();
   void defineMiscTable();
   void defineRecipeTable();
   void defineYeastTable();
   void defineWaterTable();
   void defineSaltTable();
   void defineBrewnoteTable();
   void defineSettingsTable();

   // and we can get away with one method for the child tables
   void defineChildTable(Brewtarget::DBTable table);

   // and almost one method for all the in_recipe tables
   void defineInRecipeTable(QString childIdx, Brewtarget::DBTable table);
   // Instructions in recipe actually carry information. Sigh.
   void defineInstructionInRecipeTable( QString childIdx, Brewtarget::DBTable table);

   // one method for all the bt_tables
   void defineBtTable(QString childIdx, Brewtarget::DBTable table);

   // Inventory tables are strange and I didn't feel quite comfortable trying to make one
   // method for all of them;
   void defineFermInventoryTable();
   void defineHopInventoryTable();
   void defineMiscInventoryTable();
   void defineYeastInventoryTable();

};

#endif
