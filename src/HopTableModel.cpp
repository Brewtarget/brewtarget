/*
 * HopTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Luke Vincent <luke.r.vincent@gmail.com>
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

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QComboBox>
#include <QLineEdit>
#include <QHeaderView>

#include "database.h"
#include "hop.h"
#include <QString>
#include <QVector>
#include "hop.h"
#include "HopTableModel.h"
#include "unit.h"
#include "brewtarget.h"

HopTableModel::HopTableModel(QTableView* parent, bool editable)
   : QAbstractTableModel(parent),
     colFlags(HOPNUMCOLS),
     _inventoryEditable(false),
     recObs(nullptr),
     parentTableWidget(parent),
     showIBUs(false)
{
   hopObs.clear();
   setObjectName("hopTable");

   int i;
   for( i = 0; i < HOPNUMCOLS; ++i )
   {
      if( i == HOPNAMECOL )
         colFlags[i] = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      else if( i == HOPINVENTORYCOL )
         colFlags[i] = Qt::ItemIsEnabled;
      else
         colFlags[i] = Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }

   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   parentTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->setWordWrap(false);

   connect(headerView, &QWidget::customContextMenuRequested, this, &HopTableModel::contextMenu);
   connect( &(Database::instance()), &Database::changedInventory, this, &HopTableModel::changedInventory );
}

HopTableModel::~HopTableModel()
{
   hopObs.clear();
}

void HopTableModel::observeRecipe(Recipe* rec)
{
   if( recObs )
   {
      disconnect( recObs, nullptr, this, nullptr );
      removeAll();
   }

   recObs = rec;
   if( recObs )
   {
      connect( recObs, &Ingredient::changed, this, &HopTableModel::changed );
      addHops( recObs->hops() );
   }
}

void HopTableModel::observeDatabase(bool val)
{
   if( val )
   {
      observeRecipe(nullptr);
      removeAll();
      connect( &(Database::instance()), &Database::newHopSignal, this, &HopTableModel::addHop );
      connect( &(Database::instance()), SIGNAL(deletedSignal(Hop*)), this, SLOT(removeHop(Hop*)) );
      addHops( Database::instance().hops() );
   }
   else
   {
      removeAll();
      disconnect( &(Database::instance()), nullptr, this, nullptr );
   }
}

void HopTableModel::addHop(Hop* hop)
{
   if( hop == nullptr || hopObs.contains(hop) )
      return;

   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if( recObs == nullptr && ( hop->deleted() || !hop->display() ) )
      return;

   int size = hopObs.size();
   beginInsertRows( QModelIndex(), size, size );
   hopObs.append(hop);
   connect( hop, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
}

void HopTableModel::addHops(QList<Hop*> hops)
{
   QList<Hop*>::iterator i;
   QList<Hop*> tmp;

   for( i = hops.begin(); i != hops.end(); i++ )
   {
      if( recObs == nullptr && ( (*i)->deleted() || !(*i)->display() ) )
         continue;
      if( !hopObs.contains(*i) )
         tmp.append(*i);
   }

   int size = hopObs.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      hopObs.append(tmp);

      for( i = tmp.begin(); i != tmp.end(); i++ )
         connect( *i, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );

      endInsertRows();
   }
}

bool HopTableModel::removeHop(Hop* hop)
{
   int i;
   i = hopObs.indexOf(hop);
   if( i >= 0 )
   {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( hop, nullptr, this, nullptr );
      hopObs.removeAt(i);
      //reset(); // Tell everybody the table has changed.
      endRemoveRows();

      return true;
   }

   return false;
}

void HopTableModel::setShowIBUs( bool var )
{
   showIBUs = var;
}

void HopTableModel::removeAll()
{
   if (hopObs.size())
   {
      beginRemoveRows( QModelIndex(), 0, hopObs.size()-1 );
      while( !hopObs.isEmpty() )
      {
         disconnect( hopObs.takeLast(), nullptr, this, nullptr );
      }
      endRemoveRows();
   }
}

void HopTableModel::changedInventory(Brewtarget::DBTable table, int invKey, QVariant val)
{
   if ( table == Brewtarget::HOPTABLE ) {
      for( int i = 0; i < hopObs.size(); ++i ) {
         Hop* holdmybeer = hopObs.at(i);

         if ( invKey == holdmybeer->inventoryId() ) {
            holdmybeer->setCacheOnly(true);
            holdmybeer->setInventoryAmount(val.toDouble());
            holdmybeer->setCacheOnly(false);
            emit dataChanged( QAbstractItemModel::createIndex(i,HOPINVENTORYCOL),
                              QAbstractItemModel::createIndex(i,HOPINVENTORYCOL) );
         }
      }
   }
}

void HopTableModel::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;

   // Find the notifier in the list
   Hop* hopSender = qobject_cast<Hop*>(sender());
   if( hopSender )
   {
      i = hopObs.indexOf(hopSender);
      if( i < 0 )
         return;

      emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                        QAbstractItemModel::createIndex(i, HOPNUMCOLS-1));
      emit headerDataChanged( Qt::Vertical, i, i );
      return;
   }

   // See if sender is our recipe.
   Recipe* recSender = qobject_cast<Recipe*>(sender());
   if( recSender && recSender == recObs )
   {
      if( QString(prop.name()) == "hops" )
      {
         removeAll();
         addHops( recObs->hops() );
      }
      if( rowCount() > 0 )
         emit headerDataChanged( Qt::Vertical, 0, rowCount()-1 );
      return;
   }
}

int HopTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return hopObs.size();
}

int HopTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return HOPNUMCOLS;
}

QVariant HopTableModel::data( const QModelIndex& index, int role ) const
{
   Hop* row;
   int col = index.column();
   Unit::unitScale scale;
   Unit::unitDisplay unit;

   // Ensure the row is ok.
   if( index.row() >= static_cast<int>(hopObs.size() ))
   {
      Brewtarget::logW(QString("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = hopObs[index.row()];

   switch( index.column() )
   {
      case HOPNAMECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->name());
         else
            return QVariant();
      case HOPALPHACOL:
         if( role == Qt::DisplayRole )
            return QVariant( Brewtarget::displayAmount(row->alpha_pct(), nullptr) );
         else
            return QVariant();
      case HOPINVENTORYCOL:
         if( role != Qt::DisplayRole )
            return QVariant();
         unit = displayUnit(col);
         scale = displayScale(col);

         return QVariant(Brewtarget::displayAmount(row->inventory(), Units::kilograms, 3, unit, scale));

      case HOPAMOUNTCOL:
         if( role != Qt::DisplayRole )
            return QVariant();
         unit = displayUnit(col);
         scale = displayScale(col);

         return QVariant(Brewtarget::displayAmount(row->amount_kg(), Units::kilograms, 3, unit, scale));

      case HOPUSECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->useStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->use());
         else
            return QVariant();
      case HOPTIMECOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         scale = displayScale(col);

         return QVariant( Brewtarget::displayAmount(row->time_min(), Units::minutes, 3, Unit::noUnit, scale) );
      case HOPFORMCOL:
        if ( role == Qt::DisplayRole )
          return QVariant( row->formStringTr() );
        else if ( role == Qt::UserRole )
           return QVariant( row->form());
        else
           return QVariant();
      default :
         Brewtarget::logW(QString("HopTableModel::data Bad column: %1").arg(index.column()));
         return QVariant();
   }
}

QVariant HopTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case HOPNAMECOL:
            return QVariant(tr("Name"));
         case HOPALPHACOL:
            return QVariant(tr("Alpha %"));
         case HOPINVENTORYCOL:
            return QVariant(tr("Inventory"));
         case HOPAMOUNTCOL:
            return QVariant(tr("Amount"));
         case HOPUSECOL:
            return QVariant(tr("Use"));
         case HOPTIMECOL:
            return QVariant(tr("Time"));
         case HOPFORMCOL:
            return QVariant(tr("Form"));
         default:
            Brewtarget::logW(QString("HopTableModel::headerdata Bad column: %1").arg(section));
            return QVariant();
      }
   }
   else if( showIBUs && recObs && orientation == Qt::Vertical && role == Qt::DisplayRole )
   {
      QList<double> ibus = recObs->IBUs();

      if ( ibus.size() > section )
         return QVariant( QString("%L1 IBU").arg( ibus.at(section), 0, 'f', 1 ) );
   }
   return QVariant();
}

Qt::ItemFlags HopTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();

   return colFlags[col];
}

bool HopTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Hop *row;
   bool retVal = false;
   double amt;

   if( index.row() >= static_cast<int>(hopObs.size()) || role != Qt::EditRole )
      return false;

   row = hopObs[index.row()];

   Unit::unitDisplay dspUnit = displayUnit(index.column());
   Unit::unitScale   dspScl  = displayScale(index.column());
   switch( index.column() )
   {
      case HOPNAMECOL:
         retVal = value.canConvert(QVariant::String);
         if( retVal )
            row->setName(value.toString());
         break;
      case HOPALPHACOL:
         retVal = value.canConvert(QVariant::Double);
         if( retVal )
         {
            amt = Brewtarget::toDouble( value.toString(), &retVal );
            if ( ! retVal )
               Brewtarget::logW( QString("HopTableModel::setData() could not convert %1 to double").arg(value.toString()));
            row->setAlpha_pct( amt );
         }
         break;

      case HOPINVENTORYCOL:
         retVal = value.canConvert(QVariant::String);
         if( retVal )
            row->setInventoryAmount( Brewtarget::qStringToSI(value.toString(),Units::kilograms, displayUnit(HOPINVENTORYCOL)));
         break;
      case HOPAMOUNTCOL:
         retVal = value.canConvert(QVariant::String);
         if( retVal )
            row->setAmount_kg( Brewtarget::qStringToSI(value.toString(), Units::kilograms, dspUnit, dspScl));
         break;
      case HOPUSECOL:
         retVal = value.canConvert(QVariant::Int);
         if( retVal )
            row->setUse(static_cast<Hop::Use>(value.toInt()));
         break;
      case HOPFORMCOL:
         retVal = value.canConvert(QVariant::Int);
         if( retVal )
            row->setForm(static_cast<Hop::Form>(value.toInt()));
         break;
      case HOPTIMECOL:
         retVal = value.canConvert(QVariant::String);
         if( retVal )
            row->setTime_min( Brewtarget::qStringToSI(value.toString(),Units::minutes,dspUnit,dspScl));
         break;
      default:
         Brewtarget::logW(QString("HopTableModel::setdata Bad column: %1").arg(index.column()));
         return false;
   }
   if ( retVal )
      headerDataChanged( Qt::Vertical, index.row(), index.row() ); // Need to re-show header (IBUs).

   return retVal;
}

Unit::unitDisplay HopTableModel::displayUnit(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noUnit;

   return static_cast<Unit::unitDisplay>(Brewtarget::option(attribute, QVariant(-1), this->objectName(), Brewtarget::UNIT).toInt());
}

Unit::unitScale HopTableModel::displayScale(int column) const
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
void HopTableModel::setDisplayUnit(int column, Unit::unitDisplay displayUnit)
{
   // Hop* row; // disabled per-cell magic
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayUnit,this->objectName(),Brewtarget::UNIT);
   Brewtarget::setOption(attribute,Unit::noScale,this->objectName(),Brewtarget::SCALE);

}

// Setting the scale should clear any cell-level scaling options
void HopTableModel::setDisplayScale(int column, Unit::unitScale displayScale)
{
   // Fermentable* row; //disabled per-cell magic

   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayScale,this->objectName(),Brewtarget::SCALE);

}

QString HopTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case HOPINVENTORYCOL:
         attribute = "inventory_kg";
         break;
      case HOPAMOUNTCOL:
         attribute = "amount_kg";
         break;
      case HOPTIMECOL:
         attribute = "time_min";
         break;
      default:
         attribute = "";
   }
   return attribute;
}

void HopTableModel::contextMenu(const QPoint &point)
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
      case HOPINVENTORYCOL:
      case HOPAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(parentTableWidget,currentUnit, currentScale);
         break;
      case HOPTIMECOL:
         menu = Brewtarget::setupTimeMenu(parentTableWidget,currentScale);
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == nullptr )
      return;

   QWidget* pMenu = invoked->parentWidget();
   if ( selected != HOPTIMECOL && pMenu == menu )
      setDisplayUnit(selected,static_cast<Unit::unitDisplay>(invoked->data().toInt()));
   else
      setDisplayScale(selected,static_cast<Unit::unitScale>(invoked->data().toInt()));

}

// Returns null on failure.
Hop* HopTableModel::getHop(int i) {
    if(!(hopObs.isEmpty())) {
        if(i >= 0 && i < hopObs.size())
            return hopObs[i];
    }
    else
        Brewtarget::logW( QString("HopTableModel::getHop( %1/%2 )").arg(i).arg(hopObs.size()) );
    return nullptr;
}

//==========================CLASS HopItemDelegate===============================

HopItemDelegate::HopItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}

QWidget* HopItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const
{
   if ( index.column() == HOPUSECOL )
   {
      QComboBox *box = new QComboBox(parent);

      // NOTE: these need to be in the same order as the Hop::Use enum.
      box->addItem(tr("Mash"));
      box->addItem(tr("First Wort"));
      box->addItem(tr("Boil"));
      box->addItem(tr("Aroma"));
      box->addItem(tr("Dry Hop"));
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   }
   else if ( index.column() == HOPFORMCOL )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Leaf"));
      box->addItem(tr("Pellet"));
      box->addItem(tr("Plug"));
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   }
   else
   {
      return new QLineEdit(parent);
   }
}

void HopItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   if (index.column() == HOPUSECOL )
   {
      QComboBox* box = static_cast<QComboBox*>(editor);
      int ndx = index.model()->data(index, Qt::UserRole).toInt();

      box->setCurrentIndex(ndx);
   }
   else if ( index.column() == HOPFORMCOL )
   {
      QComboBox* box = static_cast<QComboBox*>(editor);
      int ndx = index.model()->data(index,Qt::UserRole).toInt();

      box->setCurrentIndex(ndx);
   }
   else
   {
       QLineEdit* line = static_cast<QLineEdit*>(editor);
       line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }
}

void HopItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   if ( index.column() == HOPUSECOL )
   {
      QComboBox* box = static_cast<QComboBox*>(editor);
      int value = box->currentIndex();
      int ndx = model->data(index, Qt::UserRole).toInt();

      if ( value != ndx )
         model->setData(index, value, Qt::EditRole);
   }
   else if (index.column() == HOPFORMCOL )
   {
      QComboBox* box = static_cast<QComboBox*>(editor);
      int value = box->currentIndex();
      int ndx = model->data(index, Qt::UserRole).toInt();

      if ( value != ndx )
         model->setData(index, value, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = static_cast<QLineEdit*>(editor);
      if ( line->isModified() )
         model->setData(index, line->text(), Qt::EditRole);
   }
}

void HopItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
