/*
 * SetterCommand.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QModelIndexList>
#include "SetterCommand.h"

SetterCommand::SetterCommand( QSqlRelationalTableModel* table, const char* key_name, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object, bool notify)
   : QUndoCommand(QString("Change %1 to %2").arg(col_name).arg(value.toString())),
     table(table), key_name(key_name), key(key), prop(prop), col_name(col_name), value(value), object(object), notify(notify)
{
}

SetterCommand::~SetterCommand()
{
}

void SetterCommand::redo()
{
   /*
   int columnIndex = table->fieldIndex(col_name);
   int keyColumnIndex = table->fieldIndex(key_name);
   
   int rows = table->rowCount();
   //qDebug() << table->data(table->index(0,0)).toString();
   //qDebug() << table->data(table->index(0,1)).toString();
   
   // Get the current filter.
   //QString filter = table->filter();
   
   // Makes the only visible row the one that has our key.
   //table->setFilter( QString("`%1`='%2'").arg(key_name).arg(key) );
   //table->setFilter( QString("`%1`='%2'").arg(key_name).arg(1) );
   table->select();
   
   for( int i = 0; i < rows; ++i )
      qDebug() << "i: " << i << " key: " << table->data(table->index(i,keyColumnIndex)).toInt();
   
   QModelIndexList indices = table->match( table->index(0,keyColumnIndex), Qt::DisplayRole, QVariant(key) );
   if( indices.size() <= 0 )
      return;
   
   int rowIndex = indices[0].row();
   
   rows = table->rowCount();
   // Record the old data for undo.
   //oldValue = table->record(0).value( col_name );
   oldValue = table->data( table->index(rowIndex,columnIndex) );
   
   // Change the data.
   
   //table->record(0).setValue( col_name, value );
   table->setData( table->index(rowIndex,columnIndex), value );
   table->submitAll();
   
   // Unset the filter.
   //table->setFilter(filter);
   //table->select();
   */
   
   QSqlQuery q( table->database() );
   q.setForwardOnly(true);
   q.exec( QString("SELECT `%1` FROM `%2` WHERE `%3`='%4'")
                .arg(col_name)
                .arg(table->tableName())
                .arg(key_name)
                .arg(key)
         );
   if( q.next() )
      oldValue = q.record().value(col_name);
   if( q.lastError().isValid() )
   {
      Brewtarget::logE( QString("SetterCommand::redo: %1.\n   \"%2\"").arg(q.lastError().text()).arg(q.lastQuery()) );
   }
   
   q = QSqlQuery( table->database() );
   q.setForwardOnly(true);
   q.prepare(QString("UPDATE `%1` SET `%2` = :value WHERE `%3`='%4'")
                .arg(table->tableName())
                .arg(col_name)
                .arg(key_name)
                .arg(key)
            );
   q.bindValue(":value",value);
   q.exec();
   if( q.lastError().isValid() )
   {
      Brewtarget::logE( QString("SetterCommand::redo: %1.\n   \"%2\"").arg(q.lastError().text()).arg(q.lastQuery()) );
   }
   
   // Emit the notifier.
   //prop.notifySignal().invoke( object, Q_ARG(QMetaProperty, prop), Q_ARG(QVariant, value) );
   if( notify )
      emit object->changed(prop,value);
}

void SetterCommand::undo()
{
   QSqlQuery q( table->database() );
   q.setForwardOnly(true);
   q.prepare( QString("UPDATE `%1` SET `%2`= :value WHERE `%3`='%4'")
                   .arg(table->tableName())
                   .arg(col_name)
                   .arg(key_name)
                   .arg(key)
            );
   q.bindValue( ":value", oldValue );
   q.exec();
   if( q.lastError().isValid() )
   {
      Brewtarget::logE( QString("SetterCommand::redo: %1.\n   \"%2\"").arg(q.lastError().text()).arg(q.lastQuery()) );
   }
   
   // Emit the notifier.
   //prop.notifySignal().invoke( object, Q_ARG(QMetaProperty, prop), Q_ARG(QVariant, value) );
   if( notify )
      emit object->changed(prop,value);
}