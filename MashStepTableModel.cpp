/*
 * MashStepTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QItemDelegate>
#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <vector>
#include <iostream>
#include "mashstep.h"
#include "observable.h"
#include "MashStepTableModel.h"
#include "unit.h"
#include "brewtarget.h"

MashStepTableModel::MashStepTableModel(MashStepTableWidget* parent)
: QAbstractTableModel(parent), MultipleObserver()
{
   stepObs.clear();
   parentTableWidget = parent;
}

void MashStepTableModel::addMashStep(MashStep* step)
{
   std::vector<MashStep*>::iterator iter;

   //Check to see if it's already in the list
   for( iter=stepObs.begin(); iter != stepObs.end(); iter++ )
      if( *iter == step )
         return;

   stepObs.push_back(step);
   addObserved(step);
   reset(); // Tell everybody that the table has changed.

   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

bool MashStepTableModel::removeMashStep(MashStep* step)
{
   std::vector<MashStep*>::iterator iter;

   for( iter=stepObs.begin(); iter != stepObs.end(); iter++ )
      if( *iter == step )
      {
         stepObs.erase(iter);
         removeObserved(step);
         reset(); // Tell everybody the table has changed.
         
         if( parentTableWidget )
         {
            parentTableWidget->resizeColumnsToContents();
            parentTableWidget->resizeRowsToContents();
         }
         
         return true;
      }

   return false;
}

void MashStepTableModel::removeAll()
{
   unsigned int i;

   for( i = 0; i < stepObs.size(); ++i )
      removeObserved(stepObs[i]);

   stepObs.clear();
   reset();
}

void MashStepTableModel::notify(Observable* notifier, QVariant info)
{
   int i;

   // Find the notifier in the list
   for( i = 0; i < (int)stepObs.size(); ++i )
   {
      if( notifier == stepObs[i] )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, MASHSTEPNUMCOLS));
   }
}

int MashStepTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return stepObs.size();
}

int MashStepTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return MASHSTEPNUMCOLS;
}

QVariant MashStepTableModel::data( const QModelIndex& index, int role ) const
{
   MashStep* row;

   // Ensure the row is ok.
   if( index.row() >= (int)stepObs.size() )
   {
      std::cerr << "Bad model index. row = " << index.row() << std::endl;
      return QVariant();
   }
   else
      row = stepObs[index.row()];

   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();

   switch( index.column() )
   {
      case MASHSTEPNAMECOL:
         return QVariant(row->getName().c_str());
      case MASHSTEPTYPECOL:
         return QVariant(row->getType().c_str());
      case MASHSTEPAMOUNTCOL:
         return QVariant( Brewtarget::displayAmount(row->getInfuseAmount_l(), Units::liters) );
      case MASHSTEPTEMPCOL:
         return QVariant( Brewtarget::displayAmount(row->getStepTemp_c(), Units::celsius) );
      case MASHSTEPTIMECOL:
         return QVariant( Brewtarget::displayAmount(row->getStepTime_min(), Units::minutes) );
      default :
         std::cerr << "Bad column: " << index.column() << std::endl;
         return QVariant();
   }
}

QVariant MashStepTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case MASHSTEPNAMECOL:
            return QVariant("Name");
         case MASHSTEPTYPECOL:
            return QVariant("Type");
         case MASHSTEPAMOUNTCOL:
            return QVariant("Amount");
         case MASHSTEPTEMPCOL:
            return QVariant("Temp");
         case MASHSTEPTIMECOL:
            return QVariant("Time");
         default:
            std::cerr << "Bad column: " << section << std::endl;
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags MashStepTableModel::flags(const QModelIndex& /*index*/ ) const
{
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
          Qt::ItemIsEnabled;
}

bool MashStepTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   MashStep *row;

   if( index.row() >= (int)stepObs.size() || role != Qt::EditRole )
      return false;
   else
      row = stepObs[index.row()];

   switch( index.column() )
   {
      case MASHSTEPNAMECOL:
         if( value.canConvert(QVariant::String))
         {
            row->setName(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case MASHSTEPTYPECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setType(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case MASHSTEPAMOUNTCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setInfuseAmount_l( Unit::qstringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      case MASHSTEPTEMPCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setStepTemp_c( Unit::qstringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      case MASHSTEPTIMECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setStepTime_min( Unit::qstringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      default:
         std::cerr << "Bad column: " << index.column() << std::endl;
         return false;
   }
}

//==========================CLASS MashStepItemDelegate===============================

MashStepItemDelegate::MashStepItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}

QWidget* MashStepItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
   if( index.column() == MASHSTEPTYPECOL )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem("Infusion");
      box->addItem("Temperature");
      box->addItem("Decoction");
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   }
   else
      return new QLineEdit(parent);
}

void MashStepItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   if( index.column() == MASHSTEPTYPECOL )
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

void MashStepItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   if( index.column() == MASHSTEPTYPECOL )
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

void MashStepItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}

