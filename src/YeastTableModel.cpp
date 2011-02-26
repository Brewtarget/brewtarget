/*
 * YeastTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QComboBox>
#include <QLineEdit>
#include <QString>

#include <QVector>
#include <iostream>
#include "yeast.h"
#include "observable.h"
#include "YeastTableModel.h"
#include "unit.h"
#include "brewtarget.h"

YeastTableModel::YeastTableModel(YeastTableWidget* parent)
: QAbstractTableModel(parent), MultipleObserver()
{
   yeastObs.clear();
   parentTableWidget = parent;
}

void YeastTableModel::addYeast(Yeast* yeast)
{
   QVector<Yeast*>::iterator iter;

   //Check to see if it's already in the list
   for( iter=yeastObs.begin(); iter != yeastObs.end(); iter++ )
      if( *iter == yeast )
         return;

   yeastObs.push_back(yeast);
   addObserved(yeast);
   reset(); // Tell everybody that the table has changed.
   
   if(parentTableWidget)
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

bool YeastTableModel::removeYeast(Yeast* yeast)
{
   QVector<Yeast*>::iterator iter;

   for( iter=yeastObs.begin(); iter != yeastObs.end(); iter++ )
      if( *iter == yeast )
      {
         yeastObs.erase(iter);
         removeObserved(yeast);
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

void YeastTableModel::removeAll()
{
   int i;

   for( i = 0; i < yeastObs.size(); ++i )
      removeObserved(yeastObs[i]);

   yeastObs.clear();
   reset();
}

void YeastTableModel::notify(Observable* notifier, QVariant info)
{
   int i;

   // Find the notifier in the list
   for( i = 0; i < (int)yeastObs.size(); ++i )
   {
      if( notifier == yeastObs[i] )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, YEASTNUMCOLS));
   }
}

int YeastTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return yeastObs.size();
}

int YeastTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return YEASTNUMCOLS;
}

QVariant YeastTableModel::data( const QModelIndex& index, int role ) const
{
   Yeast* row;

   // Ensure the row is ok.
   if( index.row() >= (int)yeastObs.size() )
   {
      Brewtarget::log(Brewtarget::WARNING, tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = yeastObs[index.row()];

   switch( index.column() )
   {
      case YEASTNAMECOL:
      if( role == Qt::DisplayRole )
         return QVariant(row->getName());
      else
         return QVariant();
      case YEASTTYPECOL:
      if( role == Qt::DisplayRole )
         return QVariant(row->getTypeStringTr());
      else if( role == Qt::UserRole )
         return QVariant(row->getType());
      else
         return QVariant();
      case YEASTLABCOL:
      if( role == Qt::DisplayRole )
         return QVariant(row->getLaboratory());
      else
         return QVariant();
      case YEASTPRODIDCOL:
      if( role == Qt::DisplayRole )
         return QVariant(row->getProductID());
      else
         return QVariant();
      case YEASTFORMCOL:
      if( role == Qt::DisplayRole )
         return QVariant(row->getFormStringTr());
      else if( role == Qt::UserRole )
         return QVariant(row->getForm());
      else
         return QVariant();
      case YEASTAMOUNTCOL:
      if( role == Qt::DisplayRole )
         return QVariant( Brewtarget::displayAmount(row->getAmount(), row->getAmountIsWeight()? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
      else
         return QVariant();
      default :
         Brewtarget::log(Brewtarget::WARNING, tr("Bad column: %1").arg(index.column()));
         return QVariant();
   }
}

QVariant YeastTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case YEASTNAMECOL:
            return QVariant(tr("Name"));
         case YEASTTYPECOL:
            return QVariant(tr("Type"));
         case YEASTFORMCOL:
            return QVariant(tr("Form"));
         case YEASTAMOUNTCOL:
            return QVariant(tr("Amount"));
         case YEASTLABCOL:
             return QVariant(tr("Laboratory"));
         case YEASTPRODIDCOL:
             return QVariant(tr("Product ID"));
         default:
            Brewtarget::log(Brewtarget::WARNING, tr("Bad column: %1").arg(section));
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags YeastTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();
   switch(col)
   {
      case YEASTNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      default:
         return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool YeastTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Yeast *row;

   if( index.row() >= (int)yeastObs.size() || role != Qt::EditRole )
      return false;
   else
      row = yeastObs[index.row()];

   switch( index.column() )
   {
      case YEASTNAMECOL:
         if( value.canConvert(QVariant::String))
         {
            row->setName(value.toString());
            return true;
         }
         else
            return false;
      case YEASTLABCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setLaboratory(value.toString());
            return true;
         }
         else
            return false;
      case YEASTPRODIDCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setProductID(value.toString());
            return true;
         }
         else
            return false;
      case YEASTTYPECOL:
         if( value.canConvert(QVariant::Int) )
         {
            row->setType(static_cast<Yeast::Type>(value.toInt()));
            return true;
         }
         else
            return false;
      case YEASTFORMCOL:
         if( value.canConvert(QVariant::Int) )
         {
            row->setForm(static_cast<Yeast::Form>(value.toInt()));
            return true;
         }
         else
            return false;
      case YEASTAMOUNTCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setAmount( row->getAmountIsWeight() ? Brewtarget::weightQStringToSI(value.toString()) : Brewtarget::volQStringToSI(value.toString()) );
            return true;
         }
         else
            return false;
      default:
         Brewtarget::log(Brewtarget::WARNING, tr("Bad column: %1").arg(index.column()));
         return false;
   }
}

Yeast* YeastTableModel::getYeast(unsigned int i)
{
   return yeastObs[i];
}

//==========================CLASS YeastItemDelegate===============================

YeastItemDelegate::YeastItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}

QWidget* YeastItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const
{
   int col = index.column();

   if( col == YEASTTYPECOL )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Ale"));
      box->addItem(tr("Lager"));
      box->addItem(tr("Wheat"));
      box->addItem(tr("Wine"));
      box->addItem(tr("Champagne"));
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   }
   else if( col == YEASTFORMCOL )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Liquid"));
      box->addItem(tr("Dry"));
      box->addItem(tr("Slant"));
      box->addItem(tr("Culture"));

      return box;
   }
   else
      return new QLineEdit(parent);
}

void YeastItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   int col = index.column();

   if( col == YEASTTYPECOL || col == YEASTFORMCOL )
   {
      QComboBox* box = (QComboBox*)editor;
      int ndx = index.model()->data(index, Qt::UserRole).toInt();

      box->setCurrentIndex(ndx);
   }
   else
   {
      QLineEdit* line = (QLineEdit*)editor;

      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }

}

void YeastItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   int col = index.column();

   if( col == YEASTTYPECOL || col == YEASTFORMCOL )
   {
      QComboBox* box = (QComboBox*)editor;
      int value = box->currentIndex();

      model->setData(index, value, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = (QLineEdit*)editor;

      model->setData(index, line->text(), Qt::EditRole);
   }
}

void YeastItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
