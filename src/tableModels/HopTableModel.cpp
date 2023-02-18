/*
 * HopTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Luke Vincent <luke.r.vincent@gmail.com>
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
#include "tableModels/HopTableModel.h"

#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QComboBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QModelIndex>
#include <QString>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "Localization.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Hop.h"
#include "model/Inventory.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"

HopTableModel::HopTableModel(QTableView * parent, bool editable) :
   BtTableModelInventory{
      parent,
      editable,
      {{HOPNAMECOL,      {tr("Name"),      NonPhysicalQuantity::String,          ""                                                 }},
       {HOPALPHACOL,     {tr("Alpha %"),   NonPhysicalQuantity::Percentage,      ""                                                 }},
       {HOPAMOUNTCOL,    {tr("Amount"),    Measurement::PhysicalQuantity::Mass,  *PropertyNames::Hop::amount_kg                     }},
       {HOPINVENTORYCOL, {tr("Inventory"), Measurement::PhysicalQuantity::Mass,  *PropertyNames::NamedEntityWithInventory::inventory}},
       {HOPFORMCOL,      {tr("Form"),      NonPhysicalQuantity::String,          ""                                                 }},
       {HOPUSECOL,       {tr("Use"),       NonPhysicalQuantity::String,          ""                                                 }},
       {HOPTIMECOL,      {tr("Time"),      Measurement::PhysicalQuantity::Time,  *PropertyNames::Hop::time_min                      }}}
   },
   showIBUs(false) {
   this->rows.clear();
   this->setObjectName("hopTable");

   QHeaderView * headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   parentTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->setWordWrap(false);

   connect(headerView, &QWidget::customContextMenuRequested, this, &HopTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryHop>::getInstance(), &ObjectStoreTyped<InventoryHop>::signalPropertyChanged, this,
           &HopTableModel::changedInventory);
   return;
}

HopTableModel::~HopTableModel() {
   this->rows.clear();
   return;
}

void HopTableModel::observeRecipe(Recipe * rec) {
   if (this->recObs) {
      disconnect(this->recObs, nullptr, this, nullptr);
      removeAll();
   }

   this->recObs = rec;
   if (this->recObs) {
      connect(this->recObs, &NamedEntity::changed, this, &HopTableModel::changed);
      this->addHops(this->recObs->getAll<Hop>());
   }
}

void HopTableModel::observeDatabase(bool val) {
   if (val) {
      observeRecipe(nullptr);
      removeAll();
      connect(&ObjectStoreTyped<Hop>::getInstance(), &ObjectStoreTyped<Hop>::signalObjectInserted, this,
              &HopTableModel::addHop);
      connect(&ObjectStoreTyped<Hop>::getInstance(),
              &ObjectStoreTyped<Hop>::signalObjectDeleted,
              this,
              &HopTableModel::removeHop);
      this->addHops(ObjectStoreWrapper::getAll<Hop>());
   } else {
      removeAll();
      disconnect(&ObjectStoreTyped<Hop>::getInstance(), nullptr, this, nullptr);
   }
   return;
}

void HopTableModel::addHop(int hopId) {
   auto hopAdded = ObjectStoreTyped<Hop>::getInstance().getById(hopId);
   if (!hopAdded) {
      // Not sure this should ever happen in practice, but, if there ever is no hop with the specified ID, there's not
      // a lot we can do.
      qWarning() << Q_FUNC_INFO << "Received signal that Hop ID" << hopId << "added, but unable to retrieve the Hop";
      return;
   }

   if (this->rows.contains(hopAdded)) {
      return;
   }

   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if (this->recObs == nullptr && (hopAdded->deleted() || !hopAdded->display())) {
      return;
   }

   // If we are watching a Recipe and the new Hop does not belong to it then there is nothing for us to do
   if (this->recObs) {
      Recipe * recipeOfNewHop = hopAdded->getOwningRecipe();
      if (recipeOfNewHop && this->recObs->key() != recipeOfNewHop->key()) {
         qDebug() <<
            Q_FUNC_INFO << "Ignoring signal about new Hop #" << hopAdded->key() << "as it belongs to Recipe #" <<
            recipeOfNewHop->key() << "and we are watching Recipe #" << this->recObs->key();
         return;
      }
   }

   int size = this->rows.size();
   beginInsertRows(QModelIndex(), size, size);
   this->rows.append(hopAdded);
   connect(hopAdded.get(), &NamedEntity::changed, this, &HopTableModel::changed);
   endInsertRows();
   return;
}

void HopTableModel::addHops(QList< std::shared_ptr<Hop> > hops) {
   decltype(hops) tmp;

   for (auto hop : hops) {
      if (recObs == nullptr && (hop->deleted() || !hop->display())) {
         continue;
      }
      if (!this->rows.contains(hop)) {
         tmp.append(hop);
      }
   }

   int size = this->rows.size();
   if (size + tmp.size()) {
      beginInsertRows(QModelIndex(), size, size + tmp.size() - 1);
      this->rows.append(tmp);

      for (auto hop : tmp) {
         connect(hop.get(), &NamedEntity::changed, this, &HopTableModel::changed);
      }

      endInsertRows();
   }
   return;
}

bool HopTableModel::remove(std::shared_ptr<Hop> hop) {
   int i = this->rows.indexOf(hop);
   if (i >= 0) {
      beginRemoveRows(QModelIndex(), i, i);
      disconnect(hop.get(), nullptr, this, nullptr);
      this->rows.removeAt(i);
      //reset(); // Tell everybody the table has changed.
      endRemoveRows();

      return true;
   }

   return false;
}

void HopTableModel::removeHop([[maybe_unused]] int hopId,
                              std::shared_ptr<QObject> object) {
   this->remove(std::static_pointer_cast<Hop>(object));
   return;
}

void HopTableModel::setShowIBUs(bool var) {
   showIBUs = var;
}

void HopTableModel::removeAll() {
   if (this->rows.size()) {
      beginRemoveRows(QModelIndex(), 0, this->rows.size() - 1);
      while (!this->rows.isEmpty()) {
         disconnect(this->rows.takeLast().get(), nullptr, this, nullptr);
      }
      endRemoveRows();
   }
}

void HopTableModel::changedInventory(int invKey, BtStringConst const & propertyName) {
   if (propertyName == PropertyNames::Inventory::amount) {
      for (int ii = 0; ii < this->rows.size(); ++ii) {
         if (invKey == this->rows.at(ii)->inventoryId()) {
            emit dataChanged(QAbstractItemModel::createIndex(ii, HOPINVENTORYCOL),
                             QAbstractItemModel::createIndex(ii, HOPINVENTORYCOL));
         }
      }
   }
   return;
}

void HopTableModel::changed(QMetaProperty prop, [[maybe_unused]] QVariant val) {

   // Find the notifier in the list
   Hop * hopSender = qobject_cast<Hop *>(sender());
   if (hopSender) {
      int ii = this->findIndexOf(hopSender);
      if (ii < 0) {
         return;
      }

      emit dataChanged(QAbstractItemModel::createIndex(ii, 0),
                       QAbstractItemModel::createIndex(ii, HOPNUMCOLS - 1));
      emit headerDataChanged(Qt::Vertical, ii, ii);
      return;
   }

   // See if sender is our recipe.
   Recipe * recSender = qobject_cast<Recipe *>(sender());
   if (recSender && recSender == recObs) {
      if (QString(prop.name()) == PropertyNames::Recipe::hopIds) {
         removeAll();
         this->addHops(recObs->getAll<Hop>());
      }
      if (rowCount() > 0) {
         emit headerDataChanged(Qt::Vertical, 0, rowCount() - 1);
      }
      return;
   }
}

int HopTableModel::rowCount(const QModelIndex & /*parent*/) const {
   return this->rows.size();
}

