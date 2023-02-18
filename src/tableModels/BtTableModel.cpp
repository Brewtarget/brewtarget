/*
 * tableModels/BtTableModel.cpp is part of Brewtarget, and is copyright the following
 * authors 2021-2022:
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
#include "widgets/UnitAndScalePopUpMenu.h"

BtTableModelRecipeObserver::BtTableModelRecipeObserver(QTableView * parent,
                                                       bool editable,
                                                       std::initializer_list<std::pair<int const, ColumnInfo> > columnIdToInfo) :
   BtTableModel{parent, editable, columnIdToInfo},
   recObs{nullptr} {
   return;
}

BtTableModelRecipeObserver::~BtTableModelRecipeObserver() = default;

//======================================================================================================================
BtTableModel::BtTableModel(QTableView * parent,
                           bool editable,
                           std::initializer_list<std::pair<int const, BtTableModel::ColumnInfo> > columnIdToInfo) :
   QAbstractTableModel{parent},
   parentTableWidget{parent},
   editable{editable},
   columnIdToInfo{columnIdToInfo} {
   return;
}

BtTableModel::~BtTableModel() = default;

std::optional<Measurement::SystemOfMeasurement> BtTableModel::getForcedSystemOfMeasurementForColumn(int column) const {
   QString attribute = this->columnGetAttribute(column);
   return attribute.isEmpty() ? std::nullopt : Measurement::getForcedSystemOfMeasurementForField(attribute,
                                                                                                 this->objectName());
}

std::optional<Measurement::UnitSystem::RelativeScale> BtTableModel::getForcedRelativeScaleForColumn(int column) const {
   QString attribute = this->columnGetAttribute(column);
   return attribute.isEmpty() ? std::nullopt : Measurement::getForcedRelativeScaleForField(attribute,
                                                                                           this->objectName());
}

void BtTableModel::setForcedSystemOfMeasurementForColumn(int column,
                                                         std::optional<Measurement::SystemOfMeasurement> systemOfMeasurement) {
   QString attribute = this->columnGetAttribute(column);
   if (!attribute.isEmpty()) {
      Measurement::setForcedSystemOfMeasurementForField(attribute, this->objectName(), systemOfMeasurement);
      // As we're setting/changing the forced SystemOfMeasurement, we want to clear the forced RelativeScale
      Measurement::setForcedRelativeScaleForField(attribute, this->objectName(), std::nullopt);
   }
   return;
}

void BtTableModel::setForcedRelativeScaleForColumn(int column,
                                                   std::optional<Measurement::UnitSystem::RelativeScale> relativeScale) {
   QString attribute = this->columnGetAttribute(column);
   if (!attribute.isEmpty()) {
      Measurement::setForcedRelativeScaleForField(attribute, this->objectName(), relativeScale);
   }
   return;
}

QVariant BtTableModel::getColumName(int column) const {
   if (this->columnIdToInfo.contains(column)) {
      return QVariant(this->columnIdToInfo.value(column).headerName);
   }

   qWarning() << Q_FUNC_INFO << "Bad column:" << column;
   return QVariant();
}

int BtTableModel::columnCount(QModelIndex const & /*parent*/) const {
   return this->columnIdToInfo.size();
}

QString BtTableModel::columnGetAttribute(int column) const {
   if (this->columnIdToInfo.contains(column)) {
      return this->columnIdToInfo.value(column).attribute;
   }
   return "";
}

BtFieldType BtTableModel::columnGetFieldType(int column) const {
   if (this->columnIdToInfo.contains(column)) {
      return this->columnIdToInfo.value(column).fieldType;
   }
   return NonPhysicalQuantity::String;
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
      std::optional<Measurement::SystemOfMeasurement> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::SystemOfMeasurement>(*invoked);
      qDebug() << Q_FUNC_INFO << "Column" << selected << ", selected SystemOfMeasurement" << whatSelected;
      this->setForcedSystemOfMeasurementForColumn(selected, whatSelected);
      // Choosing a forced SystemOfMeasurement resets any selection of forced RelativeScale, but this is handled by
      // unsetForcedSystemOfMeasurementForColumn() and setForcedSystemOfMeasurementForColumn()
   } else {
      // It's the sub-menu, so UnitSystem::RelativeScale
      std::optional<Measurement::UnitSystem::RelativeScale> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::UnitSystem::RelativeScale>(*invoked);
      qDebug() << Q_FUNC_INFO << "Column" << selected << ", selected RelativeScale" << whatSelected;
      this->setForcedRelativeScaleForColumn(selected, whatSelected);
   }
   return;
}

// oofrab
void BtTableModel::contextMenu(QPoint const & point) {
   qDebug() << Q_FUNC_INFO;
   QHeaderView* hView = qobject_cast<QHeaderView*>(this->sender());
   int selected = hView->logicalIndexAt(point);
   // Only makes sense to offer the pop-up "select scale" menu for physical quantities
   BtFieldType fieldType = this->columnGetFieldType(selected);
   if (std::holds_alternative<Measurement::PhysicalQuantity>(fieldType)) {
      QMenu * menu = UnitAndScalePopUpMenu::create(parentTableWidget,
                                                   std::get<Measurement::PhysicalQuantity>(fieldType),
                                                   this->getForcedSystemOfMeasurementForColumn(selected),
                                                   this->getForcedRelativeScaleForColumn(selected));
      this->doContextMenu(point, hView, menu, selected);
   }
   return;
}
