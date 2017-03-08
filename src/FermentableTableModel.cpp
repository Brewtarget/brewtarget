/*
 * FermentableTableModel.cpp is part of Brewtarget, and is Copyright the following
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

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <QItemEditorFactory>
#include <QStyle>
#include <QRect>
#include <QDebug>

#include "database.h"
#include "brewtarget.h"
#include <QSize>
#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include <QString>
#include <QVector>
#include <QHeaderView>
#include "fermentable.h"
#include "FermentableTableModel.h"
#include "unit.h"
#include "recipe.h"

//=====================CLASS FermentableTableModel==============================
FermentableTableModel::FermentableTableModel(QTableView* parent, bool editable)
   : QAbstractTableModel(parent),
     parentTableWidget(parent),
     editable(editable),
     _inventoryEditable(false),
     recObs(0),
     displayPercentages(false),
     totalFermMass_kg(0)
{
   fermObs.clear();
   // for units and scales
   setObjectName("fermentableTable");

   // Will this work here? Yes. Yes it will. Bwahahahahahaha
   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   parentTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->setWordWrap(false);
   connect(headerView, &QWidget::customContextMenuRequested, this, &FermentableTableModel::contextMenu);
}

void FermentableTableModel::observeRecipe(Recipe* rec)
{
   if( recObs )
   {
      disconnect( recObs, 0, this, 0 );
      removeAll();
   }

   recObs = rec;
   if( recObs )
   {
      connect( recObs, &BeerXMLElement::changed, this, &FermentableTableModel::changed );
      addFermentables( recObs->fermentables() );
   }
}

void FermentableTableModel::observeDatabase(bool val)
{
   if( val )
   {
      // Observing a database and a recipe are mutually exclusive.
      observeRecipe(0);

      removeAll();
      connect( &(Database::instance()), &Database::newFermentableSignal, this, &FermentableTableModel::addFermentable );
      connect( &(Database::instance()), SIGNAL(deletedSignal(Fermentable*)), this, SLOT(removeFermentable(Fermentable*)) );
      addFermentables( Database::instance().fermentables() );
   }
   else
   {
      disconnect( &(Database::instance()), 0, this, 0 );
      removeAll();
   }
}

void FermentableTableModel::addFermentable(Fermentable* ferm)
{
   //Check to see if it's already in the list
   if( fermObs.contains(ferm) )
      return;
   // If we are observing the database, ensure that the ferm is undeleted and
   // fit to display.
   if(
      recObs == 0 &&
      (
         ferm->deleted() ||
         !ferm->display()
      )
   )
      return;

   int size = fermObs.size();
   beginInsertRows( QModelIndex(), size, size );
   fermObs.append(ferm);
   connect( ferm, &BeerXMLElement::changed, this, &FermentableTableModel::changed );
   totalFermMass_kg += ferm->amount_kg();
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
}

void FermentableTableModel::addFermentables(QList<Fermentable*> ferms)
{
   QList<Fermentable*>::iterator i;
   QList<Fermentable*> tmp;

   for( i = ferms.begin(); i != ferms.end(); i++ )
   {
      if( !fermObs.contains(*i) )
         tmp.append(*i);
   }

   int size = fermObs.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      fermObs.append(tmp);

      for( i = tmp.begin(); i != tmp.end(); i++ )
      {
         connect( *i, &BeerXMLElement::changed, this, &FermentableTableModel::changed );
         totalFermMass_kg += (*i)->amount_kg();
      }

      endInsertRows();
   }
}

bool FermentableTableModel::removeFermentable(Fermentable* ferm)
{
   int i;

   i = fermObs.indexOf(ferm);
   if( i >= 0 )
   {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( ferm, 0, this, 0 );
      fermObs.removeAt(i);

      totalFermMass_kg -= ferm->amount_kg();
      //reset(); // Tell everybody the table has changed.
      endRemoveRows();

      return true;
   }

   return false;
}

void FermentableTableModel::removeAll()
{
   if (fermObs.size())
   {
      beginRemoveRows( QModelIndex(), 0, fermObs.size()-1 );
      while( !fermObs.isEmpty() )
      {
         disconnect( fermObs.takeLast(), 0, this, 0 );
      }
      endRemoveRows();
   }
   // I think we need to zero this out
   totalFermMass_kg = 0;
}

void FermentableTableModel::updateTotalGrains()
{
   int i, size;

   totalFermMass_kg = 0;

   size = fermObs.size();
   for( i = 0; i < size; ++i )
      totalFermMass_kg += fermObs[i]->amount_kg();
}

void FermentableTableModel::setDisplayPercentages(bool var)
{
   displayPercentages = var;
}

void FermentableTableModel::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;

   // Is sender one of our fermentables?
   Fermentable* fermSender = qobject_cast<Fermentable*>(sender());
   if( fermSender )
   {
      i = fermObs.indexOf(fermSender);
      if( i < 0 )
         return;

      updateTotalGrains();
      emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                        QAbstractItemModel::createIndex(i, FERMNUMCOLS-1));
      if( displayPercentages && rowCount() > 0 )
         emit headerDataChanged( Qt::Vertical, 0, rowCount()-1 );
      //reset();
      return;
   }

   // See if our recipe gained or lost fermentables.
   Recipe* recSender = qobject_cast<Recipe*>(sender());
   if( recSender && recSender == recObs && QString(prop.name()) == "fermentables" )
   {
      removeAll();
      addFermentables( recObs->fermentables() );
      return;
   }
}

int FermentableTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return fermObs.size();
}

int FermentableTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return FERMNUMCOLS;
}

QVariant FermentableTableModel::data( const QModelIndex& index, int role ) const
{
   Fermentable* row;
   int col = index.column();
   Unit::unitScale scale;
   Unit::unitDisplay unit;

   // Ensure the row is ok.
   if( index.row() >= (int)fermObs.size() )
   {
      Brewtarget::logE(tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = fermObs[index.row()];

   switch( col )
   {
      case FERMNAMECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->name());
         else
            return QVariant();
      case FERMTYPECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->typeStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->type());
         else
            return QVariant();
      case FERMINVENTORYCOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         // So just query the columns
         unit  = displayUnit(col);
         scale = displayScale(col);

         return QVariant( Brewtarget::displayAmount(row->inventory(), Units::kilograms, 3, unit, scale) );
      case FERMAMOUNTCOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         // So just query the columns
         unit  = displayUnit(col);
         scale = displayScale(col);

         return QVariant( Brewtarget::displayAmount(row->amount_kg(), Units::kilograms, 3, unit, scale) );
      case FERMISMASHEDCOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->additionMethodStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->additionMethod());
         else
            return QVariant();
      case FERMAFTERBOIL:
         if( role == Qt::DisplayRole )
            return QVariant(row->additionTimeStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->additionTime());
         else
            return QVariant();
      case FERMYIELDCOL:
         if( role == Qt::DisplayRole )
            return QVariant( Brewtarget::displayAmount(row->yield_pct(), 0) );
         else
            return QVariant();
      case FERMCOLORCOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         unit  = displayUnit(col);

         return QVariant( Brewtarget::displayAmount(row->color_srm(), Units::srm, 0, unit) );
      default :
         Brewtarget::logE(tr("Bad column: %1").arg(col));
         return QVariant();
   }
}

QVariant FermentableTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   QString uName;
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case FERMNAMECOL:
            return QVariant(tr("Name"));
         case FERMTYPECOL:
            return QVariant(tr("Type"));
         case FERMINVENTORYCOL:
            return QVariant(tr("Inventory"));
         case FERMAMOUNTCOL:
            return QVariant(tr("Amount"));
         case FERMISMASHEDCOL:
            return QVariant(tr("Method"));
         case FERMAFTERBOIL:
            return QVariant(tr("Addition"));
         case FERMYIELDCOL:
            return QVariant(tr("Yield %"));
         case FERMCOLORCOL:
            return QVariant(tr("Color"));
         default:
            Brewtarget::logW(tr("Bad column: %1").arg(section));
            return QVariant();
      }
   }
   else if( displayPercentages && orientation == Qt::Vertical && role == Qt::DisplayRole )
   {
      double perMass = 0.0;
      if ( totalFermMass_kg > 0.0 )
         perMass = fermObs[section]->amount_kg()/totalFermMass_kg;
      return QVariant( QString("%1%").arg( (double)100.0 * perMass, 0, 'f', 0 ) );
   }

   return QVariant();
}

Qt::ItemFlags FermentableTableModel::flags(const QModelIndex& index ) const
{
   Qt::ItemFlags defaults = Qt::ItemIsEnabled;
   int col = index.column();
   Fermentable* row = fermObs[index.row()];

   switch(col)
   {
      case FERMISMASHEDCOL:
         // Ensure that being mashed and being a late addition are mutually exclusive.
         if( !row->addAfterBoil() )
            return (defaults | Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled);
         else
            return Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled;
         break;
      case FERMAFTERBOIL:
         // Ensure that being mashed and being a late addition are mutually exclusive.
         if( !row->isMashed() )
            return (defaults | Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled);
         else
            return Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled;
         break;
      case FERMNAMECOL:
         return (defaults | Qt::ItemIsSelectable);
         break;
      case FERMINVENTORYCOL:
         return (defaults | (_inventoryEditable ? Qt::ItemIsEditable : Qt::NoItemFlags));
         break;
      default:
         return (defaults | Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) );
   }
}

/* --maf--
   The cell-specific work has been momentarily disabled until I can find a
   better way to implement. PLEASE DO NOT DELETE
Unit::unitDisplay FermentableTableModel::displayUnit(const QModelIndex& index)
{
   Fermentable* row;

   if ( index.row() >= fermObs.size() )
      return Unit::noUnit;

   row = fermObs[index.row()];

   return row->displayUnit();
}

void FermentableTableModel::setDisplayUnit(const QModelIndex& index, Unit::unitDisplay displayUnit)
{
   Fermentable* row;

   if ( index.row() >= fermObs.size() )
      return;

   row = fermObs[index.row()];
   row->setDisplayUnit(displayUnit);
}

Unit::unitScale FermentableTableModel::displayScale(const QModelIndex& index)
{
   Fermentable* row;

   if ( index.row() >= fermObs.size() )
      return Unit::noScale;

   row = fermObs[index.row()];

   return row->displayScale();
}

void FermentableTableModel::setDisplayScale(const QModelIndex& index, Unit::unitScale displayScale)
{
   Fermentable* row;

   if ( index.row() >= fermObs.size() )
      return;

   row = fermObs[index.row()];
   row->setDisplayScale(displayScale);
}
*/

