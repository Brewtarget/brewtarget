/*
 * SetterCommand.h is part of Brewtarget, and is Copyright Philip G. Lee
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

class SetterCommand;

#include <QUndoCommand>
#include <QMetaProperty>
#include <QSqlRelationalTableModel>
#include "BeerXMLElement.h"

class SetterCommand : public QUndoCommand
{
public:
   /*! A setter command that will set the entry specified by (table,key,col_name)
    * to value, and call prop's notify() method when done.
    */
   SetterCommand( QSqlRelationalTableModel* table,
                  const char* key_name,
                  int key,
                  const char* col_name,
                  QVariant value,
                  QMetaProperty prop,
                  BeerXMLElement* object,
                  bool notify=true);
   virtual ~SetterCommand();
   
   //virtual int id() const;
   //virtual bool mergeWith( const QUndoCommand* command );
   
   virtual void redo(); // Executes the command.
   virtual void undo(); // Undoes the command.
   
private:
   QSqlRelationalTableModel* table;
   QString key_name;
   int key;
   QMetaProperty prop;
   QString col_name;
   QVariant value;
   QVariant oldValue;
   BeerXMLElement* object;
   bool notify;
};