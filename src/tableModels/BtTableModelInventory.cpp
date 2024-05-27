////*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
/// * tableModels/BtTableModelInventory.cpp is part of Brewtarget, and is copyright the following authors 2022-2023:
/// *   • Matt Young <mfsy@yahoo.com>
/// *
/// * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
/// * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
/// * version.
/// *
/// * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
/// * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
/// * details.
/// *
/// * You should have received a copy of the GNU General Public License along with this program.  If not, see
/// * <http://www.gnu.org/licenses/>.
/// ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
///#include "tableModels/BtTableModelInventory.h"
///
///BtTableModelInventory::BtTableModelInventory(QTableView * parent,
///                                             bool editable,
///                                             std::initializer_list<BtTableModel::ColumnInfo> columnInfos) :
///   BtTableModelRecipeObserver{parent, editable, columnInfos},
///   inventoryEditable{false} {
///   return;
///}
///
///BtTableModelInventory::~BtTableModelInventory() = default;
///
///void BtTableModelInventory::setInventoryEditable(bool var) {
///   this->inventoryEditable = var;
///   return;
///}
///
///bool BtTableModelInventory::isInventoryEditable() const {
///   return this->inventoryEditable;
///}
///