QVariant HopTableModel::data(const QModelIndex & index, int role) const {


   // Ensure the row is ok.
   if (index.row() >= static_cast<int>(this->rows.size())) {
      qWarning() << Q_FUNC_INFO << "Bad model index. row =" << index.row();
      return QVariant();
   }

   auto row = this->rows[index.row()];

   int const column = index.column();
   switch (column) {
      case HOPNAMECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->name());
         }
         break;
      case HOPALPHACOL:
         if (role == Qt::DisplayRole) {
            return QVariant(Measurement::displayQuantity(row->alpha_pct(), 3));
         }
         break;
      case HOPINVENTORYCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(Measurement::displayAmount(Measurement::Amount{row->inventory(), Measurement::Units::kilograms},
                                                       3,
                                                       this->getForcedSystemOfMeasurementForColumn(column),
                                                       this->getForcedRelativeScaleForColumn(column)));
         }
         break;
      case HOPAMOUNTCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(Measurement::displayAmount(Measurement::Amount{row->amount_kg(), Measurement::Units::kilograms},
                                                       3,
                                                       this->getForcedSystemOfMeasurementForColumn(column),
                                                       this->getForcedRelativeScaleForColumn(column)));
         }
         break;
      case HOPUSECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(Hop::useDisplayNames[row->use()]);
         }
         if (role == Qt::UserRole) {
            return QVariant(static_cast<int>(row->use()));
         }
         break;
      case HOPTIMECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(Measurement::displayAmount(Measurement::Amount{row->time_min(), Measurement::Units::minutes},
                                                       3,
                                                       std::nullopt,
                                                       this->getForcedRelativeScaleForColumn(column)));
         }
         break;
      case HOPFORMCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(Hop::formDisplayNames[row->form()]);
         } else if (role == Qt::UserRole) {
            return QVariant(static_cast<int>(row->form()));
         }
         break;
      default :
         qWarning() << Q_FUNC_INFO << "Bad column: " << column;
         break;
   }
   return QVariant();
}

