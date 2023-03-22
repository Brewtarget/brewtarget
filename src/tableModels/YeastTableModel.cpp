/*
 * YeastTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
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
#include "tableModels/YeastTableModel.h"

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
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"

YeastTableModel::YeastTableModel(QTableView * parent, bool editable) :
   BtTableModelInventory{
      parent,
      editable,
      {{YEASTNAMECOL,      {tr("Name"),       NonPhysicalQuantity::String                   , ""                           }},
       {YEASTLABCOL,       {tr("Laboratory"), NonPhysicalQuantity::String                   , ""                           }},
       {YEASTPRODIDCOL,    {tr("Product ID"), NonPhysicalQuantity::String                   , ""                           }},
       {YEASTTYPECOL,      {tr("Type"),       NonPhysicalQuantity::String                   , ""                           }},
       {YEASTFORMCOL,      {tr("Form"),       NonPhysicalQuantity::String                   , ""                           }},
       {YEASTAMOUNTCOL,    {tr("Amount"),     Measurement::PhysicalQuantity::Mixed, "amount"}},
       {YEASTINVENTORYCOL, {tr("Inventory"),  NonPhysicalQuantity::Count                    , ""                           }}}
   },
   BtTableModelData<Yeast>{} {

   setObjectName("yeastTableModel");

   QHeaderView * headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   parentTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->setWordWrap(false);

   connect(headerView, &QWidget::customContextMenuRequested, this, &YeastTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryYeast>::getInstance(), &ObjectStoreTyped<InventoryYeast>::signalPropertyChanged,
           this, &YeastTableModel::changedInventory);
   return;
}

YeastTableModel::~YeastTableModel() = default;

void YeastTableModel::addYeast(int yeastId) {
   auto yeast = ObjectStoreWrapper::getById<Yeast>(yeastId);

   if (this->rows.contains(yeast)) {
      return;
   }

   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if (!this->recObs && (yeast->deleted() || !yeast->display())) {
      return;
   }

   // If we are watching a Recipe and the new Yeast does not belong to it then there is nothing for us to do
   if (this->recObs) {
      Recipe * recipeOfNewYeast = yeast->getOwningRecipe();
      if (recipeOfNewYeast && this->recObs->key() != recipeOfNewYeast->key()) {
         qDebug() <<
            Q_FUNC_INFO << "Ignoring signal about new Yeast #" << yeast->key() << "as it belongs to Recipe #" <<
            recipeOfNewYeast->key() << "and we are watching Recipe #" << this->recObs->key();
         return;
      }
   }

   int size = this->rows.size();
   beginInsertRows(QModelIndex(), size, size);
   this->rows.append(yeast);
   connect(yeast.get(), &NamedEntity::changed, this, &YeastTableModel::changed);
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
}

void YeastTableModel::observeRecipe(Recipe * rec) {
   if (this->recObs) {
      disconnect(this->recObs, nullptr, this, nullptr);
      removeAll();
   }

   this->recObs = rec;
   if (this->recObs) {
      connect(this->recObs, &NamedEntity::changed, this, &YeastTableModel::changed);
      addYeasts(this->recObs->getAll<Yeast>());
   }
   return;
}

void YeastTableModel::observeDatabase(bool val) {
   if (val) {
      observeRecipe(nullptr);

      removeAll();
      connect(&ObjectStoreTyped<Yeast>::getInstance(),
              &ObjectStoreTyped<Yeast>::signalObjectInserted,
              this,
              &YeastTableModel::addYeast);
      connect(&ObjectStoreTyped<Yeast>::getInstance(),
              &ObjectStoreTyped<Yeast>::signalObjectDeleted,
              this,
              &YeastTableModel::removeYeast);
      this->addYeasts(ObjectStoreWrapper::getAll<Yeast>());
   } else {
      removeAll();
      disconnect(&ObjectStoreTyped<Yeast>::getInstance(), nullptr, this, nullptr);
   }
   return;
}

void YeastTableModel::addYeasts(QList<std::shared_ptr<Yeast> > yeasts) {
   auto tmp = this->removeDuplicates(yeasts, this->recObs);

   int size = this->rows.size();
   if (size + tmp.size()) {
      beginInsertRows(QModelIndex(), size, size + tmp.size() - 1);
      this->rows.append(tmp);

      for (auto yeast : tmp) {
         connect(yeast.get(), &NamedEntity::changed, this, &YeastTableModel::changed);
      }

      endInsertRows();
   }
   return;
}

void YeastTableModel::removeYeast([[maybe_unused]] int yeastId,
                                  std::shared_ptr<QObject> object) {
   this->remove(std::static_pointer_cast<Yeast>(object));
   return;
}

void YeastTableModel::remove(std::shared_ptr<Yeast> yeast) {

   int i = this->rows.indexOf(yeast);

   if (i >= 0) {
      beginRemoveRows(QModelIndex(), i, i);
      disconnect(yeast.get(), nullptr, this, nullptr);
      this->rows.removeAt(i);
      //reset(); // Tell everybody the table has changed.
      endRemoveRows();
   }
   return;
}

void YeastTableModel::removeAll() {
   if (this->rows.size()) {
      beginRemoveRows(QModelIndex(), 0, this->rows.size() - 1);
      while (!this->rows.isEmpty()) {
         disconnect(this->rows.takeLast().get(), nullptr, this, nullptr);
      }
      endRemoveRows();
   }
}

void YeastTableModel::changedInventory(int invKey, BtStringConst const & propertyName) {
   if (propertyName == PropertyNames::Inventory::amount) {
      for (int ii = 0; ii < this->rows.size(); ++ii) {
         if (invKey == this->rows.at(ii)->inventoryId()) {
            emit dataChanged(QAbstractItemModel::createIndex(ii, YEASTINVENTORYCOL),
                             QAbstractItemModel::createIndex(ii, YEASTINVENTORYCOL));
         }
      }
   }
   return;
}

void YeastTableModel::changed(QMetaProperty prop, QVariant /*val*/) {
   // Find the notifier in the list
   Yeast * yeastSender = qobject_cast<Yeast *>(sender());
   if (yeastSender) {
      int ii = this->findIndexOf(yeastSender);
      if (ii >= 0) {
         emit dataChanged(QAbstractItemModel::createIndex(ii, 0),
                          QAbstractItemModel::createIndex(ii, YEASTNUMCOLS - 1));
      }
      return;
   }

   // See if sender is our recipe.
   Recipe * recSender = qobject_cast<Recipe *>(sender());
   if (recSender && recSender == recObs) {
      if (QString(prop.name()) == PropertyNames::Recipe::yeastIds) {
         removeAll();
         this->addYeasts(this->recObs->getAll<Yeast>());
      }
      if (rowCount() > 0) {
         emit headerDataChanged(Qt::Vertical, 0, rowCount() - 1);
      }
      return;
   }
}