Unit::unitDisplay FermentableTableModel::displayUnit(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noUnit;

   return (Unit::unitDisplay)Brewtarget::option(attribute, QVariant(-1), this->objectName(), Brewtarget::UNIT).toInt();
}

Unit::unitScale FermentableTableModel::displayScale(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noScale;

   return (Unit::unitScale)Brewtarget::option(attribute, QVariant(-1), this->objectName(), Brewtarget::SCALE).toInt();
}

// We need to:
//   o clear the custom scale if set
//   o clear any custom unit from the rows
//      o which should have the side effect of clearing any scale
void FermentableTableModel::setDisplayUnit(int column, Unit::unitDisplay displayUnit)
{
   // Fermentable* row; // disabled per-cell magic
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayUnit,this->objectName(),Brewtarget::UNIT);
   Brewtarget::setOption(attribute,Unit::noScale,this->objectName(),Brewtarget::SCALE);

   /* Disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getFermentable(i);
      row->setDisplayUnit(Unit::noUnit);
   }
   */
}

// Setting the scale should clear any cell-level scaling options
void FermentableTableModel::setDisplayScale(int column, Unit::unitScale displayScale)
{
   // Fermentable* row; //disabled per-cell magic

   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayScale,this->objectName(),Brewtarget::SCALE);

   /* disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getFermentable(i);
      row->setDisplayScale(Unit::noScale);
   }
   */
}

