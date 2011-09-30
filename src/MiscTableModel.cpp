/*
 * MiscTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QString>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QComboBox>
#include <QLineEdit>
#include <QVector>
#include <iostream>
#include "misc.h"
#include "observable.h"
#include "MiscTableModel.h"
#include "unit.h"
#include "brewtarget.h"

MiscTableModel::MiscTableModel(MiscTableWidget* parent)
   : QAbstractTableModel(parent), MultipleObserver()
{
   miscObs.clear();
   parentTableWidget = parent;
}

void MiscTableModel::addMisc(Misc* misc)
{
   int i;
   // Check to see if it's already in the list.
   for( i = 0; i < miscObs.size(); ++i )
      if( miscObs[i] == misc )
         return;
   
   miscObs.push_back(misc);
   addObserved(misc);
   reset();
   
   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

// Returns true when misc is successfully found and removed.
bool MiscTableModel::removeMisc(Misc* misc)
{
   QVector<Misc*>::iterator iter;
   
   for( iter=miscObs.begin(); iter != miscObs.end(); iter++ )
      if( *iter == misc )
      {
         miscObs.erase(iter);
         removeObserved(misc);
         reset();

         if( parentTableWidget )
         {
            parentTableWidget->resizeColumnsToContents();
            parentTableWidget->resizeRowsToContents();
         }
         
         return true;
      }
   
   return false;
}

void MiscTableModel::removeAll()
{
   int i;

   for( i = 0; i < miscObs.size(); ++i )
      removeObserved(miscObs[i]);

   miscObs.clear();
   reset();
}

int MiscTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return miscObs.size();
}

int MiscTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return MISCNUMCOLS;
}

QVariant MiscTableModel::data( const QModelIndex& index, int role ) const
{
   Misc* row;
   
   // Ensure the row is ok.
   if( index.row() >= (int)miscObs.size() )
   {
      Brewtarget::log(Brewtarget::WARNING, tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = miscObs[index.row()];
   
   // Deal with the column and return the right data.
   if( index.column() == MISCNAMECOL )
   {
      if( role == Qt::DisplayRole )
         return QVariant(row->getName());
      else
         return QVariant();
   }
   else if( index.column() == MISCTYPECOL )
   {
      if( role == Qt::DisplayRole )
         return QVariant(row->getTypeStringTr());
      else if( role == Qt::UserRole )
         return QVariant(row->getType());
      else
         return QVariant();
   }
   else if( index.column() == MISCUSECOL )
   {
      if( role == Qt::DisplayRole )
         return QVariant(row->getUseStringTr());
      else if( role == Qt::UserRole )
         return QVariant(row->getUse());
      else
         return QVariant();
   }
   else if( index.column() == MISCTIMECOL )
   {
      if( role == Qt::DisplayRole )
         return QVariant( Brewtarget::displayAmount(row->getTime(), Units::minutes) );
      else
         return QVariant();
   }
   else if( index.column() == MISCAMOUNTCOL )
   {
      if( role == Qt::DisplayRole )
         return QVariant( Brewtarget::displayAmount(row->getAmount(), row->getAmountIsWeight()? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
      else
         return QVariant();
   }
   else
   {
      Brewtarget::log(Brewtarget::WARNING, QString("Bad model index. column = %1").arg(index.column()));
      return QVariant();
   }
}

QVariant MiscTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case MISCNAMECOL:
            return QVariant(tr("Name"));
         case MISCTYPECOL:
            return QVariant(tr("Type"));
         case MISCUSECOL:
            return QVariant(tr("Use"));
         case MISCTIMECOL:
            return QVariant(tr("Time"));
         case MISCAMOUNTCOL:
            return QVariant(tr("Amount"));
         default:
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags MiscTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();
   switch( col )
   {
      case MISCNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      default:
         return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool MiscTableModel::setData( const QModelIndex& index, const QVariant& value, int /*role*/ )
{
   Misc *row;
   int col;
   QString tmpStr;
   
   if( index.row() >= (int)miscObs.size() )
      return false;
   else
      row = miscObs[index.row()];
   
   col = index.column();
   
   if( col == MISCNAMECOL )
   {
      if( value.canConvert(QVariant::String) )
      {
         tmpStr = value.toString();
         row->setName(tmpStr);
      }
      else
         return false;
   }
   else if( col == MISCTYPECOL )
   {
      if( value.canConvert(QVariant::Int) )
      {
         row->setType( static_cast<Misc::Type>(value.toInt()) );
      }
      else
         return false;
   }
   else if( col == MISCUSECOL )
   {
      if( value.canConvert(QVariant::Int) )
      {
         row->setUse( static_cast<Misc::Use>(value.toInt()) );
      }
      else
         return false;
   }
   else if( col == MISCTIMECOL )
   {
      if( value.canConvert(QVariant::String) )
         row->setTime( Brewtarget::timeQStringToSI(value.toString()) );
      else
         return false;
   }
   else if( col == MISCAMOUNTCOL )
   {
      if( value.canConvert(QVariant::String) )
         row->setAmount( row->getAmountIsWeight() ? Brewtarget::weightQStringToSI(value.toString()) : Brewtarget::volQStringToSI(value.toString()) );
      else
         return false;
   }
   else
      return false;
   
   emit dataChanged( index, index );
   return true;
}

void MiscTableModel::notify(Observable* notifier, QVariant info) // Gets called when an observable changes.
{
   int i;
   
   for( i = 0; i < (int)miscObs.size(); ++i )
      if( notifier == miscObs[i] )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, MISCNUMCOLS) );
}

Misc* MiscTableModel::getMisc(unsigned int i)
{
   return miscObs[i];
}

//======================CLASS MiscItemDelegate===========================

MiscItemDelegate::MiscItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}
        
QWidget* MiscItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
   if( index.column() == MISCTYPECOL )
   {
      QComboBox *box = new QComboBox(parent);
      box->addItem(tr("Spice"));
      box->addItem(tr("Fining"));
      box->addItem(tr("Water Agent"));
      box->addItem(tr("Herb"));
      box->addItem(tr("Flavor"));
      box->addItem(tr("Other"));
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }
   else if( index.column() == MISCUSECOL )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Boil"));
      box->addItem(tr("Mash"));
      box->addItem(tr("Primary"));
      box->addItem(tr("Secondary"));
      box->addItem(tr("Bottling"));
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }
   else
      return new QLineEdit(parent);
}

void MiscItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   int column = index.column();

   if( column == MISCTYPECOL || column == MISCUSECOL )
   {
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      if( box == 0 )
         return;
      box->setCurrentIndex(index.model()->data(index, Qt::UserRole).toInt());
   }
   else
   {
      QLineEdit* line = (QLineEdit*)editor;
      
      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }
   
}

void MiscItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   int column = index.column();
   if( column == MISCTYPECOL || column == MISCUSECOL )
   {
      QComboBox* box = (QComboBox*)editor;
      int ndx = box->currentIndex();
      
      model->setData(index, ndx, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = (QLineEdit*)editor;
      
      model->setData(index, line->text(), Qt::EditRole);
   }
}

void MiscItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   editor->setGeometry(option.rect);
}

