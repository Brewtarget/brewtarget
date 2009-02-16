/*
 * FermentableTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QAbstractItemView>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QSize>
#include <QComboBox>
#include <QLineEdit>

#include <vector>
#include <QString>
#include <vector>
#include <iostream>
#include "observable.h"
#include "fermentable.h"
#include "FermentableTableModel.h"

//=====================CLASS FermentableTableModel==============================
FermentableTableModel::FermentableTableModel(FermentableTableWidget* parent)
: QAbstractTableModel(parent), MultipleObserver()
{
   fermObs.clear();
   parentTableWidget = parent;
}

void FermentableTableModel::addFermentable(Fermentable* ferm)
{
   std::vector<Fermentable*>::iterator iter;
   
   //Check to see if it's already in the list
   for( iter=fermObs.begin(); iter != fermObs.end(); iter++ )
      if( *iter == ferm )
         return;
   
   fermObs.push_back(ferm);
   addObserved(ferm);
   reset(); // Tell everybody that the table has changed.
   
   if(parentTableWidget)
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

bool FermentableTableModel::removeFermentable(Fermentable* ferm)
{
   std::vector<Fermentable*>::iterator iter;
   
   for( iter=fermObs.begin(); iter != fermObs.end(); iter++ )
      if( *iter == ferm )
      {
         fermObs.erase(iter);
         removeObserved(ferm);
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

void FermentableTableModel::removeAll()
{
   unsigned int i;

   for( i = 0; i < fermObs.size(); ++i )
      removeObserved(fermObs[i]);

   fermObs.clear();
   reset();
}

void FermentableTableModel::notify(Observable* notifier)
{
   int i;
   
   // Find the notifier in the list
   for( i = 0; i < (int)fermObs.size(); ++i )
   {
      if( notifier == fermObs[i] )
      {
         /*
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, FERMNUMCOLS));
          */
         reset();
         break;
      }
   }
}

int FermentableTableModel::rowCount(const QModelIndex& parent) const
{
   return fermObs.size();
}

int FermentableTableModel::columnCount(const QModelIndex& parent) const
{
   return FERMNUMCOLS;
}

QVariant FermentableTableModel::data( const QModelIndex& index, int role ) const
{
   Fermentable* row;
   
   // Ensure the row is ok.
   if( index.row() >= (int)fermObs.size() )
   {
      std::cerr << "Bad model index. row = " << index.row() << std::endl;
      return QVariant();
   }
   else
      row = fermObs[index.row()];
   
   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();
   
   switch( index.column() )
   {
      case FERMNAMECOL:
         return QVariant(row->getName().c_str());
      case FERMTYPECOL:
         return QVariant(row->getType().c_str());
      case FERMAMOUNTCOL:
         return QVariant(row->getAmount_kg());
      case FERMYIELDCOL:
         return QVariant(row->getYield_pct());
      case FERMCOLORCOL:
         return QVariant(row->getColor_srm());
      default :
         std::cerr << "Bad column: " << index.column() << std::endl;
         return QVariant();
   }
}

QVariant FermentableTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case FERMNAMECOL:
            return QVariant("Name");
         case FERMTYPECOL:
            return QVariant("Type");
         case FERMAMOUNTCOL:
            return QVariant("Amount (kg)");
         case FERMYIELDCOL:
            return QVariant("Yield %");
         case FERMCOLORCOL:
            return QVariant("Color (SRM)");
         default:
            std::cerr << "Bad column: " << section << std::endl;
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags FermentableTableModel::flags(const QModelIndex& index ) const
{
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
          Qt::ItemIsEnabled;
}

bool FermentableTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Fermentable* row;
   
   if( index.row() >= (int)fermObs.size() || role != Qt::EditRole )
      return false;
   else
      row = fermObs[index.row()];
   
   switch( index.column() )
   {
      case FERMNAMECOL:
         if( value.canConvert(QVariant::String))
         {
            row->setName(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case FERMTYPECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setType(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case FERMAMOUNTCOL:
         if( value.canConvert(QVariant::Double) )
         {
            row->setAmount_kg(value.toDouble());
            return true;
         }
         else
            return false;
      case FERMYIELDCOL:
         if( value.canConvert(QVariant::Double) )
         {
            row->setYield_pct(value.toDouble());
            return true;
         }
         else
            return false;
      case FERMCOLORCOL:
         if( value.canConvert(QVariant::Double) )
         {
            row->setColor_srm(value.toDouble());
            return true;
         }
         else
            return false;
      default:
         std::cerr << "Bad column: " << index.column() << std::endl;
         return false;
   }
}

Fermentable* FermentableTableModel::getFermentable(unsigned int i)
{
   return fermObs[i];
}

//======================CLASS FermentableItemDelegate===========================

FermentableItemDelegate::FermentableItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}
        
QWidget* FermentableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   if( index.column() == FERMTYPECOL )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem("Grain");
      box->addItem("Sugar");
      box->addItem("Extract");
      box->addItem("Dry Extract");
      box->addItem("Adjunct");
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }
   else
      return new QLineEdit(parent);
}

void FermentableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   if( index.column() == FERMTYPECOL )
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

void FermentableItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   if( index.column() == FERMTYPECOL )
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

void FermentableItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   editor->setGeometry(option.rect);
}
