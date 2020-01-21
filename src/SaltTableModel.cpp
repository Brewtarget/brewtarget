/*
 * SaltTableModel.cpp is part of Brewtarget, and is Copyright the following
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
#include "SaltTableModel.h"
#include "SaltTableWidget.h"
#include "salt.h"
#include "unit.h"
#include "recipe.h"
#include "brewtarget.h"

SaltTableModel::SaltTableModel(QTableView* parent)
   : QAbstractTableModel(parent),
     recObs(nullptr)
{
}

void SaltTableModel::observeRecipe(Recipe* rec)
{
   if( recObs )
   {
      disconnect( recObs, nullptr, this, nullptr );
      removeAll();
   }

   recObs = rec;
   if( recObs )
   {
      connect( recObs, &BeerXMLElement::changed, this, &SaltTableModel::changed );
      addSalts( recObs->salts() );
   }
}

void SaltTableModel::observeDatabase(bool val)
{
   if( val )
   {
      observeRecipe(nullptr);
      removeAll();
      connect( &(Database::instance()), &Database::newSaltSignal, this, &SaltTableModel::addSalt );
      connect( &(Database::instance()), SIGNAL(deletedSignal(Salt*)), this, SLOT(removeSalt(Salt*)) );
      addSalts( Database::instance().salts() );
   }
   else
   {
      removeAll();
      disconnect( &(Database::instance()), nullptr, this, nullptr );
   }
}

void SaltTableModel::addSalt(Salt* salt)
{
   if( saltObs.contains(salt) )
      return;
   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if(
      recObs == nullptr &&
      (
         salt->deleted() ||
         !salt->display()
      )
   )
      return;

   beginInsertRows( QModelIndex(), saltObs.size(), saltObs.size() );
   saltObs.append(salt);
   connect( salt, &BeerXMLElement::changed, this, &SaltTableModel::changed );
   endInsertRows();

   if(parentTableWidget)
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void SaltTableModel::addSalts(QList<Salt*> salts)
{
   QList<Salt*>::iterator i;
   QList<Salt*> tmp;

   for( i = salts.begin(); i != salts.end(); i++ )
   {
      if( !saltObs.contains(*i) )
         tmp.append(*i);
   }

   int size = saltObs.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      saltObs.append(tmp);

      for( i = tmp.begin(); i != tmp.end(); i++ )
         connect( *i, &BeerXMLElement::changed, this, &SaltTableModel::changed );

      endInsertRows();
   }

   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }

}

void SaltTableModel::removeSalt(Salt* salt)
{
   int i;

   i = saltObs.indexOf(salt);
   if( i >= 0 )
   {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( salt, nullptr, this, nullptr );
      saltObs.removeAt(i);
      endRemoveRows();

      if(parentTableWidget)
      {
         parentTableWidget->resizeColumnsToContents();
         parentTableWidget->resizeRowsToContents();
      }
   }
}

void SaltTableModel::removeAll()
{
   beginRemoveRows( QModelIndex(), 0, saltObs.size()-1 );
   while( !saltObs.isEmpty() )
   {
      disconnect( saltObs.takeLast(), nullptr, this, nullptr );
   }
   endRemoveRows();
}

void SaltTableModel::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;

   // Find the notifier in the list
   Salt* saltSender = qobject_cast<Salt*>(sender());
   if( saltSender )
   {
      i = saltObs.indexOf(saltSender);
      if( i >= 0 )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, SALTNUMCOLS-1));
      return;
   }
}

int SaltTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return saltObs.size();
}

int SaltTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return SALTNUMCOLS;
}

QVariant SaltTableModel::data( const QModelIndex& index, int role ) const
{
   Salt* row;

   // Ensure the row is ok.
   if( index.row() >= static_cast<int>(saltObs.size()) )
   {
      Brewtarget::logW(tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = saltObs[index.row()];

   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();

   switch( index.column() )
   {
      case SALTNAMECOL:
         return QVariant(row->name());
      case SALTAMOUNTCOL:
         return QVariant( Brewtarget::displayAmount(row->amount(), Units::kilograms) );
      case SALTADDTOCOL:
         return QVariant( tr(row->addTo() == Salt::MASH ? "Mash" : "Sparge"));
      default :
         Brewtarget::logW(tr("Bad column: %1").arg(index.column()));
         return QVariant();
   }
}

QVariant SaltTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case SALTNAMECOL:
            return QVariant(tr("Name"));
         case SALTAMOUNTCOL:
            return QVariant(tr("Amount"));
         case SALTADDTOCOL:
            return QVariant(tr("Added To"));
         default:
            Brewtarget::logW(tr("Bad column: %1").arg(section));
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags SaltTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();
   switch(col)
   {
      case SALTNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      default:
         return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool SaltTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Salt *row;
   bool retval = false;

   if( index.row() >= saltObs.size() || role != Qt::EditRole )
      return false;
   else
      row = saltObs[index.row()];

   retval = value.canConvert(QVariant::String);
   if ( ! retval )
      return retval;

   Unit::unitDisplay dspUnit = displayUnit(index.column());
   Unit::unitScale   dspScl  = displayScale(index.column());

   switch( index.column() )
   {
      case SALTNAMECOL:
         row->setName(value.toString());
         break;
      case SALTAMOUNTCOL:
         row->setAmount( Brewtarget::qStringToSI(value.toString(), Units::kilograms, dspUnit, dspScl) );
         break;
      case SALTADDTOCOL:
         row->setAddTo( static_cast<Salt::WhenToAdd>(value.toInt()) );
         break;
      default:
         retval = false;
         Brewtarget::logW(tr("Bad column: %1").arg(index.column()));
   }

   return retval;
}

Unit::unitDisplay SaltTableModel::displayUnit(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noUnit;

   return static_cast<Unit::unitDisplay>(Brewtarget::option(attribute, QVariant(-1), this->objectName(), Brewtarget::UNIT).toInt());
}

Unit::unitScale SaltTableModel::displayScale(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noScale;

   return static_cast<Unit::unitScale>(Brewtarget::option(attribute, QVariant(-1), this->objectName(), Brewtarget::SCALE).toInt());
}

// We need to:
//   o clear the custom scale if set
//   o clear any custom unit from the rows
//      o which should have the side effect of clearing any scale
void SaltTableModel::setDisplayUnit(int column, Unit::unitDisplay displayUnit)
{
   // Yeast* row; // disabled per-cell magic
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayUnit,this->objectName(),Brewtarget::UNIT);
   Brewtarget::setOption(attribute,Unit::noScale,this->objectName(),Brewtarget::SCALE);

   /* Disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getYeast(i);
      row->setDisplayUnit(Unit::noUnit);
   }
   */
}

// Setting the scale should clear any cell-level scaling options
void SaltTableModel::setDisplayScale(int column, Unit::unitScale displayScale)
{
   // Yeast* row; //disabled per-cell magic

   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayScale,this->objectName(),Brewtarget::SCALE);

   /* disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getYeast(i);
      row->setDisplayScale(Unit::noScale);
   }
   */
}

QString SaltTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case SALTAMOUNTCOL:
         attribute = "amount";
         break;
      default:
         attribute = "";
   }
   return attribute;
}

//==========================CLASS HopItemDelegate===============================

SaltItemDelegate::SaltItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}

QWidget* SaltItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
   return new QLineEdit(parent);
}

void SaltItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   QLineEdit* line = qobject_cast<QLineEdit*>(editor);
   line->setText(index.model()->data(index, Qt::DisplayRole).toString());
}

void SaltItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   QLineEdit* line = qobject_cast<QLineEdit*>(editor);

   if ( line->isModified() )
      model->setData(index, line->text(), Qt::EditRole);
}

void SaltItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
