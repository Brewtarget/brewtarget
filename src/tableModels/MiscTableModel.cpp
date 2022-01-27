/*
 * MiscTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#include "tableModels/MiscTableModel.h"

#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>

#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"


MiscTableModel::MiscTableModel(QTableView* parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {{MISCNAMECOL,      {tr("Name"),        NonPhysicalQuantity::String,          ""                                                 }},
       {MISCTYPECOL,      {tr("Type"),        NonPhysicalQuantity::String,          ""                                                 }},
       {MISCUSECOL,       {tr("Use"),         NonPhysicalQuantity::String,          ""                                                 }},
       {MISCTIMECOL,      {tr("Time"),        Measurement::PhysicalQuantity::Time,  *PropertyNames::Misc::time                         }},
       {MISCAMOUNTCOL,    {tr("Amount"),      Measurement::PhysicalQuantity::Mixed, *PropertyNames::Misc::amount                       }},
       {MISCINVENTORYCOL, {tr("Inventory"),   Measurement::PhysicalQuantity::Mixed, *PropertyNames::NamedEntityWithInventory::inventory}},
       {MISCISWEIGHT,     {tr("Amount Type"), NonPhysicalQuantity::String,          ""                                                 }}}
   },
   _inventoryEditable(false),
   recObs(nullptr) {
   miscObs.clear();
   setObjectName("miscTableModel");

   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   parentTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->setWordWrap(false);

   connect(headerView, &QWidget::customContextMenuRequested, this, &MiscTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryMisc>::getInstance(),
           &ObjectStoreTyped<InventoryMisc>::signalPropertyChanged,
           this,
           &MiscTableModel::changedInventory);
   return;
}

MiscTableModel::~MiscTableModel() = default;

void MiscTableModel::observeRecipe(Recipe* rec)
{
   if( recObs )
   {
      disconnect( recObs, nullptr, this, nullptr );
      removeAll();
   }

   recObs = rec;
   if( recObs )
   {
      connect( recObs, &NamedEntity::changed, this, &MiscTableModel::changed );
      addMiscs( recObs->miscs() );
   }
}

void MiscTableModel::observeDatabase(bool val) {
   if (val) {
      observeRecipe(nullptr);
      removeAll();
      connect(&ObjectStoreTyped<Misc>::getInstance(), &ObjectStoreTyped<Misc>::signalObjectInserted,  this, &MiscTableModel::addMisc);
      connect(&ObjectStoreTyped<Misc>::getInstance(), &ObjectStoreTyped<Misc>::signalObjectDeleted,   this, &MiscTableModel::removeMisc);
      addMiscs( ObjectStoreTyped<Misc>::getInstance().getAllRaw() );
   } else {
      removeAll();
      disconnect(&ObjectStoreTyped<Misc>::getInstance(), nullptr, this, nullptr );
   }
   return;
}

void MiscTableModel::addMisc(int miscId) {
   Misc* misc = ObjectStoreWrapper::getByIdRaw<Misc>(miscId);

   if( miscObs.contains(misc) ) {
      return;
   }

   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if (recObs == nullptr &&
       (misc->deleted() || !misc->display())) {
      return;
   }

   // If we are watching a Recipe and the new Misc does not belong to it then there is nothing for us to do
   if (this->recObs) {
      Recipe * recipeOfNewMisc = misc->getOwningRecipe();
      if (recipeOfNewMisc && this->recObs->key() != recipeOfNewMisc->key()) {
         qDebug() <<
            Q_FUNC_INFO << "Ignoring signal about new Misc #" << misc->key() << "as it belongs to Recipe #" <<
            recipeOfNewMisc->key() << "and we are watching Recipe #" << this->recObs->key();
         return;
      }
   }

   int size = miscObs.size();
   beginInsertRows( QModelIndex(), size, size );
   miscObs.append(misc);
   connect( misc, &NamedEntity::changed, this, &MiscTableModel::changed );
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
   return;
}

void MiscTableModel::addMiscs(QList<Misc*> miscs)
{
   QList<Misc*>::iterator i;
   QList<Misc*> tmp;

   for( i = miscs.begin(); i != miscs.end(); i++ )
   {
      if( recObs == nullptr && ( (*i)->deleted() || !(*i)->display() ) )
         continue;
      if( !miscObs.contains(*i) )
         tmp.append(*i);
   }

   int size = miscObs.size();
   if (size+tmp.size())
   {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      miscObs.append(tmp);

      for( i = tmp.begin(); i != tmp.end(); i++ )
         connect( *i, &NamedEntity::changed, this, &MiscTableModel::changed );

      endInsertRows();
   }
}

// Returns true when misc is successfully found and removed.
void MiscTableModel::removeMisc(int miscId, std::shared_ptr<QObject> object) {
   this->remove(std::static_pointer_cast<Misc>(object).get());
   return;
}

bool MiscTableModel::remove(Misc * misc) {
   int i = miscObs.indexOf(misc);
   if( i >= 0 ) {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( misc, nullptr, this, nullptr );
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
         disconnect( miscObs.takeLast(), nullptr, this, nullptr );
      }
      endRemoveRows();
   }
}

int MiscTableModel::rowCount(const QModelIndex& /*parent*/) const {
   return miscObs.size();
}

