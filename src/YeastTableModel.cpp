/*
 * YeastTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2020
 * - Matt Young <mfsy@yahoo.com>
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
#include <QString>
#include <QVector>
#include <QHeaderView>

#include "database.h"
#include "model/Yeast.h"
#include "YeastTableModel.h"
#include "Unit.h"
#include "brewtarget.h"
#include "model/Recipe.h"
#include "MainWindow.h"

YeastTableModel::YeastTableModel(QTableView* parent, bool editable)
   : QAbstractTableModel(parent),
     editable(editable),
     _inventoryEditable(false),
     parentTableWidget(parent),
     recObs(nullptr)
{
   yeastObs.clear();
   setObjectName("yeastTableModel");

   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   parentTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->setWordWrap(false);

   connect(headerView, &QWidget::customContextMenuRequested, this, &YeastTableModel::contextMenu);
   connect( &(Database::instance()), &Database::changedInventory, this, &YeastTableModel::changedInventory );
}

void YeastTableModel::addYeast(Yeast* yeast)
{
   if( yeastObs.contains(yeast) )
      return;
   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if(
      recObs == nullptr &&
      (
         yeast->deleted() ||
         !yeast->display()
      )
   )
      return;
   int size = yeastObs.size();
   beginInsertRows( QModelIndex(), size, size );
   yeastObs.append(yeast);
   connect( yeast, &NamedEntity::changed, this, &YeastTableModel::changed );
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
}

void YeastTableModel::observeRecipe(Recipe* rec)
{
   if( recObs )
   {
      disconnect( recObs, nullptr, this, nullptr );
      removeAll();
   }

   recObs = rec;
   if( recObs )
   {
      connect( recObs, &NamedEntity::changed, this, &YeastTableModel::changed );
      addYeasts( recObs->yeasts() );
   }
}

void YeastTableModel::observeDatabase(bool val)
{
   if( val )
   {
      observeRecipe(nullptr);

      removeAll();
      connect( &(Database::instance()), &Database::newYeastSignal, this, &YeastTableModel::addYeast );
      connect( &(Database::instance()), SIGNAL(deletedSignal(Yeast*)), this, SLOT(removeYeast(Yeast*)) );
      addYeasts( Database::instance().yeasts() );
   }
   else
   {
      removeAll();
      disconnect( &(Database::instance()), nullptr, this, nullptr );
   }
}

void YeastTableModel::addYeasts(QList<Yeast*> yeasts)
{
   QList<Yeast*>::iterator i;
   QList<Yeast*> tmp;

   for( i = yeasts.begin(); i != yeasts.end(); i++ )
   {
      if( recObs == nullptr && ( (*i)->deleted() || !(*i)->display() ) )
         continue;

      if( !yeastObs.contains(*i) )
         tmp.append(*i);
   }

   int size = yeastObs.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      yeastObs.append(tmp);

      for( i = tmp.begin(); i != tmp.end(); i++ )
         connect( *i, &NamedEntity::changed, this, &YeastTableModel::changed );

      endInsertRows();
   }
}

void YeastTableModel::removeYeast(Yeast* yeast)
{
   int i = yeastObs.indexOf(yeast);

   if( i >= 0 )
   {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( yeast, nullptr, this, nullptr );
      yeastObs.removeAt(i);
      //reset(); // Tell everybody the table has changed.
      endRemoveRows();
   }
}

void YeastTableModel::removeAll()
{
   if (yeastObs.size())
   {
      beginRemoveRows( QModelIndex(), 0, yeastObs.size()-1 );
      while( !yeastObs.isEmpty() )
      {
         disconnect( yeastObs.takeLast(), nullptr, this, nullptr );
      }
      endRemoveRows();
   }
}

void YeastTableModel::changedInventory(Brewtarget::DBTable table, int invKey, QVariant val)
{
   if ( table == Brewtarget::YEASTTABLE ) {
      for( int i = 0; i < yeastObs.size(); ++i ) {
         Yeast* holdmybeer = yeastObs.at(i);

         if ( invKey == holdmybeer->inventoryId() ) {
            holdmybeer->setCacheOnly(true);
            holdmybeer->setInventoryQuanta(val.toInt());
            holdmybeer->setCacheOnly(false);
            emit dataChanged( QAbstractItemModel::createIndex(i,YEASTINVENTORYCOL),
                              QAbstractItemModel::createIndex(i,YEASTINVENTORYCOL) );
         }
      }
   }
}
void YeastTableModel::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;

   // Find the notifier in the list
   Yeast* yeastSender = qobject_cast<Yeast*>(sender());
   if( yeastSender )
   {
      i = yeastObs.indexOf(yeastSender);
      if( i < 0 )
         return;

      emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                        QAbstractItemModel::createIndex(i, YEASTNUMCOLS-1));
      return;
   }

   // See if sender is our recipe.
   Recipe* recSender = qobject_cast<Recipe*>(sender());
   if( recSender && recSender == recObs )
   {
      if( QString(prop.name()) == "yeasts" )
      {
         removeAll();
         addYeasts( recObs->yeasts() );
      }
      if( rowCount() > 0 )
         emit headerDataChanged( Qt::Vertical, 0, rowCount()-1 );
      return;
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
   Unit::unitDisplay unit;

   // Ensure the row is ok.
   if( index.row() >= static_cast<int>(yeastObs.size() ))
   {
      qWarning() << tr("Bad model index. row = %1").arg(index.row());
      return QVariant();
   }
   else
      row = yeastObs[index.row()];

   switch( index.column() )
   {
      case YEASTNAMECOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         return QVariant(row->name());
      case YEASTTYPECOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->typeStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->type());
         else
            return QVariant();
      case YEASTLABCOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->laboratory());
         else
            return QVariant();
      case YEASTPRODIDCOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->productID());
         else
            return QVariant();
      case YEASTFORMCOL:
         if( role == Qt::DisplayRole )
            return QVariant(row->formStringTr());
         else if( role == Qt::UserRole )
            return QVariant(row->form());
         else
            return QVariant();
      case YEASTINVENTORYCOL:
         if( role != Qt::DisplayRole )
            return QVariant();
         return QVariant( row->inventory() );
      case YEASTAMOUNTCOL:
         if( role != Qt::DisplayRole )
            return QVariant();

         unit  = displayUnit(index.column());

         return QVariant(
                           Brewtarget::displayAmount( row->amount(),
                                                      row->amountIsWeight() ? static_cast<Unit const *>(&Units::kilograms) : static_cast<Unit const *>(&Units::liters),
                                                      3,
                                                      unit,
                                                      Unit::noScale
                                                   )
                        );

      default :
         qWarning() << tr("Bad column: %1").arg(index.column());
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
         case YEASTINVENTORYCOL:
            return QVariant(tr("Inventory"));
         case YEASTAMOUNTCOL:
            return QVariant(tr("Amount"));
         case YEASTLABCOL:
             return QVariant(tr("Laboratory"));
         case YEASTPRODIDCOL:
             return QVariant(tr("Product ID"));
         default:
            qWarning() << tr("Bad column: %1").arg(section);
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
      case YEASTINVENTORYCOL:
         return (Qt::ItemIsEnabled | (_inventoryEditable ? Qt::ItemIsEditable : Qt::NoItemFlags));
      default:
         return Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool YeastTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Yeast *row;
   Unit const * unit;

   if( index.row() >= static_cast<int>(yeastObs.size()) || role != Qt::EditRole )
      return false;
   else
      row = yeastObs[index.row()];

   Unit::unitDisplay dspUnit = displayUnit(index.column());
   Unit::unitScale   dspScl  = displayScale(index.column());

   switch( index.column() )
   {
      case YEASTNAMECOL:
         if( ! value.canConvert(QVariant::String))
            return false;
         Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                  PropertyNames::NamedEntity::name,
                                                  value.toString(),
                                                  tr("Change Yeast Name"));
         break;
      case YEASTLABCOL:
         if( ! value.canConvert(QVariant::String) )
            return false;
         Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                  PropertyNames::Yeast::laboratory,
                                                  value.toString(),
                                                  tr("Change Yeast Laboratory"));
         break;
      case YEASTPRODIDCOL:
         if( ! value.canConvert(QVariant::String) )
            return false;
         Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                  PropertyNames::Yeast::productID,
                                                  value.toString(),
                                                  tr("Change Yeast Product ID"));
         break;
      case YEASTTYPECOL:
         if( ! value.canConvert(QVariant::Int) )
            return false;
         Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                  "type",
                                                  static_cast<Yeast::Type>(value.toInt()),
                                                  tr("Change Yeast Type"));
         break;
      case YEASTFORMCOL:
         if( ! value.canConvert(QVariant::Int) )
            return false;
         Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                  "form",
                                                  static_cast<Yeast::Form>(value.toInt()),
                                                  tr("Change Yeast Form"));
         break;
      case YEASTINVENTORYCOL:
         if( ! value.canConvert(QVariant::Int) )
            return false;
         Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                  "inventoryQuanta",
                                                  value.toInt(),
                                                  tr("Change Yeast Inventory Unit Size")); // .:TBD:. MY 2020-12-11 Whilst it's admirably concise, I find "quanta" unclear, and I'm not sure it's that easy to translate either
         break;
      case YEASTAMOUNTCOL:
         if( ! value.canConvert(QVariant::String) )
            return false;

         unit = row->amountIsWeight() ? static_cast<Unit const *>(&Units::kilograms) : static_cast<Unit const *>(&Units::liters);

         Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                  "amount",
                                                  Brewtarget::qStringToSI(value.toString(), unit, dspUnit, dspScl),
                                                  tr("Change Yeast Amount"));
         break;

      default:
         qWarning() << tr("Bad column: %1").arg(index.column());
         return false;
   }
   return true;
}

Yeast* YeastTableModel::getYeast(unsigned int i)
{
   return yeastObs[static_cast<int>(i)];
}

Unit::unitDisplay YeastTableModel::displayUnit(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noUnit;

   return static_cast<Unit::unitDisplay>(Brewtarget::option(attribute, QVariant(-1), this->objectName(), Brewtarget::UNIT).toInt());
}

Unit::unitScale YeastTableModel::displayScale(int column) const
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
void YeastTableModel::setDisplayUnit(int column, Unit::unitDisplay displayUnit)
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
void YeastTableModel::setDisplayScale(int column, Unit::unitScale displayScale)
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

QString YeastTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case YEASTAMOUNTCOL:
         attribute = "amount";
         break;
      default:
         attribute = "";
   }
   return attribute;
}

void YeastTableModel::contextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   Unit::unitDisplay currentUnit;
   Unit::unitScale  currentScale;

   currentUnit  = displayUnit(selected);
   currentScale = displayScale(selected);

   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case YEASTAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(parentTableWidget,currentUnit, currentScale, false);
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == nullptr )
      return;

   setDisplayUnit(selected,static_cast<Unit::unitDisplay>(invoked->data().toInt()));
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
      box->setMinimumWidth(box->minimumSizeHint().width());
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
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
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
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      int ndx = index.model()->data(index, Qt::UserRole).toInt();

      box->setCurrentIndex(ndx);
   }
   else
   {
      QLineEdit* line = qobject_cast<QLineEdit*>(editor);

      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }

}

void YeastItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   int col = index.column();

   if( col == YEASTTYPECOL || col == YEASTFORMCOL )
   {
      QComboBox* box = static_cast<QComboBox*>(editor);
      int ndx = box->currentIndex();
      int curr = model->data(index,Qt::UserRole).toInt();

      if ( ndx != curr )
         model->setData(index, ndx, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = static_cast<QLineEdit*>(editor);

      if ( line->isModified() )
         model->setData(index, line->text(), Qt::EditRole);
   }
}

void YeastItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