int YeastTableModel::rowCount(const QModelIndex & /*parent*/) const {
   return this->rows.size();
}

QVariant YeastTableModel::data(QModelIndex const & index, int role) const {
   // Ensure the row is ok.
   if (index.row() >= static_cast<int>(this->rows.size())) {
      qWarning() << Q_FUNC_INFO << "Bad model index. row = " << index.row();
      return QVariant();
   }

   auto row = this->rows[index.row()];

   int const column = index.column();
   switch (column) {
      case YEASTNAMECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->name());
         }
         break;
      case YEASTTYPECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->typeStringTr());
         }
         if (role == Qt::UserRole) {
            return QVariant(static_cast<int>(row->type()));
         }
         break;
      case YEASTLABCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->laboratory());
         }
         break;
      case YEASTPRODIDCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->productID());
         }
         break;
      case YEASTFORMCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->formStringTr());
         }
         if (role == Qt::UserRole) {
            return QVariant(static_cast<int>(row->form()));
         }
         break;
      case YEASTINVENTORYCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(row->inventory());
         }
         break;
      case YEASTAMOUNTCOL:
         if (role == Qt::DisplayRole) {
            return QVariant(
               Measurement::displayAmount(
                  Measurement::Amount{
                     row->amount(),
                     row->amountIsWeight() ? Measurement::Units::kilograms : Measurement::Units::liters
                  },
                  3,
                  this->getForcedSystemOfMeasurementForColumn(column),
                  std::nullopt
               )
            );
         }
         break;
      default :
         qWarning() << Q_FUNC_INFO << "Bad column: " << column;
         break;
   }
   return QVariant();
}

QVariant YeastTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumName(section);
   }
   return QVariant();
}

