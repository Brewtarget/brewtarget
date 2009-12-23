/*
 * WaterTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QLineEdit>
#include <QString>

#include <vector>
#include <iostream>
#include "observable.h"
#include "WaterTableModel.h"
#include "water.h"
#include "unit.h"
#include "brewtarget.h"

WaterTableModel::WaterTableModel(WaterTableWidget* parent)
: QAbstractTableModel(parent), MultipleObserver()
{
   waterObs.clear();
   parentTableWidget = parent;
}

void WaterTableModel::addWater(Water* water)
{
   std::vector<Water*>::iterator iter;

   //Check to see if it's already in the list
   for( iter=waterObs.begin(); iter != waterObs.end(); iter++ )
      if( *iter == water )
         return;

   waterObs.push_back(water);
   addObserved(water);
   reset(); // Tell everybody that the table has changed.
   
   if(parentTableWidget)
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

bool WaterTableModel::removeWater(Water* water)
{
   std::vector<Water*>::iterator iter;

   for( iter=waterObs.begin(); iter != waterObs.end(); iter++ )
      if( *iter == water )
      {
         waterObs.erase(iter);
         removeObserved(water);
         reset(); // Tell everybody the table has changed.
         
         if(parentTableWidget)
         {
            parentTableWidget->resizeColumnsToContents();
            parentTableWidget->resizeRowsToContents();
         }
         
         return true;
      }

   return false;
}

void WaterTableModel::removeAll()
{
   unsigned int i;

   for( i = 0; i < waterObs.size(); ++i )
      removeObserved(waterObs[i]);

   waterObs.clear();
   reset();
}

void WaterTableModel::notify(Observable* notifier, QVariant info)
{
   int i;

   // Find the notifier in the list
   for( i = 0; i < (int)waterObs.size(); ++i )
   {
      if( notifier == waterObs[i] )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, WATERNUMCOLS));
   }
}

int WaterTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return waterObs.size();
}

int WaterTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return WATERNUMCOLS;
}

QVariant WaterTableModel::data( const QModelIndex& index, int role ) const
{
   Water* row;

   // Ensure the row is ok.
   if( index.row() >= (int)waterObs.size() )
   {
      Brewtarget::log(Brewtarget::WARNING, tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = waterObs[index.row()];

   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();

   switch( index.column() )
   {
      case WATERNAMECOL:
         return QVariant(row->getName().c_str());
      case WATERAMOUNTCOL:
         return QVariant( Brewtarget::displayAmount(row->getAmount_l(), Units::liters) );
      case WATERCALCIUMCOL:
         return QVariant( Brewtarget::displayAmount(row->getCalcium_ppm(), 0) );
      case WATERBICARBONATECOL:
         return QVariant( Brewtarget::displayAmount(row->getBicarbonate_ppm(), 0) );
      case WATERSULFATECOL:
         return QVariant( Brewtarget::displayAmount(row->getSulfate_ppm(), 0) );
      case WATERCHLORIDECOL:
         return QVariant( Brewtarget::displayAmount(row->getChloride_ppm(), 0) );
      case WATERSODIUMCOL:
         return QVariant( Brewtarget::displayAmount(row->getSodium_ppm(), 0) );
      case WATERMAGNESIUMCOL:
         return QVariant( Brewtarget::displayAmount(row->getMagnesium_ppm(), 0) );
      default :
         Brewtarget::log(Brewtarget::WARNING, tr("Bad column: %1").arg(index.column()));
         return QVariant();
   }
}

QVariant WaterTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case WATERNAMECOL:
            return QVariant(tr("Name"));
         case WATERAMOUNTCOL:
            return QVariant(tr("Amount"));
         case WATERCALCIUMCOL:
            return QVariant(tr("Calcium (ppm)"));
         case WATERBICARBONATECOL:
            return QVariant(tr("Bicarbonate (ppm)"));
         case WATERSULFATECOL:
            return QVariant(tr("Sulfate (ppm)"));
         case WATERCHLORIDECOL:
            return QVariant(tr("Chloride (ppm)"));
         case WATERSODIUMCOL:
            return QVariant(tr("Sodium (ppm)"));
         case WATERMAGNESIUMCOL:
            return QVariant(tr("Magnesium (ppm)"));
         default:
            Brewtarget::log(Brewtarget::WARNING, tr("Bad column: %1").arg(section));
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags WaterTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();
   switch(col)
   {
      case WATERNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      default:
         return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool WaterTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Water *row;

   if( index.row() >= (int)waterObs.size() || role != Qt::EditRole )
      return false;
   else
      row = waterObs[index.row()];

   switch( index.column() )
   {
      case WATERNAMECOL:
         if( value.canConvert(QVariant::String))
         {
            row->setName(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case WATERAMOUNTCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setAmount_l( Unit::qstringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      case WATERCALCIUMCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setCalcium_ppm( value.toString().toDouble() );
            return true;
         }
         else
            return false;
      case WATERBICARBONATECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setBicarbonate_ppm( value.toString().toDouble() );
            return true;
         }
         else
            return false;
      case WATERSULFATECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setSulfate_ppm( value.toString().toDouble() );
            return true;
         }
         else
            return false;
      case WATERCHLORIDECOL:
         if( value.canConvert(QVariant::String))
         {
            row->setChloride_ppm( value.toString().toDouble() );
            return true;
         }
         else
            return false;
      case WATERSODIUMCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setSodium_ppm( value.toString().toDouble() );
            return true;
         }
         else
            return false;
      case WATERMAGNESIUMCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setMagnesium_ppm( value.toString().toDouble() );
            return true;
         }
         else
            return false;
      default:
         Brewtarget::log(Brewtarget::WARNING, tr("Bad column: %1").arg(index.column()));
         return false;
   }
}

//==========================CLASS HopItemDelegate===============================

WaterItemDelegate::WaterItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}

QWidget* WaterItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
   return new QLineEdit(parent);
}

void WaterItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   QLineEdit* line = (QLineEdit*)editor;
   line->setText(index.model()->data(index, Qt::DisplayRole).toString());
}

void WaterItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   QLineEdit* line = (QLineEdit*)editor;
   model->setData(index, line->text(), Qt::EditRole);
}

void WaterItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
