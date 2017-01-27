/*
 * MiscTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
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

#include <QComboBox>
#include <QLineEdit>
#include <QHeaderView>
#include "database.h"
#include "misc.h"
#include "MiscTableModel.h"
#include "unit.h"
#include "brewtarget.h"
#include "recipe.h"

MiscTableModel::MiscTableModel(QTableView* parent, bool editable)
   : QAbstractTableModel(parent),
     editable(editable),
     _inventoryEditable(false),
     recObs(0),
     parentTableWidget(parent)
{
   miscObs.clear();
   setObjectName("miscTableModel");

   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   parentTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    parentTableWidget->setWordWrap(false);

   connect(headerView, &QWidget::customContextMenuRequested, this, &MiscTableModel::contextMenu);
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
      connect( recObs, &BeerXMLElement::changed, this, &MiscTableModel::changed );
      addMiscs( recObs->miscs() );
   }
}

void MiscTableModel::observeDatabase(bool val)
{
   if( val )
   {
      observeRecipe(0);
      removeAll();
      connect( &(Database::instance()), &Database::newMiscSignal, this, &MiscTableModel::addMisc );
      connect( &(Database::instance()), SIGNAL(deletedSignal(Misc*)), this, SLOT(removeMisc(Misc*)) );
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
   connect( misc, &BeerXMLElement::changed, this, &MiscTableModel::changed );
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
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
         connect( *i, &BeerXMLElement::changed, this, &MiscTableModel::changed );

      endInsertRows();
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
   Unit::unitDisplay unit;
   Unit::unitScale scale;

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
         if( role != Qt::DisplayRole )
            return QVariant();

         scale = displayScale(MISCTIMECOL);

         return QVariant( Brewtarget::displayAmount(row->time(), Units::minutes, 3, Unit::noUnit, scale) );
      case MISCINVENTORYCOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         unit = displayUnit(index.column());
         return QVariant( Brewtarget::displayAmount(row->inventory(), row->amountIsWeight()? (Unit*)Units::kilograms : (Unit*)Units::liters, 3, unit, Unit::noScale ) );
      case MISCAMOUNTCOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         unit = displayUnit(index.column());
         return QVariant( Brewtarget::displayAmount(row->amount(), row->amountIsWeight()? (Unit*)Units::kilograms : (Unit*)Units::liters, 3, unit, Unit::noScale ) );

      case MISCISWEIGHT:
         if( role == Qt::DisplayRole )
            return QVariant(row->amountTypeStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->amountType());
         else
            return QVariant();
      default:
         Brewtarget::logW(QString("Bad model index. column = %1").arg(index.column()));
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
         case MISCINVENTORYCOL:
            return QVariant(tr("Inventory"));
         case MISCAMOUNTCOL:
            return QVariant(tr("Amount"));
         case MISCISWEIGHT:
            return QVariant(tr("Amount Type"));
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
   Qt::ItemFlags defaults = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
   switch( col )
   {
      case MISCNAMECOL:
         return defaults;
      case MISCINVENTORYCOL:
         return (defaults | (_inventoryEditable ? Qt::ItemIsEditable : Qt::NoItemFlags));
      default:
         return defaults | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags);
   }
}

bool MiscTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Misc *row;
   int col;
   QString tmpStr;
   Unit* unit;

   if( index.row() >= (int)miscObs.size() )
      return false;
   else
      row = miscObs[index.row()];

   col = index.column();
   unit = row->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters;

   Unit::unitDisplay dspUnit = displayUnit(index.column());
   Unit::unitScale   dspScl  = displayScale(index.column());

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
         if( ! value.canConvert(QVariant::Int) )
            return false;
         row->setType( static_cast<Misc::Type>(value.toInt()) );
         break;
      case MISCUSECOL:
         if( ! value.canConvert(QVariant::Int) )
            return false;
         row->setUse( static_cast<Misc::Use>(value.toInt()) );
         break;
      case MISCTIMECOL:
         if( ! value.canConvert(QVariant::String) )
            return false;

         row->setTime( Brewtarget::qStringToSI(value.toString(), Units::minutes, dspUnit, dspScl) );
         break;
      case MISCINVENTORYCOL:
         if( ! value.canConvert(QVariant::String) )
            return false;

         row->setInventoryAmount(Brewtarget::qStringToSI(value.toString(), unit, dspUnit,dspScl));
      case MISCAMOUNTCOL:
         if( ! value.canConvert(QVariant::String) )
            return false;

         row->setAmount( Brewtarget::qStringToSI(value.toString(), unit, dspUnit,dspScl ));
         break;
      case MISCISWEIGHT:
         if( ! value.canConvert(QVariant::Int) )
            return false;

         row->setAmountType( static_cast<Misc::AmountType>(value.toInt()) );
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

Unit::unitDisplay MiscTableModel::displayUnit(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noUnit;

   return (Unit::unitDisplay)Brewtarget::option(attribute, Unit::noUnit, this->objectName(), Brewtarget::UNIT).toInt();
}

Unit::unitScale MiscTableModel::displayScale(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noScale;

   return (Unit::unitScale)Brewtarget::option(attribute, Unit::noScale, this->objectName(), Brewtarget::SCALE).toInt();
}

// We need to:
//   o clear the custom scale if set
//   o clear any custom unit from the rows
//      o which should have the side effect of clearing any scale
void MiscTableModel::setDisplayUnit(int column, Unit::unitDisplay displayUnit)
{
   // Misc* row; // disabled per-cell magic
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayUnit,this->objectName(),Brewtarget::UNIT);
   Brewtarget::setOption(attribute,Unit::noScale,this->objectName(),Brewtarget::SCALE);

   /* Disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getMisc(i);
      row->setDisplayUnit(Unit::noUnit);
   }
   */
}

