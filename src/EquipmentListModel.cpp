/*
 * EquipmentListModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - swstim <swstim@gmail.com>
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

#include "EquipmentListModel.h"
#include "equipment.h"
#include "database.h"
#include "recipe.h"

EquipmentListModel::EquipmentListModel(QWidget* parent)
   : QAbstractListModel(parent), recipe(0)
{
   connect( &(Database::instance()), &Database::newEquipmentSignal, this, &EquipmentListModel::addEquipment );
   connect( &(Database::instance()), SIGNAL(deletedSignal(Equipment*)), this, SLOT(removeEquipment(Equipment*)) );
   repopulateList();
}

void EquipmentListModel::addEquipment(Equipment* equipment)
{
   if( !equipment ||
      equipments.contains(equipment) ||
      equipment->deleted() ||
      !equipment->display()
   )
      return;
   
   int size = equipments.size();
   beginInsertRows( QModelIndex(), size, size );
   equipments.append(equipment);
   connect( equipment, &Ingredient::changed, this, &EquipmentListModel::equipChanged );
   endInsertRows();
}

void EquipmentListModel::addEquipments(QList<Equipment*> equips)
{
   QList<Equipment*>::iterator i;
   QList<Equipment*> tmp;
   
   for( i = equips.begin(); i != equips.end(); i++ )
   {
      // if the equipment is not already in the list and
      // if the equipment has not been deleted and
      // if the equipment is to be displayed, then append it
      if( !equipments.contains(*i) &&
          !(*i)->deleted()         &&  
           (*i)->display() )
         tmp.append(*i);
   }
   
   int size = equipments.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      equipments.append(tmp);
   
      for( i = tmp.begin(); i != tmp.end(); i++ )
         connect( *i, &Ingredient::changed, this, &EquipmentListModel::equipChanged );
   
      endInsertRows();
   }
}

void EquipmentListModel::removeEquipment(Equipment* equipment)
{
   int ndx = equipments.indexOf(equipment);
   if( ndx > 0 )
   {
      beginRemoveRows( QModelIndex(), ndx, ndx );
      disconnect( equipment, 0, this, 0 );
      equipments.removeAt(ndx);
      endRemoveRows();
   }
}

void EquipmentListModel::removeAll()
{
   if (equipments.size())
   {
      beginRemoveRows( QModelIndex(), 0, equipments.size()-1 );
      while( !equipments.isEmpty() )
      {
         disconnect( equipments.takeLast(), 0, this, 0 );
      }
      endRemoveRows();
   }
}

void EquipmentListModel::equipChanged(QMetaProperty prop, QVariant val)
{   
   Equipment* eSend = qobject_cast<Equipment*>(sender());
   
   // NOTE: how to get around the issue that the sender might live in
   // a different thread and therefore always cause eSend == 0?
   if( eSend == 0 )
      return;
   
   QString propName(prop.name());
   if( propName == "name" )
   {
      int ndx = equipments.indexOf(eSend);
      if( ndx >= 0 )
         emit dataChanged( createIndex(ndx,0), createIndex(ndx,0) );
   }
}

void EquipmentListModel::recChanged(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());
   if( propName == "equipment" )
   {
      Equipment* newEquip = qobject_cast<Equipment*>(Ingredient::extractPtr(val));
      // Now do something with the equipment.
      Q_UNUSED(newEquip); // Until then, this will keep the compiler happy
   }
}

void EquipmentListModel::repopulateList()
{
   removeAll();
   addEquipments( Database::instance().equipments() );
}

Equipment* EquipmentListModel::at(int ndx)
{
   if( ndx >= 0 && ndx < equipments.size() )
      return equipments[ndx];
   else
      return 0;
}

int EquipmentListModel::indexOf(Equipment* e)
{
   return equipments.indexOf(e);
}

QModelIndex EquipmentListModel::find(Equipment* e)
{
   int indx = equipments.indexOf(e);
   if( indx < 0 )
      return QModelIndex();
   else
      return index(indx,0);
}

void EquipmentListModel::observeRecipe(Recipe* rec)
{
   if( recipe )
      disconnect( recipe, 0, this, 0 );
   recipe = rec;
   
   if( recipe )
      connect( recipe, &Ingredient::changed, this, &EquipmentListModel::recChanged );
}

int EquipmentListModel::rowCount( QModelIndex const& parent ) const
{
   return equipments.size();
}

QVariant EquipmentListModel::data( QModelIndex const& index, int role ) const
{
   int row = index.row();
   int col = index.column();
   if( col == 0 && role == Qt::DisplayRole )
      return QVariant(equipments.at(row)->name());
   else
      return QVariant();
}

QVariant EquipmentListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   return QVariant(QString("Testing..."));
}
