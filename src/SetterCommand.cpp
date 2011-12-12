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
   // Get the current filter.
   QString filter = table->filter();
   
   // Makes the only visible row the one that has our key.
   table->setFilter( QString("%1=%2").arg(key_name).arg(key) );
   table->select();
   
   // Record the old data for undo.
   oldValue = table->record(0).value( col_name );
   
   // Change the data.
   table->record(0).setValue( col_name, value );
   table->submitAll();
   
   // Unset the filter.
   table->setFilter(filter);
   table->select();
   
   // Emit the notifier.
   //prop.notifySignal().invoke( object, Q_ARG(QMetaProperty, prop), Q_ARG(QVariant, value) );
   if( notify )
      emit object->changed(prop,value);
}

void SetterCommand::undo()
{
   // Get the current filter.
   QString filter = table->filter();
   
   // Makes the only visible row the one that has our key.
   table->setFilter( QString("%1=%2").arg(key_name).arg(key) );
   table->select();
   
   // Change the data back
   table->record(0).setValue( col_name, oldValue );
   table->submitAll();
   
   // Unset the filter.
   table->setFilter(filter);
   table->select();
   
   // Emit the notifier.
   //prop.notifySignal().invoke( object, Q_ARG(QMetaProperty, prop), Q_ARG(QVariant, value) );
   if( notify )
      emit object->changed(prop,value);
}