// Setting the scale should clear any cell-level scaling options
void MiscTableModel::setDisplayScale(int column, Unit::unitScale displayScale)
{
   // Misc* row; //disabled per-cell magic

   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayScale,this->objectName(),Brewtarget::SCALE);

   /* disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getMisc(i);
      row->setDisplayScale(Unit::noScale);
   }
   */
}

QString MiscTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case MISCINVENTORYCOL:
         attribute = "inventory";
         break;
      case MISCAMOUNTCOL:
         attribute = "amount";
         break;
      case MISCTIMECOL:
         attribute = "time";
         break;
      default:
         attribute = "";
   }
   return attribute;
}

void MiscTableModel::contextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   Unit::unitDisplay currentUnit;
   Unit::unitScale  currentScale;

   // Since we need to call generateVolumeMenu() two different ways, we need
   // to figure out the currentUnit and Scale here

   currentUnit  = displayUnit(selected);
   currentScale = displayScale(selected);

   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case MISCINVENTORYCOL:
      case MISCAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(parentTableWidget,currentUnit, currentScale, false);
         break;
      case MISCTIMECOL:
         menu = Brewtarget::setupTimeMenu(parentTableWidget,currentScale);
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   if ( selected == MISCTIMECOL )
      setDisplayScale(selected,(Unit::unitScale)invoked->data().toInt());
   else
      setDisplayUnit(selected,(Unit::unitDisplay)invoked->data().toInt());
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
      box->setMinimumWidth(box->minimumSizeHint().width());
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
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }
   else if ( index.column() == MISCISWEIGHT )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Weight"));
      box->addItem(tr("Volume"));
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }
   else
      return new QLineEdit(parent);
}

void MiscItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   int column = index.column();

   if( column == MISCTYPECOL || column == MISCUSECOL || column == MISCISWEIGHT)
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
   if( column == MISCTYPECOL || column == MISCUSECOL || column == MISCISWEIGHT)
   {
      QComboBox* box = (QComboBox*)editor;
      int ndx = box->currentIndex();
      int curr = model->data(index, Qt::UserRole).toInt();

      if ( curr != ndx )
         model->setData(index, ndx, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = (QLineEdit*)editor;

      if ( line->isModified() )
         model->setData(index, line->text(), Qt::EditRole);
   }
}

void MiscItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   editor->setGeometry(option.rect);
}