Qt::ItemFlags YeastTableModel::flags(const QModelIndex & index) const {
   int col = index.column();
   switch (col) {
      case YEASTNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      case YEASTINVENTORYCOL:
         return (Qt::ItemIsEnabled | (this->isInventoryEditable() ? Qt::ItemIsEditable : Qt::NoItemFlags));
      default:
         return Qt::ItemIsSelectable | (editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled |
                Qt::ItemIsEnabled;
   }
}

bool YeastTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (index.row() >= static_cast<int>(this->rows.size()) || role != Qt::EditRole) {
      return false;
   }

   auto row = this->rows[index.row()];

   int const column = index.column();
   switch (column) {
      case YEASTNAMECOL:
         if (!value.canConvert(QVariant::String)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::NamedEntity::name,
                                               value.toString(),
                                               tr("Change Yeast Name"));
         break;
      case YEASTLABCOL:
         if (!value.canConvert(QVariant::String)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::Yeast::laboratory,
                                               value.toString(),
                                               tr("Change Yeast Laboratory"));
         break;
      case YEASTPRODIDCOL:
         if (!value.canConvert(QVariant::String)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::Yeast::productID,
                                               value.toString(),
                                               tr("Change Yeast Product ID"));
         break;
      case YEASTTYPECOL:
         if (!value.canConvert(QVariant::Int)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::Yeast::type,
                                               value.toInt(),
                                               tr("Change Yeast Type"));
         break;
      case YEASTFORMCOL:
         if (!value.canConvert(QVariant::Int)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::Yeast::form,
                                               value.toInt(),
                                               tr("Change Yeast Form"));
         break;
      case YEASTINVENTORYCOL:
         if (!value.canConvert(QVariant::Int)) {
            return false;
         }
         MainWindow::instance().doOrRedoUpdate(*row,
                                               PropertyNames::NamedEntityWithInventory::inventory,
                                               value.toInt(),
                                               tr("Change Yeast Inventory Unit Size"));
         break;
      case YEASTAMOUNTCOL:
         if (!value.canConvert(QVariant::String)) {
            return false;
         }

         MainWindow::instance().doOrRedoUpdate(
            *row,
            PropertyNames::Yeast::amount,
            Measurement::qStringToSI(
               value.toString(),
               row->amountIsWeight() ? Measurement::PhysicalQuantity::Mass : Measurement::PhysicalQuantity::Volume,
               this->getForcedSystemOfMeasurementForColumn(column),
               this->getForcedRelativeScaleForColumn(column)
            ).quantity(),
            tr("Change Yeast Amount")
         );
         break;
      default:
         qWarning() << Q_FUNC_INFO << "Bad column: " << column;
         return false;
   }
   return true;
}

//==========================CLASS YeastItemDelegate===============================

YeastItemDelegate::YeastItemDelegate(QObject * parent)
   : QItemDelegate(parent) {
}

QWidget * YeastItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & /*option*/,
                                          const QModelIndex & index) const {
   int col = index.column();

   if (col == YEASTTYPECOL) {
      QComboBox * box = new QComboBox(parent);

      box->addItem(tr("Ale"));
      box->addItem(tr("Lager"));
      box->addItem(tr("Wheat"));
      box->addItem(tr("Wine"));
      box->addItem(tr("Champagne"));
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   } else if (col == YEASTFORMCOL) {
      QComboBox * box = new QComboBox(parent);

      box->addItem(tr("Liquid"));
      box->addItem(tr("Dry"));
      box->addItem(tr("Slant"));
      box->addItem(tr("Culture"));
      box->setMinimumWidth(box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   } else {
      return new QLineEdit(parent);
   }
}

void YeastItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
   int col = index.column();

   if (col == YEASTTYPECOL || col == YEASTFORMCOL) {
      QComboBox * box = qobject_cast<QComboBox *>(editor);
      int ndx = index.model()->data(index, Qt::UserRole).toInt();

      box->setCurrentIndex(ndx);
   } else {
      QLineEdit * line = qobject_cast<QLineEdit *>(editor);

      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }

}

void YeastItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
   int col = index.column();

   if (col == YEASTTYPECOL || col == YEASTFORMCOL) {
      QComboBox * box = static_cast<QComboBox *>(editor);
      int ndx = box->currentIndex();
      int curr = model->data(index, Qt::UserRole).toInt();

      if (ndx != curr) {
         model->setData(index, ndx, Qt::EditRole);
      }
   } else {
      QLineEdit * line = static_cast<QLineEdit *>(editor);

      if (line->isModified()) {
         model->setData(index, line->text(), Qt::EditRole);
      }
   }
}

void YeastItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option,
                                             const QModelIndex & /*index*/) const {
   editor->setGeometry(option.rect);
}
