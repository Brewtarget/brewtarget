/*
 * tableModels/BtTableModel.cpp is part of Brewtarget, and is copyright the following
 * authors 2021-2023
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
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
#include "tableModels/BtTableModel.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>

#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "utils/OptionalHelpers.h"
#include "widgets/SmartAmounts.h"
#include "widgets/UnitAndScalePopUpMenu.h"

BtTableModelRecipeObserver::BtTableModelRecipeObserver(QTableView * parent,
                                                       bool editable,
                                                       std::initializer_list<ColumnInfo> columnInfos) :
   BtTableModel{parent, editable, columnInfos},
   recObs{nullptr} {
   return;
}

BtTableModelRecipeObserver::~BtTableModelRecipeObserver() = default;

//======================================================================================================================

void BtTableModel::ColumnInfo::setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement) const {
   SmartAmounts::setForcedSystemOfMeasurement(this->tableModelName, this->columnName, forcedSystemOfMeasurement);
   return;
}

void BtTableModel::ColumnInfo::setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) const {
   SmartAmounts::setForcedRelativeScale(this->tableModelName, this->columnName, forcedScale);
   return;
}

std::optional<Measurement::SystemOfMeasurement> BtTableModel::ColumnInfo::getForcedSystemOfMeasurement() const {
   return SmartAmounts::getForcedSystemOfMeasurement(this->tableModelName, this->columnName);
}

std::optional<Measurement::UnitSystem::RelativeScale> BtTableModel::ColumnInfo::getForcedRelativeScale() const {
   return SmartAmounts::getForcedRelativeScale(this->tableModelName, this->columnName);
}

//======================================================================================================================

BtTableModel::BtTableModel(QTableView * parent,
                           bool editable,
                           std::initializer_list<BtTableModel::ColumnInfo> columnInfos) :
   QAbstractTableModel{parent},
   parentTableWidget{parent},
   editable{editable},
   m_columnInfos{columnInfos} {
   return;
}

BtTableModel::~BtTableModel() = default;

BtTableModel::ColumnInfo const & BtTableModel::getColumnInfo(size_t const columnIndex) const {
   // It's a coding error to call this for a non-existent column
   Q_ASSERT(columnIndex < this->m_columnInfos.size());

   BtTableModel::ColumnInfo const & columnInfo = this->m_columnInfos[columnIndex];

   // It's a coding error if the info for column N isn't at position N in the vector (in both cases counting from 0)
   Q_ASSERT(columnInfo.index == columnIndex);

   return columnInfo;
}

QVariant BtTableModel::getColumnLabel(size_t const columnIndex) const {
   return this->getColumnInfo(columnIndex).label;
}

int BtTableModel::columnCount(QModelIndex const & /*parent*/) const {
   return this->m_columnInfos.size();
}

void BtTableModel::doContextMenu(QPoint const & point, QHeaderView * hView, QMenu * menu, int selected) {
   QAction* invoked = menu->exec(hView->mapToGlobal(point));
   if (invoked == nullptr) {
      return;
   }

   // User will either have selected a SystemOfMeasurement or a UnitSystem::RelativeScale.  We can know which based on
   // whether it's the menu or the sub-menu that it came from.
   bool isTopMenu{invoked->parentWidget() == menu};
   if (isTopMenu) {
      // It's the menu, so SystemOfMeasurement
      std::optional<Measurement::SystemOfMeasurement> const whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::SystemOfMeasurement>(*invoked);
      qDebug() << Q_FUNC_INFO << "Column" << selected << ", selected SystemOfMeasurement" << whatSelected;
      this->getColumnInfo(selected).setForcedSystemOfMeasurement(whatSelected);
      // Choosing a forced SystemOfMeasurement resets any selection of forced RelativeScale, but this is handled by
      // unsetForcedSystemOfMeasurementForColumn() and setForcedSystemOfMeasurementForColumn()
   } else {
      // It's the sub-menu, so UnitSystem::RelativeScale
      std::optional<Measurement::UnitSystem::RelativeScale> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::UnitSystem::RelativeScale>(*invoked);
      qDebug() << Q_FUNC_INFO << "Column" << selected << ", selected RelativeScale" << whatSelected;
      this->getColumnInfo(selected).setForcedRelativeScale(whatSelected);
   }
   return;
}

// oofrab
void BtTableModel::contextMenu(QPoint const & point) {
   qDebug() << Q_FUNC_INFO;
   QHeaderView* hView = qobject_cast<QHeaderView*>(this->sender());
   int selected = hView->logicalIndexAt(point);
   // Only makes sense to offer the pop-up "select scale" menu for physical quantities
   BtFieldType const fieldType = this->getColumnInfo(selected).fieldType;
   if (std::holds_alternative<Measurement::PhysicalQuantity>(fieldType)) {
      QMenu * menu = UnitAndScalePopUpMenu::create(parentTableWidget,
                                                   std::get<Measurement::PhysicalQuantity>(fieldType),
                                                   this->getColumnInfo(selected).getForcedSystemOfMeasurement(),
                                                   this->getColumnInfo(selected).getForcedRelativeScale());
      this->doContextMenu(point, hView, menu, selected);
   }
   return;
}
