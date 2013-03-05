/*
 * MiscTableModel.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QComboBox>
#include <QLineEdit>
#include "database.h"
#include "misc.h"
#include "MiscTableModel.h"
#include "unit.h"
#include "brewtarget.h"
#include "recipe.h"

MiscTableModel::MiscTableModel(QTableView* parent, bool editable)
   : QAbstractTableModel(parent), editable(editable), recObs(0), parentTableWidget(parent)
{
   miscObs.clear();
}

void MiscTableModel::observeRecipe(Recipe* rec)
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
      addMiscs( recObs->miscs() );
   }
}

void MiscTableModel::observeDatabase(bool val)
{
   if( val )
   {
      observeRecipe(0);
      removeAll();
      connect( &(Database::instance()), SIGNAL(newMiscSignal(Misc*)), this, SLOT(addMisc(Misc*)) );
      connect( &(Database::instance()), SIGNAL(deletedMiscSignal(Misc*)), this, SLOT(removeMisc(Misc*)) );
      addMiscs( Database::instance().miscs() );
   }
   else
   {
      removeAll();
      disconnect( &(Database::instance()), 0, this, 0 );
   }
}

void MiscTableModel::addMisc(Misc* misc)
{
   if( miscObs.contains(misc) )
      return;
   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if(
      recObs == 0 &&
      (
         misc->deleted() ||
         !misc->display()
      )
   )
      return;
 
   int size = miscObs.size();
   beginInsertRows( QModelIndex(), size, size );
   miscObs.append(misc);
   connect( misc, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
   
   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void MiscTableModel::addMiscs(QList<Misc*> miscs)
{
   QList<Misc*>::iterator i;
   QList<Misc*> tmp;
   
   for( i = miscs.begin(); i != miscs.end(); i++ )
   {
      if( !miscObs.contains(*i) )
         tmp.append(*i);
   }
   
   int size = miscObs.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      miscObs.append(tmp);
      
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

// Returns true when misc is successfully found and removed.
bool MiscTableModel::removeMisc(Misc* misc)
{
   int i;
   
   i = miscObs.indexOf(misc);
   if( i >= 0 )
   {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( misc, 0, this, 0 );
      miscObs.removeAt(i);
      //reset(); // Tell everybody the table has changed.
      endRemoveRows();
      
      if(parentTableWidget)
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
   if (miscObs.size())
   {
      beginRemoveRows( QModelIndex(), 0, miscObs.size()-1 );
      while( !miscObs.isEmpty() )
      {
         disconnect( miscObs.takeLast(), 0, this, 0 );
      }
      endRemoveRows();
   }
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
   unitDisplay unit;
   
   // Ensure the row is ok.
   if( index.row() >= (int)miscObs.size() )
   {
      Brewtarget::logW(QString("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = miscObs[index.row()];
   
   // Deal with the column and return the right data.
   switch( index.column() )
   {
      case MISCNAMECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->name());
         else
            return QVariant();
      case MISCTYPECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->typeStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->type());
         else
            return QVariant();
      case MISCUSECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->useStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->use());
         else
            return QVariant();
      case MISCTIMECOL:
         if( role == Qt::DisplayRole )
            return QVariant( Brewtarget::displayAmount(row->time(), Units::minutes) );
         else
            return QVariant();
      case MISCAMOUNTCOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         unit = displayUnit(index.column());
         return QVariant( Brewtarget::displayAmount(row->amount(), row->amountIsWeight()? (Unit*)Units::kilograms : (Unit*)Units::liters, 3, unit, noScale ) );

      default:
         Brewtarget::log(Brewtarget::WARNING, QString("Bad model index. column = %1").arg(index.column()));
   }
   return QVariant();
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
         return Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool MiscTableModel::setData( const QModelIndex& index, const QVariant& value, int /*role*/ )
{
   Misc *row;
   int col;
   QString tmpStr;
   unitDisplay unit;
   
   if( index.row() >= (int)miscObs.size() )
      return false;
   else
      row = miscObs[index.row()];
   
   col = index.column();
   switch (col )
   {
      case MISCNAMECOL:
         if( value.canConvert(QVariant::String) )
         {
            tmpStr = value.toString();
            row->setName(tmpStr);
         }
         else
            return false;
         break;
      case MISCTYPECOL:
         if( value.canConvert(QVariant::Int) )
            row->setType( static_cast<Misc::Type>(value.toInt()) );
         else
            return false;
         break;
      case MISCUSECOL:
         if( value.canConvert(QVariant::Int) )
            row->setUse( static_cast<Misc::Use>(value.toInt()) );
         else
            return false;
         break;
      case MISCTIMECOL:
         if( value.canConvert(QVariant::String) )
            row->setTime( Brewtarget::timeQStringToSI(value.toString()) );
         else
            return false;
         break;
      case MISCAMOUNTCOL:
         unit = displayUnit(col);
         if( value.canConvert(QVariant::String) )
            row->setAmount( row->amountIsWeight() ? 
                            Brewtarget::weightQStringToSI(value.toString(),unit) : 
                            Brewtarget::volQStringToSI(value.toString(),unit) );
         else
            return false;
         break;
      default:
         return false;
   }
   
   emit dataChanged( index, index );
   return true;
}

void MiscTableModel::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;
   
   Misc* miscSender = qobject_cast<Misc*>(sender());
   if( miscSender )
   {
      i = miscObs.indexOf(miscSender);
      if( i < 0 )
         return;
      
      emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                        QAbstractItemModel::createIndex(i, MISCNUMCOLS-1) );
      return;
   }
   
   // See if sender is our recipe.
   Recipe* recSender = qobject_cast<Recipe*>(sender());
   if( recSender && recSender == recObs )
   {
      if( QString(prop.name()) == "miscs" )
      {
         removeAll();
         addMiscs( recObs->miscs() );
      }
      if( rowCount() > 0 )
         emit headerDataChanged( Qt::Vertical, 0, rowCount()-1 );
      return;
   }
   
   // See if sender is the database.
   if( sender() == &(Database::instance()) && QString(prop.name()) == "miscs" )
   {
      removeAll();
      addMiscs( Database::instance().miscs() );
      return;
   }
}

Misc* MiscTableModel::getMisc(unsigned int i)
{
   return miscObs[i];
}

unitDisplay MiscTableModel::displayUnit(int column) const
{ 
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return noUnit;

   return (unitDisplay)Brewtarget::option(attribute, noUnit, this, Brewtarget::UNIT).toInt();
}

unitScale MiscTableModel::displayScale(int column) const
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
void MiscTableModel::setDisplayUnit(int column, unitDisplay displayUnit) 
{
   // Misc* row; // disabled per-cell magic
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayUnit,this,Brewtarget::UNIT); 
   Brewtarget::setOption(attribute,noScale,this,Brewtarget::SCALE);

   /* Disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getMisc(i);
      row->setDisplayUnit(noUnit);
   }
   */
}

// Setting the scale should clear any cell-level scaling options
void MiscTableModel::setDisplayScale(int column, unitScale displayScale) 
{ 
   // Misc* row; //disabled per-cell magic

   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayScale,this,Brewtarget::SCALE); 

   /* disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getMisc(i);
      row->setDisplayScale(noScale);
   }
   */
}

QString MiscTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case MISCAMOUNTCOL:
         attribute = "amount";
         break;
      default:
         attribute = "";
   }
   return attribute;
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

