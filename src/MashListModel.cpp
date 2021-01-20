/*
 * MashListModel.cpp is part of Brewtarget, and is Copyright the following
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

#include "MashListModel.h"
#include "style.h"
#include "database.h"
#include "recipe.h"
#include "mash.h"

MashListModel::MashListModel(QWidget* parent)
   : QAbstractListModel(parent), recipe(0)
{
   connect( &(Database::instance()), &Database::newMashSignal, this, &MashListModel::addMash );
   connect( &(Database::instance()), SIGNAL(deletedSignal(Mash*)), this, SLOT(removeMash(Mash*)) );
   repopulateList();
}

void MashListModel::addMash(Mash* m)
{
   if( !m || !m->display() || m->deleted() )
      return;
   
   if( !mashes.contains(m) )
   {
      int size = mashes.size();
      beginInsertRows( QModelIndex(), size, size );
      mashes.append(m);
      connect( m, &Ingredient::changed, this, &MashListModel::mashChanged );
      endInsertRows();
   }
}

void MashListModel::addMashes(QList<Mash*> m)
{
   QList<Mash*>::iterator i;
   QList<Mash*> tmp;
   
   for( i = m.begin(); i != m.end(); i++ )
   {
      if( !mashes.contains(*i) && (*i)->display() && ! (*i)->deleted())
         tmp.append(*i);
   }
   
   int size = mashes.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      mashes.append(tmp);
      
      for( i = tmp.begin(); i != tmp.end(); i++ )
         connect( *i, &Ingredient::changed, this, &MashListModel::mashChanged );
      
      endInsertRows();
   }
}

void MashListModel::removeMash(Mash* mash)
{
   int ndx = mashes.indexOf(mash);
   if( ndx >= 0 )
   {
      beginRemoveRows( QModelIndex(), ndx, ndx );
      disconnect( mash, 0, this, 0 );
      mashes.removeAt(ndx);
      endRemoveRows();
   }
}

void MashListModel::removeAll()
{
   if (mashes.size())
   {
      beginRemoveRows( QModelIndex(), 0, mashes.size()-1 );
      while( !mashes.isEmpty() )
         disconnect( mashes.takeLast(), 0, this, 0 );
      endRemoveRows();
   }
}

void MashListModel::mashChanged(QMetaProperty prop, QVariant val)
{   
   Mash* mSend = qobject_cast<Mash*>(sender());
   
   // NOTE: how to get around the issue that the sender might live in
   // a different thread and therefore always cause sSend == 0?
   if( mSend == 0 )
      return;
   
   QString propName(prop.name());
   if( propName == "name" )
   {
      int ndx = mashes.indexOf(mSend);
      if( ndx >= 0 )
         emit dataChanged( createIndex(ndx,0), createIndex(ndx,0) );
   }
}

void MashListModel::repopulateList()
{
   removeAll();
   addMashes( Database::instance().mashs() );
}

Mash* MashListModel::at(int ndx)
{
   if( ndx >= 0 && ndx < mashes.size() )
      return mashes[ndx];
   else
      return 0;
}

int MashListModel::indexOf(Mash* m)
{
   return mashes.indexOf(m);
}

int MashListModel::rowCount( QModelIndex const& parent ) const
{
   return mashes.size();
}

QVariant MashListModel::data( QModelIndex const& index, int role ) const
{
   int row = index.row();
   int col = index.column();
   if( col == 0 && role == Qt::DisplayRole )
      return QVariant(mashes.at(row)->name());
   else
      return QVariant();
}

QVariant MashListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   return QVariant(QString("Header Data..."));
}
