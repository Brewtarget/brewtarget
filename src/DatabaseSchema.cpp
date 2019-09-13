/*
 * DatabaseSchema.cpp is part of Brewtarget, and is Copyright the following
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
#include <QDebug>
#include <QString>
#include <QStringBuilder>

#include "TableSchema.h"
#include "TableSchemaConst.h"
#include "DatabaseSchema.h"
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

static const QString kPgSQLId("id SERIAL PRIMARY KEY,");
static const QString kSQLiteId("id INTEGER PRIMARY KEY autoincrement,");

static QStringList dbPrimaryKey  = QStringList() <<
      kSQLiteId <<
      kPgSQLId;

static const QString kDefault("DEFAULT");
static const QString kNameColumn("name text not null DEFAULT '',");

DatabaseSchema::DatabaseSchema()
{
   loadTables();
   m_type = Brewtarget::dbType();
   m_id = dbPrimaryKey[m_type];
   m_name = kNameColumn;
}

void DatabaseSchema::loadTables()
{
   int iT;

   for ( iT = Brewtarget::NOTABLE; iT <= Brewtarget::YEASTINVTABLE; iT++ ) {
      Brewtarget::DBTable tab = static_cast<Brewtarget::DBTable>(iT);
      TableSchema* tmp = new TableSchema(tab);
      m_tables.insert(tab, tmp);
   }
}

TableSchema *DatabaseSchema::table(Brewtarget::DBTable table)
{
   if ( m_tables.contains( table ) ) {
      return m_tables.value(table);
   }

   return nullptr;
}

TableSchema *DatabaseSchema::table(QString tableName)
{
   if ( dbTableToName.contains(tableName) ) {
      return m_tables.value( static_cast<Brewtarget::DBTable>(dbTableToName.indexOf(tableName)));
   }
   return nullptr;
}

// I believe one method replaces EVERY create_ method in DatabaseSchemaHelper.
// It is so beautiful, it must be evil.
const QString DatabaseSchema::generateCreateTable(Brewtarget::DBTable table)
{

   if ( ! m_tables.contains(table) ) {
      return QString();
   }

   TableSchema* tSchema = m_tables.value(table);
   QString retVal = QString("CREATE TABLE %1 (%2 %3")
                     .arg(tSchema->tableName())
                     .arg( m_id )
                     .arg( m_name );

   QMapIterator<QString, PropertySchema*> i(tSchema->properties());
   while ( i.hasNext() ) {
      i.next();
      PropertySchema* prop = i.value();

      retVal.append( QString("%1 %2 %3 %4, ")
                       .arg( prop->colName(Brewtarget::dbType()))
                       .arg( prop->colType() )
                       .arg( kDefault )
                       .arg( prop->defaultValue().toString() )
      );
   }

   QMapIterator<QString, PropertySchema*> j(tSchema->foreignKeys());
   while ( j.hasNext() ) {
      j.next();
      PropertySchema* key = j.value();

      retVal.append( QString("FOREIGN KEY(%1) REFERENCES %2(id), ")
                       .arg( key->colName(Brewtarget::dbType()) )
                       .arg( dbTableToName[ key->fTable() ] )
      );
   }

   // always have to worry about the damned trailing ,
   retVal.chop(1);
   retVal.append(");");

   return retVal;
}

