/*
 * WaterListModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2025
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

#include "WaterListModel.h"
#include "water.h"
#include "database.h"
#include "recipe.h"

WaterListModel::WaterListModel(QWidget* parent)
   : QAbstractListModel(parent), m_recipe(nullptr)
{
   connect( &(Database::instance()), &Database::newWaterSignal, this, &WaterListModel::addWater );
   connect( &(Database::instance()), SIGNAL(deletedSignal(Water*)), this, SLOT(removeWater(Water*)) );
   repopulateList();
}

void WaterListModel::addWater(Water* water)
{
   if ( !water || m_waters.contains(water) || water->deleted() || !water->display() ) {
      return;
   }

   int size = m_waters.size();
   beginInsertRows( QModelIndex(), size, size );
   m_waters.append(water);
   connect( water, &Ingredient::changed, this, &WaterListModel::waterChanged );
   endInsertRows();
}

void WaterListModel::addWaters(QList<Water*> waters)
{
//   QList<Water*>::iterator i;
   QList<Water*> tmp;

   foreach ( Water* i, waters) {
      // if the water is not already in the list and
      // if the water has not been deleted and
      // if the water is to be displayed, then append it
      if ( waters.contains(i) && ! i->deleted() && i->display() ) {
         tmp.append(i);
      }
   }

   int size = waters.size();
   if (size+tmp.size()) {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      m_waters.append(tmp);

      foreach ( Water* i, tmp) {
         connect( i, &Ingredient::changed, this, &WaterListModel::waterChanged );
      }
      endInsertRows();
   }
}

void WaterListModel::removeWater(Water* water)
{
   int ndx = m_waters.indexOf(water);
   if( ndx > 0 ) {
      beginRemoveRows( QModelIndex(), ndx, ndx );
      disconnect( water, nullptr, this, nullptr );
      m_waters.removeAt(ndx);
      endRemoveRows();
   }
}

void WaterListModel::removeAll() {
   if (m_waters.size()) {
      beginRemoveRows( QModelIndex(), 0, m_waters.size()-1 );
      while( !m_waters.isEmpty() ) {
         disconnect( m_waters.takeLast(), nullptr, this, nullptr );
      }
      endRemoveRows();
   }
}

void WaterListModel::waterChanged(QMetaProperty prop, QVariant val)
{
   Water* eSend = qobject_cast<Water*>(sender());
   Q_UNUSED(val)

   // NOTE: how to get around the issue that the sender might live in
   // a different thread and therefore always cause eSend == 0?
   if( eSend == nullptr )
      return;

   QString propName(prop.name());
   if ( propName == "name" ) {
      int ndx = m_waters.indexOf(eSend);
      if ( ndx >= 0 )
         emit dataChanged( createIndex(ndx,0), createIndex(ndx,0) );
   }
}

void WaterListModel::recChanged(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());
   if ( propName == "water" ) {
      Water* newWater = qobject_cast<Water*>(Ingredient::extractPtr(val));
      // Now do something with the water.
      Q_UNUSED(newWater) // Until then, this will keep the compiler happy
   }
}

void WaterListModel::repopulateList()
{
   removeAll();
   addWaters( Database::instance().waters() );
}

Water* WaterListModel::at(int ndx)
{
   if( ndx >= 0 && ndx < m_waters.size() )
      return m_waters[ndx];
   else
      return nullptr;
}

int WaterListModel::indexOf(Water* w)
{
   return m_waters.indexOf(w);
}

QModelIndex WaterListModel::find(Water* w)
{
   int indx = m_waters.indexOf(w);
   if( indx < 0 )
      return QModelIndex();
   else
      return index(indx,0);
}

void WaterListModel::observeRecipe(Recipe* rec)
{
   if( m_recipe )
      disconnect( m_recipe, nullptr, this, nullptr );
   m_recipe = rec;

   if( m_recipe )
      connect( m_recipe, &Ingredient::changed, this, &WaterListModel::recChanged );
}

int WaterListModel::rowCount( QModelIndex const& parent ) const
{
   Q_UNUSED(parent)
   return m_waters.size();
}

QVariant WaterListModel::data( QModelIndex const& index, int role ) const
{
   int row = index.row();
   int col = index.column();
   if( col == 0 && role == Qt::DisplayRole )
      return QVariant(m_waters.at(row)->name());
   else
      return QVariant();
}

QVariant WaterListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   Q_UNUSED(section)
   Q_UNUSED(orientation)
   Q_UNUSED(role)
   return QVariant(QString("Testing..."));
}
