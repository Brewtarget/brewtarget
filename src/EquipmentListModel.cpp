/*
 * EquipmentListModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
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

#include <QAbstractListModel>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "model/Equipment.h"
#include "model/Recipe.h"

EquipmentListModel::EquipmentListModel(QWidget* parent) :
   QAbstractListModel(parent), recipe(0) {
   connect(&ObjectStoreTyped<Equipment>::getInstance(), &ObjectStoreTyped<Equipment>::signalObjectInserted, this, &EquipmentListModel::addEquipment);
   connect(&ObjectStoreTyped<Equipment>::getInstance(), &ObjectStoreTyped<Equipment>::signalObjectDeleted,  this, &EquipmentListModel::removeEquipment);
   this->repopulateList();
   return;
}


void EquipmentListModel::addEquipment(int equipmentId) {
   Equipment* equipment = ObjectStoreWrapper::getByIdRaw<Equipment>(equipmentId);

   if( !equipment ||
      equipments.contains(equipment) ||
      equipment->deleted() ||
      !equipment->display()
   )
      return;

   int size = equipments.size();
   beginInsertRows( QModelIndex(), size, size );
   equipments.append(equipment);
   connect( equipment, &NamedEntity::changed, this, &EquipmentListModel::equipChanged );
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
         connect( *i, &NamedEntity::changed, this, &EquipmentListModel::equipChanged );

      endInsertRows();
   }
}


void EquipmentListModel::removeEquipment([[maybe_unused]] int equipmentId,
                                         std::shared_ptr<QObject> object) {
   auto equipment = std::static_pointer_cast<Equipment>(object);
   int ndx = equipments.indexOf(equipment.get());
   if( ndx > 0 )
   {
      beginRemoveRows( QModelIndex(), ndx, ndx );
      disconnect( equipment.get(), 0, this, 0 );
      equipments.removeAt(ndx);
      endRemoveRows();
   }
   return;
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

void EquipmentListModel::equipChanged(QMetaProperty prop,
                                      [[maybe_unused]] QVariant val) {
   Equipment* eSend = qobject_cast<Equipment*>(sender());

   // NOTE: how to get around the issue that the sender might live in
   // a different thread and therefore always cause eSend == 0?
   if( eSend == 0 )
      return;

   QString propName(prop.name());
   if (propName == PropertyNames::NamedEntity::name) {
      int ndx = equipments.indexOf(eSend);
      if (ndx >= 0) {
         emit dataChanged( createIndex(ndx,0), createIndex(ndx,0) );
      }
   }
}

void EquipmentListModel::recChanged(QMetaProperty prop, QVariant val) {
   if (prop.name()== PropertyNames::Recipe::equipment) {
      Equipment* newEquip = val.value<Equipment *>();
      // .:TODO:. Now do something with the equipment.
      Q_UNUSED(newEquip); // Until then, this will keep the compiler happy
   }
   return;
}

void EquipmentListModel::repopulateList() {
   removeAll();
   addEquipments( ObjectStoreTyped<Equipment>::getInstance().getAllRaw() );
   return;
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
      connect( recipe, &NamedEntity::changed, this, &EquipmentListModel::recChanged );
}

int EquipmentListModel::rowCount([[maybe_unused]] QModelIndex const & parent ) const {
   return equipments.size();
}

QVariant EquipmentListModel::data( QModelIndex const & index, int role ) const
{
   int row = index.row();
   int col = index.column();
   if( col == 0 && role == Qt::DisplayRole )
      return QVariant(equipments.at(row)->name());
   else
      return QVariant();
}

QVariant EquipmentListModel::headerData([[maybe_unused]] int section,
                                        [[maybe_unused]] Qt::Orientation orientation,
                                        [[maybe_unused]] int role ) const {
   return QVariant(QString("Testing..."));
}
