/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/WaterTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Tim Payne <swstim@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "tableModels/WaterTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "Localization.h"

WaterTableModel::WaterTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(Water, Name       , tr("Name"             ), PropertyNames::NamedEntity::name     ),
         TABLE_MODEL_HEADER(Water, Calcium    , tr("Calcium (ppm)"    ), PropertyNames::Water::calcium_ppm    ),
         TABLE_MODEL_HEADER(Water, Bicarbonate, tr("Bicarbonate (ppm)"), PropertyNames::Water::bicarbonate_ppm),
         TABLE_MODEL_HEADER(Water, Sulfate    , tr("Sulfate (ppm)"    ), PropertyNames::Water::sulfate_ppm    ),
         TABLE_MODEL_HEADER(Water, Chloride   , tr("Chloride (ppm)"   ), PropertyNames::Water::chloride_ppm   ),
         TABLE_MODEL_HEADER(Water, Sodium     , tr("Sodium (ppm)"     ), PropertyNames::Water::sodium_ppm     ),
         TABLE_MODEL_HEADER(Water, Magnesium  , tr("Magnesium (ppm)"  ), PropertyNames::Water::magnesium_ppm  ),
      }
   },
   TableModelBase<WaterTableModel, Water>{} {
   return;
}

WaterTableModel::~WaterTableModel() = default;


void WaterTableModel::added  ([[maybe_unused]] std::shared_ptr<Water> item) { return; }
void WaterTableModel::removed([[maybe_unused]] std::shared_ptr<Water> item) { return; }
void WaterTableModel::updateTotals()                                        { return; }


QVariant WaterTableModel::data(const QModelIndex & index, int role) const {
   if (!this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   auto row = this->rows[index.row()];

   auto const columnIndex = static_cast<WaterTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case WaterTableModel::ColumnIndex::Name:
         return QVariant(row->name());
      case WaterTableModel::ColumnIndex::Calcium:
         return QVariant(Measurement::displayQuantity(row->calcium_ppm(), 3));
      case WaterTableModel::ColumnIndex::Bicarbonate:
         return QVariant(Measurement::displayQuantity(row->bicarbonate_ppm(), 3));
      case WaterTableModel::ColumnIndex::Sulfate:
         return QVariant(Measurement::displayQuantity(row->sulfate_ppm(), 3));
      case WaterTableModel::ColumnIndex::Chloride:
         return QVariant(Measurement::displayQuantity(row->chloride_ppm(), 3));
      case WaterTableModel::ColumnIndex::Sodium:
         return QVariant(Measurement::displayQuantity(row->sodium_ppm(), 3));
      case WaterTableModel::ColumnIndex::Magnesium:
         return QVariant(Measurement::displayQuantity(row->magnesium_ppm(), 3));
      default :
         qWarning() << Q_FUNC_INFO << tr("Bad column: %1").arg(index.column());
         return QVariant();
   }
}

Qt::ItemFlags WaterTableModel::flags(const QModelIndex & index) const {
   auto const columnIndex = static_cast<WaterTableModel::ColumnIndex>(index.column());
   if (columnIndex == WaterTableModel::ColumnIndex::Name) {
      return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool WaterTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->indexAndRoleOk(index, role)) {
      return false;
   }

   bool retval = value.canConvert<QString>();
   if (!retval) {
      return retval;
   }

   auto row = this->rows[index.row()];

   auto const columnIndex = static_cast<WaterTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case WaterTableModel::ColumnIndex::Name:
         row->setName(value.toString());
         break;
///      case WaterTableModel::ColumnIndex::Amount:
///         row->setAmount(Measurement::qStringToSI(value.toString(),
///                                                 Measurement::PhysicalQuantity::Volume,
///                                                 this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///                                                 this->getColumnInfo(columnIndex).getForcedRelativeScale()).quantity);
///         break;
      case WaterTableModel::ColumnIndex::Calcium:
         row->setCalcium_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WaterTableModel::ColumnIndex::Bicarbonate:
         row->setBicarbonate_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WaterTableModel::ColumnIndex::Sulfate:
         row->setSulfate_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WaterTableModel::ColumnIndex::Chloride:
         row->setChloride_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WaterTableModel::ColumnIndex::Sodium:
         row->setSodium_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      case WaterTableModel::ColumnIndex::Magnesium:
         row->setMagnesium_ppm(Localization::toDouble(value.toString(), Q_FUNC_INFO));
         break;
      default:
         retval = false;
         qWarning() << Q_FUNC_INFO << "Bad column: " << index.column();
         break;
   }

   return retval;
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Water, water, PropertyNames::None::none)

//==========================CLASS WaterItemDelegate===============================
// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Water)
