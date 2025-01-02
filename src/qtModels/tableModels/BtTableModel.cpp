/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/BtTableModel.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#include "qtModels/tableModels/BtTableModel.h"

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

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_BtTableModel.cpp"

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
   m_parentTableWidget{parent},
   m_editable{editable},
   m_columnInfos{columnInfos} {

   QHeaderView * rowHeaderView = this->m_parentTableWidget->verticalHeader();
   rowHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
   QHeaderView * columnHeaderView = this->m_parentTableWidget->horizontalHeader();
   columnHeaderView->setContextMenuPolicy(Qt::CustomContextMenu);
   this->m_parentTableWidget->setWordWrap(false);
   // We use QHeaderView::Interactive here because it's the only option that allows the user to resize the columns
   // (In theory, QHeaderView::ResizeToContents automatically sets a fixed size that's right for all the data, but, in
   // practice it doesn't always do what you want, so it's better to give the user some control).
   columnHeaderView->setSectionResizeMode(QHeaderView::Interactive);
//   columnHeaderView->setMinimumSectionSize(parent->width()/this->columnCount()); SaltTableModel

   return;
}

BtTableModel::~BtTableModel() = default;

BtTableModel::ColumnInfo const & BtTableModel::getColumnInfo(size_t const columnIndex) const {
   // Uncomment this block if the assert below is firing
   if (columnIndex >= this->m_columnInfos.size()) {
      qCritical().noquote() <<
         Q_FUNC_INFO << "columnIndex:" << columnIndex << ", this->m_columnInfos.size():" <<
         this->m_columnInfos.size() << Logging::getStackTrace();
      // TODO : This is temporary until we fix the bug!
      return this->m_columnInfos[0];
   }
   // It's a coding error to call this for a non-existent column
   Q_ASSERT(columnIndex < this->m_columnInfos.size());

   BtTableModel::ColumnInfo const & columnInfo = this->m_columnInfos[columnIndex];

   // Normally the following log statement should be left commented, as it generates a _lot_ of logging.  Uncomment it
   // temporarily if the assert below is firing.
//   qDebug().noquote() <<
//      Q_FUNC_INFO << "columnInfo.index:" << columnInfo.index << ", columnIndex:" << columnIndex <<
//      Logging::getStackTrace();

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

QVariant BtTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumnLabel(section);
   }

   return QVariant();
}

void BtTableModel::contextMenu(QPoint const & point) {
   qDebug() << Q_FUNC_INFO;
   QHeaderView* hView = qobject_cast<QHeaderView*>(this->sender());
   int selected = hView->logicalIndexAt(point);
   BtTableModel::ColumnInfo const & columnInfo = this->getColumnInfo(selected);

   // .:TBD:. The logic from here on is similar to that in SmartLabel::popContextMenu, but I didn't yet figure out how
   // to have more of the code be shared.

   // Only makes sense to offer the pop-up "select scale" menu for physical quantities
   BtFieldType const fieldType = *columnInfo.typeInfo.fieldType;
   if (std::holds_alternative<NonPhysicalQuantity>(fieldType)) {
      return;
   }

   //
   // Note that UnitAndScalePopUpMenu::create handles the case where there are two possible physical quantities
   //
   std::unique_ptr<QMenu> menu =
      UnitAndScalePopUpMenu::create(this->m_parentTableWidget,
                                    ConvertToPhysicalQuantities(fieldType),
                                    columnInfo.getForcedSystemOfMeasurement(),
                                    columnInfo.getForcedRelativeScale());

   // If the pop-up menu has no entries, then we can bail out here
   if (menu->actions().size() == 0) {
      qDebug() << Q_FUNC_INFO << "Nothing to show for" << fieldType;
   }

   //
   // This shows the context menu and returns when the user either selects something or dismisses the menu
   //
   QAction * invoked = menu->exec(hView->mapToGlobal(point));
   if (invoked == nullptr) {
      return;
   }

   // User will either have selected a SystemOfMeasurement or a UnitSystem::RelativeScale.  We can know which based
   // on whether it's the menu or the sub-menu that it came from.
   bool isTopMenu{invoked->parent() == menu.get()};
   if (isTopMenu) {
      // It's the menu, so SystemOfMeasurement
      std::optional<Measurement::SystemOfMeasurement> const whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::SystemOfMeasurement>(*invoked);
      qDebug() << Q_FUNC_INFO << "Column" << selected << ", selected SystemOfMeasurement" << whatSelected;
      columnInfo.setForcedSystemOfMeasurement(whatSelected);
      // Choosing a forced SystemOfMeasurement resets any selection of forced RelativeScale, but this is handled by
      // unsetForcedSystemOfMeasurementForColumn() and setForcedSystemOfMeasurementForColumn()
   } else {
      // It's the sub-menu, so UnitSystem::RelativeScale
      std::optional<Measurement::UnitSystem::RelativeScale> whatSelected =
         UnitAndScalePopUpMenu::dataFromQAction<Measurement::UnitSystem::RelativeScale>(*invoked);
      qDebug() << Q_FUNC_INFO << "Column" << selected << ", selected RelativeScale" << whatSelected;
      columnInfo.setForcedRelativeScale(whatSelected);
   }

   return;
}
