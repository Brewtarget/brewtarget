/*
 * MiscTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QString>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QComboBox>
#include <QLineEdit>
#include <vector>
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
   unsigned int i;
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
   std::vector<Misc*>::iterator iter;
   
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
   unsigned int i;

   for( i = 0; i < miscObs.size(); ++i )
      removeObserved(miscObs[i]);

   miscObs.clear();
   reset();
}

int MiscTableModel::rowCount(const QModelIndex& parent) const
{
   return miscObs.size();
}

int MiscTableModel::columnCount(const QModelIndex& parent) const
{
   return 5;
}

QVariant MiscTableModel::data( const QModelIndex& index, int role ) const
{
   Misc* row;
   
   // Ensure the row is ok.
   if( index.row() >= (int)miscObs.size() )
   {
      std::cerr << "Bad model index. row = " << index.row() << std::endl;
      return QVariant();
   }
   else
      row = miscObs[index.row()];
   
   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();
   
   // Deal with the column and return the right data.
   if( index.column() == MISCNAMECOL )
   {
      return QVariant(row->getName().c_str());
   }
   else if( index.column() == MISCTYPECOL )
   {
      return QVariant(row->getType().c_str());
   }
   else if( index.column() == MISCUSECOL )
   {
      return QVariant(row->getUse().c_str());
   }
   else if( index.column() == MISCTIMECOL )
   {
      return QVariant( Brewtarget::displayAmount(row->getTime(), Units::minutes) );
   }
   else if( index.column() == MISCAMOUNTCOL )
   {
      return QVariant( Brewtarget::displayAmount(row->getAmount(), row->getAmountIsWeight()? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
   }
   else
   {
      std::cerr << "Bad model index. column = " << index.column() << std::endl;
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
            return QVariant("Name");
         case MISCTYPECOL:
            return QVariant("Type");
         case MISCUSECOL:
            return QVariant("Use");
         case MISCTIMECOL:
            return QVariant("Time");
         case MISCAMOUNTCOL:
            return QVariant("Amount");
         default:
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags MiscTableModel::flags(const QModelIndex& /*index*/ ) const
{
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
          Qt::ItemIsEnabled;
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
         row->setName(tmpStr.toStdString());
      }
      else
         return false;
   }
   else if( col == MISCTYPECOL )
   {
      if( value.canConvert(QVariant::String) )
      {
         tmpStr = value.toString();
         row->setType(tmpStr.toStdString());
      }
      else
         return false;
   }
   else if( col == MISCUSECOL )
   {
      if( value.canConvert(QVariant::String) )
      {
         tmpStr = value.toString();
         row->setUse(tmpStr.toStdString());
      }
      else
         return false;
   }
   else if( col == MISCTIMECOL )
   {
      if( value.canConvert(QVariant::String) )
         row->setTime( Unit::qstringToSI(value.toString()) );
      else
         return false;
   }
   else if( col == MISCAMOUNTCOL )
   {
      if( value.canConvert(QVariant::String) )
         row->setAmount( Unit::qstringToSI(value.toString()) );
      else
         return false;
   }
   else
      return false;
   
   emit dataChanged( index, index );
   return true;
}

void MiscTableModel::notify(Observable* notifier) // Gets called when an observable changes.
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
        
QWidget* MiscItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   if( index.column() == MISCTYPECOL )
   {
      QComboBox *box = new QComboBox(parent);
      box->addItem("Spice");
      box->addItem("Fining");
      box->addItem("Water Agent");
      box->addItem("Herb");
      box->addItem("Flavor");
      box->addItem("Other");
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }
   else if( index.column() == MISCUSECOL )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem("Boil");
      box->addItem("Mash");
      box->addItem("Primary");
      box->addItem("Secondary");
      box->addItem("Bottling");
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }
   else
      return new QLineEdit(parent);
}

void MiscItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   int column = index.column();
   
   if( column == MISCTYPECOL ||  column == MISCUSECOL )
   {
      QComboBox* box = (QComboBox*)editor;
      QString text = index.model()->data(index, Qt::DisplayRole).toString();
      
      int index = box->findText(text);
      box->setCurrentIndex(index);
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
      QString value = box->currentText();
      
      model->setData(index, value, Qt::EditRole);
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