QVariant MiscTableModel::data(QModelIndex const & index, int role) const {

   // Ensure the row is ok.
   if (index.row() >= static_cast<int>(this->miscObs.size())) {
      qWarning() << Q_FUNC_INFO << "Bad model index. row = " << index.row();
      return QVariant();
   }

   Misc * row = this->miscObs[index.row()];

   // Deal with the column and return the right data.
   int const column = index.column();
   switch (column) {
      case MISCNAMECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->name());
         }
         break;
      case MISCTYPECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->typeStringTr());
         }
         if (role == Qt::UserRole) {
            return QVariant(row->type());
         }
         break;
      case MISCUSECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->useStringTr());
         }
         if (role == Qt::UserRole) {
            return QVariant(row->use());
         }
         return QVariant();
      case MISCTIMECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(Measurement::displayAmount(Measurement::Amount{row->time(), Measurement::Units::minutes},
                                                       3,
                                                       std::nullopt,
                                                       this->getForcedRelativeScaleForColumn(column)));
         }
         break;
      case MISCINVENTORYCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(
               Measurement::displayAmount(Measurement::Amount{
                                             row->inventory(),
                                             row->amountIsWeight() ? Measurement::Units::kilograms :
                                                                     Measurement::Units::liters
                                          },
                                          3,
                                          this->getForcedSystemOfMeasurementForColumn(column),
                                          std::nullopt)
            );
         }
         break;
      case MISCAMOUNTCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(
               Measurement::displayAmount(Measurement::Amount{
                                             row->amount(),
                                             row->amountIsWeight() ? Measurement::Units::kilograms :
                                                                     Measurement::Units::liters
                                          },
                                          3,
                                          this->getForcedSystemOfMeasurementForColumn(column),
                                          std::nullopt)
            );
         }
         break;
      case MISCISWEIGHT:
         if (role == Qt::DisplayRole) {
            return QVariant(row->amountTypeStringTr());
         }
         if (role == Qt::UserRole) {
            return QVariant(row->amountType());
         }
         break;
      default:
         qWarning() << Q_FUNC_INFO << "Bad model index. column = " << column;
         break;
   }
   return QVariant();
}

QVariant MiscTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumName(section);
   }
   return QVariant();
}

Qt::ItemFlags MiscTableModel::flags(QModelIndex const & index) const {
   int col = index.column();
   Qt::ItemFlags defaults = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
   switch (col) {
      case MISCNAMECOL:
         return defaults;
      case MISCINVENTORYCOL:
         return (defaults | (_inventoryEditable ? Qt::ItemIsEditable : Qt::NoItemFlags));
      default:
         return defaults | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags);
   }
}

