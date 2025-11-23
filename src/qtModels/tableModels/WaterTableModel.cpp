/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/WaterTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#include "qtModels/tableModels/WaterTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "Localization.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_WaterTableModel.cpp"
#endif

COLUMN_INFOS(
   WaterTableModel,
   TABLE_MODEL_HEADER(Water, Name            , PropertyNames::NamedEntity::name            ), // "Name"
   TABLE_MODEL_HEADER(Water, Calcium         , PropertyNames::Water::calcium_ppm           ), // "Calcium (ppm)"
   TABLE_MODEL_HEADER(Water, Bicarbonate     , PropertyNames::Water::bicarbonate_ppm       ), // "Bicarbonate (ppm)"
   TABLE_MODEL_HEADER(Water, Sulfate         , PropertyNames::Water::sulfate_ppm           ), // "Sulfate (ppm)"
   TABLE_MODEL_HEADER(Water, Chloride        , PropertyNames::Water::chloride_ppm          ), // "Chloride (ppm)"
   TABLE_MODEL_HEADER(Water, Sodium          , PropertyNames::Water::sodium_ppm            ), // "Sodium (ppm)"
   TABLE_MODEL_HEADER(Water, Magnesium       , PropertyNames::Water::magnesium_ppm         ), // "Magnesium (ppm)"
   TABLE_MODEL_HEADER(Water, NumRecipesUsedIn, PropertyNames::NamedEntity::numRecipesUsedIn), // "N° Recipes"
)

WaterTableModel::WaterTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<WaterTableModel, Water>{} {
   return;
}

WaterTableModel::~WaterTableModel() = default;


void WaterTableModel::added  ([[maybe_unused]] std::shared_ptr<Water> item) { return; }
void WaterTableModel::removed([[maybe_unused]] std::shared_ptr<Water> item) { return; }
void WaterTableModel::updateTotals()                                        { return; }


QVariant WaterTableModel::data(const QModelIndex & index, int role) const {
   return this->doDataDefault(index, role);
}

bool WaterTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->indexAndRoleOk(index, role)) {
      return false;
   }

   bool retval = value.canConvert<QString>();
   if (!retval) {
      return retval;
   }

   auto row = this->m_rows[index.row()];

   auto const columnIndex = static_cast<WaterTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case WaterTableModel::ColumnIndex::Name:
         row->setName(value.toString());
         break;
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
