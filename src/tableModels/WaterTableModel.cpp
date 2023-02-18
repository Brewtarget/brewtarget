/*
 * WaterTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Matt Young <mfsy@yahoo.com>
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
#include "tableModels/WaterTableModel.h"

#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QLineEdit>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "WaterTableWidget.h"

WaterTableModel::WaterTableModel(WaterTableWidget * parent) :
   BtTableModelRecipeObserver{
      parent,
      false,
      {  {WATERNAMECOL,        {tr("Name"),              NonPhysicalQuantity::String,           ""      }},
         {WATERAMOUNTCOL,      {tr("Amount"),            Measurement::PhysicalQuantity::Volume, "amount"}},
         {WATERCALCIUMCOL,     {tr("Calcium (ppm)"),     NonPhysicalQuantity::Count,            ""      }},
         {WATERBICARBONATECOL, {tr("Bicarbonate (ppm)"), NonPhysicalQuantity::Count,            ""      }},
         {WATERSULFATECOL,     {tr("Sulfate (ppm)"),     NonPhysicalQuantity::Count,            ""      }},
         {WATERCHLORIDECOL,    {tr("Chloride (ppm)"),    NonPhysicalQuantity::Count,            ""      }},
         {WATERSODIUMCOL,      {tr("Sodium (ppm)"),      NonPhysicalQuantity::Count,            ""      }},
         {WATERMAGNESIUMCOL,   {tr("Magnesium (ppm)"),   NonPhysicalQuantity::Count,            ""      }}}
   },
   BtTableModelData<Water>{} {
   return;
}

WaterTableModel::~WaterTableModel() = default;

void WaterTableModel::observeRecipe(Recipe * rec) {
   if (recObs) {
      disconnect(recObs, nullptr, this, nullptr);
      removeAll();
   }

   recObs = rec;
   if (recObs) {
      connect(recObs, &NamedEntity::changed, this, &WaterTableModel::changed);
      addWaters(recObs->getAll<Water>());
   }
}

void WaterTableModel::observeDatabase(bool val) {
   if (val) {
      observeRecipe(nullptr);
      removeAll();
      connect(&ObjectStoreTyped<Water>::getInstance(),
              &ObjectStoreTyped<Water>::signalObjectInserted,
              this,
              &WaterTableModel::addWater);
      connect(&ObjectStoreTyped<Water>::getInstance(),
              &ObjectStoreTyped<Water>::signalObjectDeleted,
              this,
              &WaterTableModel::removeWater);
      this->addWaters(ObjectStoreWrapper::getAll<Water>());
   } else {
      removeAll();
      disconnect(&ObjectStoreTyped<Water>::getInstance(), nullptr, this, nullptr);
   }
   return;
}

void WaterTableModel::addWater(int waterId) {
   auto water = ObjectStoreWrapper::getById<Water>(waterId);
   if (this->rows.contains(water)) {
      return;
   }

   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if (!this->recObs && (water->deleted() || !water->display())) {
      return;
   }

   // If we are watching a Recipe and the new Water does not belong to it then there is nothing for us to do
   if (this->recObs) {
      Recipe * recipeOfNewWater = water->getOwningRecipe();
      if (recipeOfNewWater && this->recObs->key() != recipeOfNewWater->key()) {
         qDebug() <<
            Q_FUNC_INFO << "Ignoring signal about new Water #" << water->key() << "as it belongs to Recipe #" <<
            recipeOfNewWater->key() << "and we are watching Recipe #" << this->recObs->key();
         return;
      }
   }

   beginInsertRows(QModelIndex(), rows.size(), rows.size());
   rows.append(water);
   connect(water.get(), &NamedEntity::changed, this, &WaterTableModel::changed);
   endInsertRows();

   if (parentTableWidget) {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void WaterTableModel::addWaters(QList<std::shared_ptr<Water> > waters) {
   auto tmp = this->removeDuplicates(waters);

   int size = rows.size();
   if (size + tmp.size()) {
      beginInsertRows(QModelIndex(), size, size + tmp.size() - 1);
      rows.append(tmp);

      for (auto water : tmp) {
         connect(water.get(), &NamedEntity::changed, this, &WaterTableModel::changed);
      }

      endInsertRows();
   }

   if (parentTableWidget) {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }

}

void WaterTableModel::removeWater([[maybe_unused]] int waterId,
                                  std::shared_ptr<QObject> object) {
   auto water = std::static_pointer_cast<Water>(object);
   int i = rows.indexOf(water);
   if (i >= 0) {
      beginRemoveRows(QModelIndex(), i, i);
      disconnect(water.get(), nullptr, this, nullptr);
      rows.removeAt(i);
      endRemoveRows();

      if (parentTableWidget) {
         parentTableWidget->resizeColumnsToContents();
         parentTableWidget->resizeRowsToContents();
      }
   }
}

void WaterTableModel::removeAll() {
   beginRemoveRows(QModelIndex(), 0, rows.size() - 1);
   while (!rows.isEmpty()) {
      disconnect(rows.takeLast().get(), nullptr, this, nullptr);
   }
   endRemoveRows();
}

void WaterTableModel::changed(QMetaProperty prop, QVariant val) {
   Q_UNUSED(prop)
   Q_UNUSED(val)
   // Find the notifier in the list
   Water * waterSender = qobject_cast<Water *>(sender());
   if (waterSender) {
      int ii = findIndexOf(waterSender);
      if (ii >= 0) {
         emit dataChanged(QAbstractItemModel::createIndex(ii, 0),
                          QAbstractItemModel::createIndex(ii, WATERNUMCOLS - 1));
      }
   }
   return;
}

int WaterTableModel::rowCount(const QModelIndex & /*parent*/) const {
   return this->rows.size();
}