bool MiscTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {

   if (index.row() >= static_cast<int>(miscObs.size())) {
      return false;
   }

   Misc *row = miscObs[index.row()];


   Measurement::PhysicalQuantity physicalQuantity =
      row->amountIsWeight() ? Measurement::PhysicalQuantity::Mass: Measurement::PhysicalQuantity::Volume;

   int column = index.column();
   switch (column) {
      case MISCNAMECOL:
         if (value.canConvert(QVariant::String)) {
            MainWindow::instance().doOrRedoUpdate(*row,
                                                  PropertyNames::NamedEntity::name,
                                                  value.toString(),
                                                  tr("Change Misc Name"));
         } else {
            return false;
         }
         break;
      case MISCTYPECOL:
         if (!value.canConvert(QVariant::Int)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::Misc::type,
                                               static_cast<Misc::Type>(value.toInt()),
                                               tr("Change Misc Type"));
         break;
      case MISCUSECOL:
         if (!value.canConvert(QVariant::Int)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::Misc::use,
                                               static_cast<Misc::Use>(value.toInt()),
                                               tr("Change Misc Use"));
         break;
      case MISCTIMECOL:
         if (!value.canConvert(QVariant::String)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(
            *row,
            PropertyNames::Misc::time,
            Measurement::qStringToSI(value.toString(),
                                     Measurement::PhysicalQuantity::Time,
                                     this->getForcedSystemOfMeasurementForColumn(column),
                                     this->getForcedRelativeScaleForColumn(column)).quantity,
            tr("Change Misc Time")
         );
         break;
      case MISCINVENTORYCOL:
         if (!value.canConvert(QVariant::String)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(
            *row,
            PropertyNames::NamedEntityWithInventory::inventory,
            Measurement::qStringToSI(value.toString(),
                                     physicalQuantity,
                                     this->getForcedSystemOfMeasurementForColumn(column),
                                     this->getForcedRelativeScaleForColumn(column)).quantity,
            tr("Change Misc Inventory Amount")
         );
         break;
      case MISCAMOUNTCOL:
         if (!value.canConvert(QVariant::String)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(
            *row,
            PropertyNames::Misc::amount,
            Measurement::qStringToSI(value.toString(),
                                     physicalQuantity,
                                     this->getForcedSystemOfMeasurementForColumn(column),
                                     this->getForcedRelativeScaleForColumn(column)).quantity,
            tr("Change Misc Amount")
         );
         break;
      case MISCISWEIGHT:
         if (!value.canConvert(QVariant::Int)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::Misc::amountType,
                                               static_cast<Misc::AmountType>(value.toInt()),
                                               tr("Change Misc Amount Type"));
         break;
      default:
         return false;
   }

   emit dataChanged(index, index);
   return true;
}

void MiscTableModel::changedInventory(int invKey, BtStringConst const & propertyName) {
   if (propertyName == PropertyNames::Inventory::amount) {
      for( int i = 0; i < miscObs.size(); ++i ) {
         Misc* holdmybeer = miscObs.at(i);

         if ( invKey == holdmybeer->inventoryId() ) {
            // No need to update amount as it's only stored in one place (the inventory object) now
            emit dataChanged( QAbstractItemModel::createIndex(i,MISCINVENTORYCOL),
                              QAbstractItemModel::createIndex(i,MISCINVENTORYCOL) );
         }
      }
   }
   return;
}

void MiscTableModel::changed(QMetaProperty prop, QVariant /*val*/) {
   Misc * miscSender = qobject_cast<Misc*>(sender());
   if (miscSender) {
      int i = miscObs.indexOf(miscSender);
      if (i < 0) {
         return;
      }

      emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                        QAbstractItemModel::createIndex(i, MISCNUMCOLS-1) );
      return;
   }

   // See if sender is our recipe.
   Recipe* recSender = qobject_cast<Recipe*>(sender());
   if (recSender && recSender == recObs) {
      if (QString(prop.name()) == PropertyNames::Recipe::miscIds) {
         removeAll();
         addMiscs( recObs->miscs() );
      }
      if (rowCount() > 0) {
         emit headerDataChanged( Qt::Vertical, 0, rowCount()-1 );
      }
      return;
   }

   return;
}

Misc * MiscTableModel::getMisc(unsigned int i) {
   return miscObs[static_cast<int>(i)];
}

//======================CLASS MiscItemDelegate===========================

MiscItemDelegate::MiscItemDelegate(QObject* parent) : QItemDelegate(parent) {
   return;
}

QWidget* MiscItemDelegate::createEditor(QWidget *parent,
                                        QStyleOptionViewItem const & /*option*/,
                                        QModelIndex const & index) const {
   int const column = index.column();
   if (column == MISCTYPECOL) {
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

   if (column == MISCUSECOL) {
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

   if (column == MISCISWEIGHT) {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Weight"));
      box->addItem(tr("Volume"));
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }

   return new QLineEdit(parent);
}

void MiscItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
   int const column = index.column();

   if( column == MISCTYPECOL || column == MISCUSECOL || column == MISCISWEIGHT)
   {
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      if( box == nullptr )
         return;
      box->setCurrentIndex(index.model()->data(index, Qt::UserRole).toInt());
   }
   else
   {
      QLineEdit* line = static_cast<QLineEdit*>(editor);

      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }
}

void MiscItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
   int column = index.column();
   if( column == MISCTYPECOL || column == MISCUSECOL || column == MISCISWEIGHT)
   {
      QComboBox* box = static_cast<QComboBox*>(editor);
      int ndx = box->currentIndex();
      int curr = model->data(index, Qt::UserRole).toInt();

      if ( curr != ndx )
         model->setData(index, ndx, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = static_cast<QLineEdit*>(editor);

      if ( line->isModified() )
         model->setData(index, line->text(), Qt::EditRole);
   }
}

void MiscItemDelegate::updateEditorGeometry(QWidget * editor,
                                            QStyleOptionViewItem const & option,
                                            QModelIndex const & index) const {
   editor->setGeometry(option.rect);
}
