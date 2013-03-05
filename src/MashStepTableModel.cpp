/*
 * MashStepTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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
#include <QTableView>
#include <QItemDelegate>
#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <QVector>
#include "database.h"
#include "mashstep.h"
#include "MashStepTableModel.h"
#include "unit.h"
#include "brewtarget.h"

MashStepTableModel::MashStepTableModel(QTableView* parent)
   : QAbstractTableModel(parent), mashObs(0), parentTableWidget(parent)
{
}

void MashStepTableModel::setMash( Mash* m )
{
   int i;
   if( mashObs )
   {
      beginRemoveRows( QModelIndex(), 0, steps.size()-1 );
      // Remove mashObs and all steps.
      disconnect( mashObs, 0, this, 0 );
      for( i = 0; i < steps.size(); ++i )
         disconnect( steps[i], 0, this, 0 );
      steps.clear();
      endRemoveRows();
   }
   
   mashObs = m;
   if( mashObs )
   {
      QList<MashStep*> tmpSteps = mashObs->mashSteps();
      beginInsertRows( QModelIndex(), 0, tmpSteps.size()-1 );
      //connect( mashObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(mashChanged(QMetaProperty,QVariant)) );
      connect( mashObs, SIGNAL(mashStepsChanged()), this, SLOT(mashChanged()) );
      steps = tmpSteps;
      for( i = 0; i < steps.size(); ++i )
         connect( steps[i], SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(mashStepChanged(QMetaProperty,QVariant)) );
      endInsertRows();
   }
   //reset(); // Tell everybody that the table has changed.
   
   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

MashStep* MashStepTableModel::getMashStep(unsigned int i)
{
   if( i < static_cast<unsigned int>(steps.size()) )
      return steps[i];
   else
      return 0;
}

void MashStepTableModel::mashChanged()
{
   // Remove and re-add all steps.
   setMash( mashObs );
}

void MashStepTableModel::mashStepChanged(QMetaProperty prop, QVariant val)
{
   int i;
   MashStep* stepSender = qobject_cast<MashStep*>(sender());
   if( stepSender && (i = steps.indexOf(stepSender)) >= 0 )
   {
      emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                        QAbstractItemModel::createIndex(i, MASHSTEPNUMCOLS-1));
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
   return steps.size();
}

int MashStepTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return MASHSTEPNUMCOLS;
}

QVariant MashStepTableModel::data( const QModelIndex& index, int role ) const
{
   MashStep* row;
   unitDisplay unit;
   unitScale scale;
   int col = index.column();

   if( mashObs == 0 )
      return QVariant();
   
   // Ensure the row is ok.
   if( index.row() >= (int)(steps.size()) )
   {
      Brewtarget::log(Brewtarget::WARNING, tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = steps[index.row()];

   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();

   switch( col )
   {
      case MASHSTEPNAMECOL:
         return QVariant(row->name());
      case MASHSTEPTYPECOL:
         return QVariant(row->typeStringTr());
      case MASHSTEPAMOUNTCOL:

         unit = displayUnit(col);
         scale = displayScale(col);

         return (row->type() == MashStep::Decoction)
                ? QVariant( Brewtarget::displayAmount(row->decoctionAmount_l(), Units::liters, 3, unit, scale ) )
                : QVariant( Brewtarget::displayAmount(row->infuseAmount_l(), Units::liters, 3, unit, scale) );
      case MASHSTEPTEMPCOL:
         unit = displayUnit(col);
         return (row->type() == MashStep::Decoction)
                ? QVariant("---")
                : QVariant( Brewtarget::displayAmount(row->infuseTemp_c(), Units::celsius, 3, unit, noScale) );
      case MASHSTEPTARGETTEMPCOL:
         unit = displayUnit(col);
         return QVariant( Brewtarget::displayAmount(row->stepTemp_c(), Units::celsius,3, unit, noScale) );
      case MASHSTEPTIMECOL:
         return QVariant( Brewtarget::displayAmount(row->stepTime_min(), Units::minutes) );
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
   unitDisplay unit;

   if( mashObs == 0 )
      return false;
   
   if( index.row() >= (int)(steps.size()) || role != Qt::EditRole )
      return false;
   else
      row = steps[index.row()];

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
            unit = displayUnit(MASHSTEPAMOUNTCOL);
            if( row->type() == MashStep::Decoction )
               row->setDecoctionAmount_l( Brewtarget::volQStringToSI(value.toString(),unit) );
            else
               row->setInfuseAmount_l( Brewtarget::volQStringToSI(value.toString(),unit) );
            return true;
         }
         else
            return false;
      case MASHSTEPTEMPCOL:
         if( value.canConvert(QVariant::String) && row->type() != MashStep::Decoction )
         {
            unit = displayUnit(MASHSTEPTEMPCOL);
            row->setInfuseTemp_c( Brewtarget::tempQStringToSI(value.toString(),unit) );
            return true;
         }
         else
            return false;
      case MASHSTEPTARGETTEMPCOL:
         if( value.canConvert(QVariant::String) )
         {
            unit = displayUnit(MASHSTEPTARGETTEMPCOL);
            row->setStepTemp_c( Brewtarget::tempQStringToSI(value.toString(),unit) );
            row->setEndTemp_c( Brewtarget::tempQStringToSI(value.toString(),unit) );
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

void MashStepTableModel::moveStepUp(int i)
{
   if( mashObs == 0 || i == 0 || i >= steps.size() )
      return;

   Database::instance().swapMashStepOrder( steps[i], steps[i-1] );
}

void MashStepTableModel::moveStepDown(int i)
{
   if( mashObs == 0 ||  i+1 >= steps.size() )
      return;

   Database::instance().swapMashStepOrder( steps[i], steps[i+1] );
}

unitDisplay MashStepTableModel::displayUnit(int column) const
{ 
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return noUnit;

   return (unitDisplay)Brewtarget::option(attribute, noUnit, this, Brewtarget::UNIT).toInt();
}

unitScale MashStepTableModel::displayScale(int column) const
{ 
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return noScale;

   return (unitScale)Brewtarget::option(attribute, noScale, this, Brewtarget::SCALE).toInt();
}

// We need to:
//   o clear the custom scale if set
//   o clear any custom unit from the rows
//      o which should have the side effect of clearing any scale
void MashStepTableModel::setDisplayUnit(int column, unitDisplay displayUnit) 
{
   // MashStep* row; // disabled per-cell magic
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayUnit,this,Brewtarget::UNIT); 
   Brewtarget::setOption(attribute,noScale,this,Brewtarget::SCALE);

   /* Disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getMashStep(i);
      row->setDisplayUnit(noUnit);
   }
   */
}

// Setting the scale should clear any cell-level scaling options
void MashStepTableModel::setDisplayScale(int column, unitScale displayScale) 
{ 
   // MashStep* row; //disabled per-cell magic

   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayScale,this,Brewtarget::SCALE); 

   /* disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getMashStep(i);
      row->setDisplayScale(noScale);
   }
   */
}

QString MashStepTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case MASHSTEPAMOUNTCOL:
         attribute = "amount";
         break;
      case MASHSTEPTEMPCOL:
         attribute = "infuseTemp_c";
         break;
      case MASHSTEPTARGETTEMPCOL:
         attribute = "stepTemp_c";
         break;
      default:
         attribute = "";
   }
   return attribute;
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
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      QString text = index.model()->data(index, Qt::DisplayRole).toString();

      int index = box->findText(text);
      box->setCurrentIndex(index);
   }
   else
   {
      QLineEdit* line = qobject_cast<QLineEdit*>(editor);

      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }

}

void MashStepItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   if( index.column() == MASHSTEPTYPECOL )
   {
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      int ndx = box->currentIndex();

      model->setData(index, ndx, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = qobject_cast<QLineEdit*>(editor);

      model->setData(index, line->text(), Qt::EditRole);
   }
}

void MashStepItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