QVariant WaterTableModel::data(const QModelIndex & index, int role) const {
   // Ensure the row is ok.
   if (index.row() >= rows.size()) {
      qWarning() << tr("Bad model index. row = %1").arg(index.row());
      return QVariant();
   }

   auto row = this->rows[index.row()];

   // Make sure we only respond to the DisplayRole role.
   if (role != Qt::DisplayRole) {
      return QVariant();
   }

   switch (index.column()) {
      case WATERNAMECOL:
         return QVariant(row->name());
      case WATERAMOUNTCOL:
         return QVariant(Measurement::displayAmount(Measurement::Amount{row->amount(), Measurement::Units::liters}));
      case WATERCALCIUMCOL:
         return QVariant(Measurement::displayQuantity(row->calcium_ppm(), 3));
      case WATERBICARBONATECOL:
         return QVariant(Measurement::displayQuantity(row->bicarbonate_ppm(), 3));
      case WATERSULFATECOL:
         return QVariant(Measurement::displayQuantity(row->sulfate_ppm(), 3));
      case WATERCHLORIDECOL:
         return QVariant(Measurement::displayQuantity(row->chloride_ppm(), 3));
      case WATERSODIUMCOL:
         return QVariant(Measurement::displayQuantity(row->sodium_ppm(), 3));
      case WATERMAGNESIUMCOL:
         return QVariant(Measurement::displayQuantity(row->magnesium_ppm(), 3));
      default :
         qWarning() << tr("Bad column: %1").arg(index.column());
         return QVariant();
   }
}

QVariant WaterTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumName(section);
   }
   return QVariant();
}

Qt::ItemFlags WaterTableModel::flags(const QModelIndex & index) const {
   int col = index.column();
   switch (col) {
      case WATERNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      default:
         return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
                Qt::ItemIsEnabled;
   }
}

bool WaterTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (index.row() >= rows.size() || role != Qt::EditRole) {
      return false;
   }

   bool retval = value.canConvert(QVariant::String);
   if (!retval) {
      return retval;
   }

   auto row = this->rows[index.row()];

   int const column = index.column();
   switch (column) {
      case WATERNAMECOL:
         row->setName(value.toString());
         break;
      case WATERAMOUNTCOL:
         row->setAmount(Measurement::qStringToSI(value.toString(),
                                                 Measurement::PhysicalQuantity::Volume,
                                                 this->getForcedSystemOfMeasurementForColumn(column),
                                                 this->getForcedRelativeScaleForColumn(column)).quantity());
         break;
      case WATERCALCIUMCOL:
         row->setCalcium_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WATERBICARBONATECOL:
         row->setBicarbonate_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WATERSULFATECOL:
         row->setSulfate_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WATERCHLORIDECOL:
         row->setChloride_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WATERSODIUMCOL:
         row->setSodium_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WATERMAGNESIUMCOL:
         row->setMagnesium_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      default:
         retval = false;
         qWarning() << Q_FUNC_INFO << "Bad column: " << column;
         break;
   }

   return retval;
}

//==========================CLASS HopItemDelegate===============================

WaterItemDelegate::WaterItemDelegate(QObject * parent)
   : QItemDelegate(parent) {
}

QWidget * WaterItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & /*option*/,
                                          const QModelIndex & /*index*/) const {
   return new QLineEdit(parent);
}

void WaterItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
   QLineEdit * line = qobject_cast<QLineEdit *>(editor);
   line->setText(index.model()->data(index, Qt::DisplayRole).toString());
}

void WaterItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
   QLineEdit * line = qobject_cast<QLineEdit *>(editor);

   if (line->isModified()) {
      model->setData(index, line->text(), Qt::EditRole);
   }
}

void WaterItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option,
                                             const QModelIndex & /*index*/) const {
   editor->setGeometry(option.rect);
}
