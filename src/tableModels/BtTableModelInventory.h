/*
 * tableModels/BtTableModelInventory.h is part of Brewtarget, and is copyright the following
 * authors 2021-2023:
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef TABLEMODELS_BTTABLEMODELINVENTORY_H
#define TABLEMODELS_BTTABLEMODELINVENTORY_H
#pragma once

#include "tableModels/BtTableModel.h"

/**
 * \class BtTableModelInventory
 *
 * \brief Extends \c BtTableModel to add support for inventory
 */
class BtTableModelInventory : public BtTableModelRecipeObserver {
public:
   BtTableModelInventory(QTableView * parent,
                         bool editable,
                         std::initializer_list<BtTableModel::ColumnInfo> columnInfos);
   ~BtTableModelInventory();

   /*!
    * \brief True if the inventory column should be editable, false otherwise.
    *
    * The default is that the inventory column is not editable
    */
   void setInventoryEditable(bool var);
   bool isInventoryEditable() const;

private:
   bool inventoryEditable;
};

#endif
