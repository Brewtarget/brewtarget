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

#include <vector>
#include <iostream>
#include "yeast.h"
#include "observable.h"
#include "YeastTableModel.h"

YeastTableModel::YeastTableModel(QWidget* parent)
: QAbstractTableModel(parent), MultipleObserver()
{
   yeastObs.clear();
}

void YeastTableModel::addYeast(Yeast* yeast)
{
   std::vector<Yeast*>::iterator iter;

   //Check to see if it's already in the list
   for( iter=yeastObs.begin(); iter != yeastObs.end(); iter++ )
      if( *iter == yeast )
         return;

   yeastObs.push_back(yeast);
   addObserved(yeast);
   reset(); // Tell everybody that the table has changed.
}

bool YeastTableModel::removeYeast(Yeast* yeast)
{
   std::vector<Yeast*>::iterator iter;

   for( iter=yeastObs.begin(); iter != yeastObs.end(); iter++ )
      if( *iter == yeast )
      {
         yeastObs.erase(iter);
         removeObserved(yeast);
         reset(); // Tell everybody the table has changed.
         return true;
      }

   return false;
}

void YeastTableModel::removeAll()
{
   unsigned int i;

   for( i = 0; i < yeastObs.size(); ++i )
      removeObserved(yeastObs[i]);

   yeastObs.clear();
   reset();
}

void YeastTableModel::notify(Observable* notifier)
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
      std::cerr << "Bad model index. row = " << index.row() << std::endl;
      return QVariant();
   }
   else
      row = yeastObs[index.row()];

   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();

   switch( index.column() )
   {
      case YEASTNAMECOL:
         return QVariant(row->getName().c_str());
      case YEASTTYPECOL:
         return QVariant(row->getType().c_str());
      case YEASTFORMCOL:
         return QVariant(row->getForm().c_str());
      case YEASTAMOUNTCOL:
         return QVariant(row->getAmount());
      default :
         std::cerr << "Bad column: " << index.column() << std::endl;
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
            return QVariant("Name");
         case YEASTTYPECOL:
            return QVariant("Type");
         case YEASTFORMCOL:
            return QVariant("Form");
         case YEASTAMOUNTCOL:
            return QVariant("Amount (kg or l)");
         default:
            std::cerr << "Bad column: " << section << std::endl;
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags YeastTableModel::flags(const QModelIndex& /*index*/ ) const
{
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
          Qt::ItemIsEnabled;
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
            row->setName(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case YEASTTYPECOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setType(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case YEASTFORMCOL:
         if( value.canConvert(QVariant::String) )
         {
            row->setForm(value.toString().toStdString());
            return true;
         }
         else
            return false;
      case YEASTAMOUNTCOL:
         if( value.canConvert(QVariant::Double) )
         {
            row->setAmount(value.toDouble());
            return true;
         }
         else
            return false;
      default:
         std::cerr << "Bad column: " << index.column() << std::endl;
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

      /*May be “Ale”, “Lager”, “Wheat”, “Wine” or “Champagne”*/
      box->addItem("Ale");
      box->addItem("Lager");
      box->addItem("Wheat");
      box->addItem("Wine");
      box->addItem("Champagne");
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   }
   else if( col == YEASTFORMCOL )
   {
      QComboBox *box = new QComboBox(parent);

      /*May be “Liquid”, “Dry”, “Slant” or “Culture”*/
      box->addItem("Liquid");
      box->addItem("Dry");
      box->addItem("Slant");
      box->addItem("Culture");

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

void YeastItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   int col = index.column();

   if( col == YEASTTYPECOL || col == YEASTFORMCOL )
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

void YeastItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
