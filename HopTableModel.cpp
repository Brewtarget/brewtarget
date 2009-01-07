/*
 * HopTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>

#include "hop.h"
#include <vector>
#include <QString>
#include <vector>
#include <iostream>
#include "hop.h"
#include "observable.h"
#include "HopTableModel.h"

HopTableModel::HopTableModel(QWidget* parent)
: QAbstractTableModel(parent), MultipleObserver()
{
   hopObs.clear();
}

void HopTableModel::addHop(Hop* hop)
{
   std::vector<Hop*>::iterator iter;
   
   //Check to see if it's already in the list
   for( iter=hopObs.begin(); iter != hopObs.end(); iter++ )
      if( *iter == hop )
         return;
   
   hopObs.push_back(hop);
   reset(); // Tell everybody that the table has changed.
}

bool HopTableModel::removeHop(Hop* hop)
{
   std::vector<Hop*>::iterator iter;
   
   for( iter=hopObs.begin(); iter != hopObs.end(); iter++ )
      if( *iter == hop )
      {
         hopObs.erase(iter);
         reset(); // Tell everybody the table has changed.
         return true;
      }
   
   return false;
}

void HopTableModel::notify(Observable* notifier)
{
   int i;
   
   // Find the notifier in the list
   for( i = 0; i < (int)hopObs.size(); ++i )
   {
      if( notifier == hopObs[i] )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, HOPNUMCOLS));
   }
}

int HopTableModel::rowCount(const QModelIndex& parent) const
{
   return hopObs.size();
}

int HopTableModel::columnCount(const QModelIndex& parent) const
{
   return HOPNUMCOLS;
}

QVariant HopTableModel::data( const QModelIndex& index, int role ) const
{
   Hop* row;
   
   // Ensure the row is ok.
   if( index.row() >= (int)hopObs.size() )
   {
      std::cerr << "Bad model index. row = " << index.row() << std::endl;
      return QVariant();
   }
   else
      row = hopObs[index.row()];
   
   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();
   
   switch( index.column() )
   {
      case HOPNAMECOL:
         return QVariant(row->getName().c_str());
      case HOPALPHACOL:
         return QVariant(row->getAlpha_pct());
      case HOPAMOUNTCOL:
         return QVariant(row->getAmount_kg());
      case HOPUSECOL:
         return QVariant(row->getUse().c_str());
      case HOPTIMECOL:
         return QVariant(row->getTime_min());
      default :
         std::cerr << "Bad column: " << index.column() << std::endl;
         return QVariant();
   }
}

QVariant HopTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case HOPNAMECOL:
            return QVariant("Name");
         case HOPALPHACOL:
            return QVariant("Alpha %");
         case HOPAMOUNTCOL:
            return QVariant("Amount (kg)");
         case HOPUSECOL:
            return QVariant("Use");
         case HOPTIMECOL:
            return QVariant("Time (min)");
         default:
            std::cerr << "Bad column: " << section << std::endl;
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags HopTableModel::flags(const QModelIndex& index ) const
{
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
          Qt::ItemIsEnabled;
}

bool HopTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Hop *row;
   
   if( index.row() >= (int)hopObs.size() || role != Qt::EditRole )
      return false;
   else
      row = hopObs[index.row()];
   
   switch( index.column() )
   {
      case HOPNAMECOL:
         if( value.canConvert(QVariant::String))
         {
            row->setName(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case HOPALPHACOL:
         if( value.canConvert(QVariant::Double) )
         {
            row->setAlpha_pct(value.toDouble());
            return true;
         }
         else
            return false;
      case HOPAMOUNTCOL:
         if( value.canConvert(QVariant::Double) )
         {
            row->setAmount_kg(value.toDouble());
            return true;
         }
         else
            return false;
      case HOPUSECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setUse(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case HOPTIMECOL:
         if( value.canConvert(QVariant::Double) )
         {
            row->setTime_min(value.toDouble());
            return true;
         }
         else
            return false;
      default:
         std::cerr << "Bad column: " << index.column() << std::endl;
         return false;
   }
}
