/*
 * WaterTableModel.cpp is part of Brewtarget, and is Copyright the following
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

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QLineEdit>
#include <QString>

#include <QList>
#include "database.h"
#include "WaterTableModel.h"
#include "WaterTableWidget.h"
#include "water.h"
#include "unit.h"
#include "recipe.h"
#include "brewtarget.h"

WaterTableModel::WaterTableModel(WaterTableWidget* parent)
   : QAbstractTableModel(parent), recObs(0), parentTableWidget(parent)
{
}

void WaterTableModel::observeRecipe(Recipe* rec)
{
   if( recObs )
   {
      disconnect( recObs, 0, this, 0 );
      removeAll();
   }
   
   recObs = rec;
   if( recObs )
   {
      connect( recObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      addWaters( recObs->waters() );
   }
}

void WaterTableModel::observeDatabase(bool val)
{
   if( val )
   {
      observeRecipe(0);
      removeAll();
      connect( &(Database::instance()), SIGNAL(newWaterSignal(Water*)), this, SLOT(addWater(Water*)) );
      connect( &(Database::instance()), SIGNAL(deletedWaterSignal(Water*)), this, SLOT(removeWater(Water*)) );
      addWaters( Database::instance().waters() );
   }
   else
   {
      removeAll();
      disconnect( &(Database::instance()), 0, this, 0 );
   }
}

void WaterTableModel::addWater(Water* water)
{
   if( waterObs.contains(water) )
      return;
   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if(
      recObs == 0 &&
      (
         water->deleted() ||
         !water->display()
      )
   )
      return;
   
   beginInsertRows( QModelIndex(), waterObs.size(), waterObs.size() );
   waterObs.append(water);
   connect( water, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   endInsertRows();
   
   if(parentTableWidget)
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void WaterTableModel::addWaters(QList<Water*> waters)
{
   QList<Water*>::iterator i;
   QList<Water*> tmp;
   
   for( i = waters.begin(); i != waters.end(); i++ )
   {
      if( !waterObs.contains(*i) )
         tmp.append(*i);
   }
   
   int size = waterObs.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      waterObs.append(tmp);
      
      for( i = tmp.begin(); i != tmp.end(); i++ )
         connect( *i, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      
      endInsertRows();
   }

   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
   
}

void WaterTableModel::removeWater(Water* water)
{
   int i;
   
   i = waterObs.indexOf(water);
   if( i >= 0 )
   {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( water, 0, this, 0 );
      waterObs.removeAt(i);
      endRemoveRows();
         
      if(parentTableWidget)
      {
         parentTableWidget->resizeColumnsToContents();
         parentTableWidget->resizeRowsToContents();
      }
   }
}

void WaterTableModel::removeAll()
{
   beginRemoveRows( QModelIndex(), 0, waterObs.size()-1 );
   while( !waterObs.isEmpty() )
   {
      disconnect( waterObs.takeLast(), 0, this, 0 );
   }
   endRemoveRows();
}

void WaterTableModel::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;

   // Find the notifier in the list
   Water* waterSender = qobject_cast<Water*>(sender());
   if( waterSender )
   {
      i = waterObs.indexOf(waterSender);
      if( i >= 0 )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, WATERNUMCOLS-1));
      return;
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
      Brewtarget::logW(tr("Bad model index. row = %1").arg(index.row()));
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
         return QVariant(row->name());
      case WATERAMOUNTCOL:
         return QVariant( Brewtarget::displayAmount(row->amount_l(), Units::liters) );
      case WATERCALCIUMCOL:
         return QVariant( Brewtarget::displayAmount(row->calcium_ppm(), 0) );
      case WATERBICARBONATECOL:
         return QVariant( Brewtarget::displayAmount(row->bicarbonate_ppm(), 0) );
      case WATERSULFATECOL:
         return QVariant( Brewtarget::displayAmount(row->sulfate_ppm(), 0) );
      case WATERCHLORIDECOL:
         return QVariant( Brewtarget::displayAmount(row->chloride_ppm(), 0) );
      case WATERSODIUMCOL:
         return QVariant( Brewtarget::displayAmount(row->sodium_ppm(), 0) );
      case WATERMAGNESIUMCOL:
         return QVariant( Brewtarget::displayAmount(row->magnesium_ppm(), 0) );
      default :
         Brewtarget::logW(tr("Bad column: %1").arg(index.column()));
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
            Brewtarget::logW(tr("Bad column: %1").arg(section));
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
            row->setName(value.toString());
            return true;
         }
         else
            return false;
      case WATERAMOUNTCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setAmount_l( Brewtarget::volQStringToSI(value.toString()) );
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
         Brewtarget::logW(tr("Bad column: %1").arg(index.column()));
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
   QLineEdit* line = qobject_cast<QLineEdit*>(editor);
   line->setText(index.model()->data(index, Qt::DisplayRole).toString());
}

void WaterItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   QLineEdit* line = qobject_cast<QLineEdit*>(editor);

   if ( line->isModified() )
      model->setData(index, line->text(), Qt::EditRole);
}

void WaterItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
