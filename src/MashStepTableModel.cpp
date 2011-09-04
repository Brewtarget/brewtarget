/*
 * MashStepTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QItemDelegate>
#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <QVector>
#include <iostream>
#include "mashstep.h"
#include "observable.h"
#include "MashStepTableModel.h"
#include "unit.h"
#include "brewtarget.h"

MashStepTableModel::MashStepTableModel(MashStepTableWidget* parent)
: QAbstractTableModel(parent), Observer()
{
   parentTableWidget = parent;
   mashObs = 0;
}

void MashStepTableModel::setMash( Mash* m )
{
   mashObs = m;
   setObserved(mashObs);
   reset(); // Tell everybody that the table has changed.
   
   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

/***************************************************
void MashStepTableModel::addMashStep(MashStep* step)
{
   QVector<MashStep*>::iterator iter;

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
   QVector<MashStep*>::iterator iter;

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

MashStep* MashStepTableModel::getMashStep(unsigned int i)
{
   return stepObs[i];
}
*************************************************************/

void MashStepTableModel::notify(Observable* notifier, QVariant info)
{
   int i;
   bool ok = false;
   
   if( notifier != mashObs )
      return;
   
   i = info.toInt(&ok); // mashObs' info says which MashStep changed.
   
   if( ok )
   {
      emit dataChanged( QAbstractItemModel::createIndex(i, 0),
         QAbstractItemModel::createIndex(i, MASHSTEPNUMCOLS));
   }
   else
      reset();
   
   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

int MashStepTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   if( mashObs != 0 )
      return mashObs->getNumMashSteps();
   else
      return 0;
}

int MashStepTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return MASHSTEPNUMCOLS;
}

QVariant MashStepTableModel::data( const QModelIndex& index, int role ) const
{
   MashStep* row;

   if( mashObs == 0 )
      return QVariant();
   
   // Ensure the row is ok.
   if( index.row() >= (int)(mashObs->getNumMashSteps()) )
   {
      Brewtarget::log(Brewtarget::WARNING, tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = mashObs->getMashStep(index.row());

   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();

   switch( index.column() )
   {
      case MASHSTEPNAMECOL:
         return QVariant(row->getName());
      case MASHSTEPTYPECOL:
         return QVariant(row->getTypeStringTr());
      case MASHSTEPAMOUNTCOL:
         return (row->getType() == MashStep::TYPEDECOCTION)
                ? QVariant( Brewtarget::displayAmount(row->getDecoctionAmount_l(), Units::liters ) )
                : QVariant( Brewtarget::displayAmount(row->getInfuseAmount_l(), Units::liters) );
      case MASHSTEPTEMPCOL:
         return (row->getType() == MashStep::TYPEDECOCTION)
                ? QVariant("---")
                : QVariant( Brewtarget::displayAmount(row->getInfuseTemp_c(), Units::celsius) );
      case MASHSTEPTARGETTEMPCOL:
         return QVariant( Brewtarget::displayAmount(row->getStepTemp_c(), Units::celsius) );
      case MASHSTEPTIMECOL:
         return QVariant( Brewtarget::displayAmount(row->getStepTime_min(), Units::minutes) );
      default :
         Brewtarget::log(Brewtarget::WARNING, tr("Bad column: %1").arg(index.column()));
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
            return QVariant(tr("Name"));
         case MASHSTEPTYPECOL:
            return QVariant(tr("Type"));
         case MASHSTEPAMOUNTCOL:
            return QVariant(tr("Amount"));
         case MASHSTEPTEMPCOL:
            return QVariant(tr("Infusion Temp"));
         case MASHSTEPTARGETTEMPCOL:
            return QVariant(tr("Target Temp"));
         case MASHSTEPTIMECOL:
            return QVariant(tr("Time"));
         default:
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags MashStepTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();
   switch(col)
   {
      case MASHSTEPNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      default:
         return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool MashStepTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   MashStep *row;

   if( mashObs == 0 )
      return false;
   
   if( index.row() >= (int)(mashObs->getNumMashSteps()) || role != Qt::EditRole )
      return false;
   else
      row = mashObs->getMashStep(index.row());

   switch( index.column() )
   {
      case MASHSTEPNAMECOL:
         if( value.canConvert(QVariant::String))
         {
            row->setName(value.toString());
            return true;
         }
         else
            return false;
      case MASHSTEPTYPECOL:
         if( value.canConvert(QVariant::Int) )
         {
            row->setType(static_cast<MashStep::Type>(value.toInt()));
            return true;
         }
         else
            return false;
      case MASHSTEPAMOUNTCOL:
         if( value.canConvert(QVariant::String) )
         {
            if( row->getType() == MashStep::TYPEDECOCTION )
               row->setDecoctionAmount_l( Brewtarget::volQStringToSI(value.toString()) );
            else
               row->setInfuseAmount_l( Brewtarget::volQStringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      case MASHSTEPTEMPCOL:
         if( value.canConvert(QVariant::String) && row->getType() != MashStep::TYPEDECOCTION )
         {
            row->setInfuseTemp_c( Brewtarget::tempQStringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      case MASHSTEPTARGETTEMPCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setStepTemp_c( Brewtarget::tempQStringToSI(value.toString()) );
            row->setEndTemp_c( Brewtarget::tempQStringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      case MASHSTEPTIMECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setStepTime_min( Brewtarget::timeQStringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      default:
         return false;
   }
}

void MashStepTableModel::moveStepUp(unsigned int i)
{
   if( mashObs == 0 || i == 0 || i >= mashObs->getNumMashSteps() )
      return;

   mashObs->swapSteps(i, i-1);
}

void MashStepTableModel::moveStepDown(unsigned int i)
{
   // i is an unsigned int. How can i be less than 0?
   if( mashObs == 0 || i < 0 || i+1 >= mashObs->getNumMashSteps() )
      return;

   mashObs->swapSteps(i, i+1);
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