QString FermentableTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case FERMINVENTORYCOL:
         attribute = "inventory_kg";
         break;
      case FERMAMOUNTCOL:
         attribute = "amount_kg";
         break;
      case FERMCOLORCOL:
         attribute = "color_srm";
         break;
      default:
         attribute = "";
   }
   return attribute;
}

// oofrab
void FermentableTableModel::contextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   Unit::unitDisplay currentUnit;
   Unit::unitScale  currentScale;

   // Since we need to call setupMassMenu() two different ways, we need
   // to figure out the currentUnit and Scale here
   currentUnit  = displayUnit(selected);
   currentScale = displayScale(selected);

   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case FERMINVENTORYCOL:
      case FERMAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(parentTableWidget,currentUnit, currentScale);
         break;
      case FERMCOLORCOL:
         menu = Brewtarget::setupColorMenu(parentTableWidget,currentUnit);
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   QWidget* pMenu = invoked->parentWidget();
   if ( pMenu == menu )
      setDisplayUnit(selected,(Unit::unitDisplay)invoked->data().toInt());
   else
      setDisplayScale(selected,(Unit::unitScale)invoked->data().toInt());

}

bool FermentableTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Fermentable* row;
   bool retVal = false;

   if( index.row() >= (int)fermObs.size() )
   {
      return false;
   }
   else
      row = fermObs[index.row()];

   Unit::unitDisplay dspUnit = displayUnit(index.column());
   Unit::unitScale   dspScl  = displayScale(index.column());

   switch( index.column() )
   {
      case FERMNAMECOL:
         retVal = value.canConvert(QVariant::String);
         if ( retVal )
            row->setName(value.toString());
         break;
      case FERMTYPECOL:
         retVal = value.canConvert(QVariant::Int);
         if ( retVal )
            row->setType( static_cast<Fermentable::Type>(value.toInt()));
         break;
      case FERMINVENTORYCOL:
         retVal = value.canConvert(QVariant::String);
         if( retVal )
            row->setInventoryAmount(Brewtarget::qStringToSI(value.toString(), Units::kilograms,dspUnit,dspScl));
         break;
      case FERMAMOUNTCOL:
         retVal = value.canConvert(QVariant::String);
         if( retVal )
         {
            row->setAmount_kg(Brewtarget::qStringToSI(value.toString(), Units::kilograms,dspUnit,dspScl));
            if( rowCount() > 0 )
               headerDataChanged( Qt::Vertical, 0, rowCount()-1 ); // Need to re-show header (grain percent).
         }
         break;
      case FERMISMASHEDCOL:
         retVal = value.canConvert(QVariant::Int);
         if( retVal )
            row->setAdditionMethod(static_cast<Fermentable::AdditionMethod>(value.toInt()));
         break;
      case FERMAFTERBOIL:
         retVal = value.canConvert(QVariant::Int);
         if( retVal )
            row->setAdditionTime(static_cast<Fermentable::AdditionTime>(value.toInt()));
         break;
      case FERMYIELDCOL:
         retVal = value.canConvert(QVariant::Double);
         if( retVal )
            row->setYield_pct( value.toDouble() );
         break;
      case FERMCOLORCOL:
         retVal = value.canConvert(QVariant::Double);
         if( retVal )
            row->setColor_srm(Brewtarget::qStringToSI(value.toString(), Units::srm, dspUnit, dspScl));
         break;
      default:
         Brewtarget::logW(tr("Bad column: %1").arg(index.column()));
         return false;
   }
   row->save();
   return retVal;
}

