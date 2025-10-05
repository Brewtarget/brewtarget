/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/MashTableModel.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
 *   • Matt Young <mfsy@yahoo.com>
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "qtModels/tableModels/MashTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QTableView>
#include <QVariant>
#include <QWidget>

#include "model/Mash.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MashTableModel.cpp"
#endif

COLUMN_INFOS(
   MashTableModel,
   TABLE_MODEL_HEADER(Mash, Name             , tr("Name"                     ), PropertyNames:: NamedEntity::name            ),
   TABLE_MODEL_HEADER(Mash, InitialGrainTemp , tr("Initial Grain Temperature"), PropertyNames::        Mash::grainTemp_c     ),
   TABLE_MODEL_HEADER(Mash, TotalMashWater   , tr("Total Mash Water"         ), PropertyNames::        Mash::totalMashWater_l),
   TABLE_MODEL_HEADER(Mash, TotalTime        , tr("Total Time"               ), PropertyNames::        Mash::totalTime_mins  ),
   TABLE_MODEL_HEADER(Mash, NumRecipesUsedIn , tr("N° Recipes"               ), PropertyNames::NamedEntity::numRecipesUsedIn ),
)

MashTableModel::MashTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<MashTableModel, Mash>{} {

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &MashTableModel::contextMenu);
   return;
}

MashTableModel::~MashTableModel() = default;

void MashTableModel::added  ([[maybe_unused]] std::shared_ptr<Mash> item) { return; }
void MashTableModel::removed([[maybe_unused]] std::shared_ptr<Mash> item) { return; }
void MashTableModel::updateTotals()                                       { return; }

QVariant MashTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

Qt::ItemFlags MashTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<MashTableModel>(index, this->m_editable);
}

bool MashTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Mash, mash, PropertyNames::Recipe::mashId)

//=============================================== CLASS MashItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Mash)
