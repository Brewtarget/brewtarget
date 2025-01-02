/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/EquipmentTableModel.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "qtModels/tableModels/EquipmentTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_EquipmentTableModel.cpp"

EquipmentTableModel::EquipmentTableModel(QTableView* parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(Equipment, Name           , tr("Name"            ), PropertyNames::NamedEntity::name              ),
         TABLE_MODEL_HEADER(Equipment, MashTunVolume  , tr("Mash Tun Volume" ), PropertyNames::Equipment::mashTunVolume_l     ),
         TABLE_MODEL_HEADER(Equipment, KettleVolume   , tr("Kettle Volume"   ), PropertyNames::Equipment::kettleBoilSize_l    ),
         TABLE_MODEL_HEADER(Equipment, FermenterVolume, tr("Fermenter Volume"), PropertyNames::Equipment::fermenterBatchSize_l),
      }
   },
   TableModelBase<EquipmentTableModel, Equipment>{} {
   this->rows.clear();

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &EquipmentTableModel::contextMenu);
   return;
}

EquipmentTableModel::~EquipmentTableModel() = default;

void EquipmentTableModel::added  ([[maybe_unused]] std::shared_ptr<Equipment> item) { return; }
void EquipmentTableModel::removed([[maybe_unused]] std::shared_ptr<Equipment> item) { return; }
void EquipmentTableModel::updateTotals()                                            { return; }

QVariant EquipmentTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

Qt::ItemFlags EquipmentTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<EquipmentTableModel>(index, this->m_editable);
}

bool EquipmentTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Equipment, equipment, PropertyNames::Recipe::equipmentId)
//=============================================== CLASS EquipmentItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Equipment)