Fermentable* FermentableTableModel::getFermentable(unsigned int i)
{
   return fermObs.at(i);
}

//======================CLASS FermentableItemDelegate===========================

FermentableItemDelegate::FermentableItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}

QWidget* FermentableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   if( index.column() == FERMTYPECOL )
   {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Grain"));
      box->addItem(tr("Sugar"));
      box->addItem(tr("Extract"));
      box->addItem(tr("Dry Extract"));
      box->addItem(tr("Adjunct"));

      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      box->setFocusPolicy(Qt::StrongFocus);

      return box;
   }
   else if( index.column() == FERMISMASHEDCOL )
   {
      QComboBox* box = new QComboBox(parent);
      QListWidget* list = new QListWidget(parent);
      list->setResizeMode(QListWidget::Adjust);

      list->addItem(tr("Mashed"));
      list->addItem(tr("Steeped"));
      list->addItem(tr("Not mashed"));
      box->setModel(list->model());
      box->setView(list);

      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      box->setFocusPolicy(Qt::StrongFocus);

      // Can we access to the data model into FermentableItemDelegate ? Yes we can !
      int type = index.model()->index(index.row(), FERMTYPECOL).data(Qt::UserRole).toInt();

      // Hide the unsuitable item keeping the same enumeration
      if(type == Fermentable::Grain)
      {
         list->item(Fermentable::Not_Mashed)->setHidden(true);
      }
      else
      {
         list->item(Fermentable::Steeped)->setHidden(true);
      }

      return box;
   }
   else if( index.column() == FERMAFTERBOIL )
   {
      QComboBox* box = new QComboBox(parent);

      box->addItem(tr("Normal"));
      box->addItem(tr("Late"));

      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      box->setFocusPolicy(Qt::StrongFocus);

      return box;
   }
   else
      return new QLineEdit(parent);
}

void FermentableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   int col = index.column();

   if( col == FERMTYPECOL || col == FERMISMASHEDCOL || col == FERMAFTERBOIL)
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

void FermentableItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   int col = index.column();

   if( col == FERMTYPECOL || col == FERMISMASHEDCOL || col == FERMAFTERBOIL )
   {
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      int value = box->currentIndex();
      int ndx = model->data(index, Qt::UserRole).toInt();

     // Only do something when something needs to be done
      if ( value != ndx )
         model->setData(index, value, Qt::EditRole);
   }
   else if( col == FERMISMASHEDCOL || col == FERMAFTERBOIL )
   {
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      int value = box->currentIndex();
      int ndx = model->data(index, Qt::UserRole).toInt();

     // Only do something when something needs to be done
      if ( value != ndx )
         model->setData(index, value, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = qobject_cast<QLineEdit*>(editor);

      if ( line->isModified() )
          model->setData(index, line->text(), Qt::EditRole);
   }
}

void FermentableItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   editor->setGeometry(option.rect);
}