QVariant HopTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumName(section);
   }
   if (showIBUs && recObs && orientation == Qt::Vertical && role == Qt::DisplayRole) {
      QList<double> ibus = recObs->IBUs();

      if (ibus.size() > section) {
         return QVariant(QString("%L1 IBU").arg(ibus.at(section), 0, 'f', 1));
      }
   }
   return QVariant();
}

Qt::ItemFlags HopTableModel::flags(const QModelIndex & index) const {
   int col = index.column();
   switch (col) {
      case HOPNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      case HOPINVENTORYCOL:
         return Qt::ItemIsEnabled | (this->isInventoryEditable() ? Qt::ItemIsEditable : Qt::NoItemFlags);
      default:
         return Qt::ItemIsSelectable | (this->editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled |
                       Qt::ItemIsEnabled;
   }
}

bool HopTableModel::setData(const QModelIndex & index, const QVariant & value, int role) {
   bool retVal = false;
   double amt;

   if (index.row() >= static_cast<int>(this->rows.size()) || role != Qt::EditRole) {
      return false;
   }

   auto row = this->rows[index.row()];

   int const column = index.column();
   switch (column) {
      case HOPNAMECOL:
         retVal = value.canConvert(QVariant::String);
         if (retVal) {
            MainWindow::instance().doOrRedoUpdate(*row,
                                                  PropertyNames::NamedEntity::name,
                                                  value.toString(),
                                                  tr("Change Hop Name"));
         }
         break;
      case HOPALPHACOL:
         retVal = value.canConvert(QVariant::Double);
         if (retVal) {
            amt = Localization::toDouble(value.toString(), &retVal);
            if (!retVal) {
               qWarning() << Q_FUNC_INFO << "Could not convert" << value.toString() << "to double";
            }
            MainWindow::instance().doOrRedoUpdate(*row,
                                                  PropertyNames::Hop::alpha_pct,
                                                  amt,
                                                  tr("Change Hop Alpha %"));
         }
         break;

      case HOPINVENTORYCOL:
         retVal = value.canConvert(QVariant::String);
         if (retVal) {
            MainWindow::instance().doOrRedoUpdate(
               *row,
               PropertyNames::NamedEntityWithInventory::inventory,
               Measurement::qStringToSI(value.toString(),
                                        Measurement::PhysicalQuantity::Mass,
                                        this->getForcedSystemOfMeasurementForColumn(column),
                                        this->getForcedRelativeScaleForColumn(column)).quantity(),
               tr("Change Hop Inventory Amount")
            );
         }
         break;
      case HOPAMOUNTCOL:
         retVal = value.canConvert(QVariant::String);
         if (retVal) {
            MainWindow::instance().doOrRedoUpdate(
               *row,
               PropertyNames::Hop::amount_kg,
               Measurement::qStringToSI(value.toString(),
                                        Measurement::PhysicalQuantity::Mass,
                                        this->getForcedSystemOfMeasurementForColumn(column),
                                        this->getForcedRelativeScaleForColumn(column)).quantity(),
               tr("Change Hop Amount")
            );
         }
         break;
      case HOPUSECOL:
         retVal = value.canConvert(QVariant::Int);
         if (retVal) {
            MainWindow::instance().doOrRedoUpdate(*row,
                                                  PropertyNames::Hop::use,
                                                  value.toInt(),
                                                  tr("Change Hop Use"));
         }
         break;
      case HOPFORMCOL:
         retVal = value.canConvert(QVariant::Int);
         if (retVal) {
            MainWindow::instance().doOrRedoUpdate(*row,
                                                  PropertyNames::Hop::form,
                                                  value.toInt(),
                                                  tr("Change Hop Form"));
         }
         break;
      case HOPTIMECOL:
         retVal = value.canConvert(QVariant::String);
         if (retVal) {
            MainWindow::instance().doOrRedoUpdate(
               *row,
               PropertyNames::Hop::time_min,
               Measurement::qStringToSI(value.toString(),
                                        Measurement::PhysicalQuantity::Time,
                                        std::nullopt,
                                        this->getForcedRelativeScaleForColumn(column)).quantity(),
               tr("Change Hop Time")
            );
         }
         break;
      default:
         qWarning() << Q_FUNC_INFO << "Bad column: " << column;
         return false;
   }

   if (retVal) {
      headerDataChanged(Qt::Vertical, index.row(), index.row());   // Need to re-show header (IBUs).
   }

   return retVal;
}

//==========================CLASS HopItemDelegate===============================

HopItemDelegate::HopItemDelegate(QObject * parent)
   : QItemDelegate(parent) {
}

QWidget * HopItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & /*option*/,
                                        const QModelIndex & index) const {
   if (index.column() == HOPUSECOL) {
      QComboBox * box = new QComboBox(parent);

      // NOTE: these need to be in the same order as the Hop::Use enum.
      box->addItem(tr("Mash"));
      box->addItem(tr("First Wort"));
      box->addItem(tr("Boil"));
      box->addItem(tr("Aroma"));
      box->addItem(tr("Dry Hop"));
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   } else if (index.column() == HOPFORMCOL) {
      QComboBox * box = new QComboBox(parent);

      box->addItem(tr("Leaf"));
      box->addItem(tr("Pellet"));
      box->addItem(tr("Plug"));
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   } else {
      return new QLineEdit(parent);
   }
}

void HopItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
   if (index.column() == HOPUSECOL) {
      QComboBox * box = static_cast<QComboBox *>(editor);
      int ndx = index.model()->data(index, Qt::UserRole).toInt();

      box->setCurrentIndex(ndx);
   } else if (index.column() == HOPFORMCOL) {
      QComboBox * box = static_cast<QComboBox *>(editor);
      int ndx = index.model()->data(index, Qt::UserRole).toInt();

      box->setCurrentIndex(ndx);
   } else {
      QLineEdit * line = static_cast<QLineEdit *>(editor);
      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }
}

void HopItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
   if (index.column() == HOPUSECOL) {
      QComboBox * box = static_cast<QComboBox *>(editor);
      int value = box->currentIndex();
      int ndx = model->data(index, Qt::UserRole).toInt();

      if (value != ndx) {
         model->setData(index, value, Qt::EditRole);
      }
   } else if (index.column() == HOPFORMCOL) {
      QComboBox * box = static_cast<QComboBox *>(editor);
      int value = box->currentIndex();
      int ndx = model->data(index, Qt::UserRole).toInt();

      if (value != ndx) {
         model->setData(index, value, Qt::EditRole);
      }
   } else {
      QLineEdit * line = static_cast<QLineEdit *>(editor);
      if (line->isModified()) {
         model->setData(index, line->text(), Qt::EditRole);
      }
   }
}

void HopItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option,
                                           const QModelIndex & /*index*/) const {
   editor->setGeometry(option.rect);
}
