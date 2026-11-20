/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/BtTableModel.cpp is part of Brewtarget, and is copyright the following authors 2021-2026:
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
#include "measurement/QuantityFieldType.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "PersistentSettings.h"
#include "utils/OptionalHelpers.h"
#include "widgets/SmartAmounts.h"
#include "widgets/UnitAndScalePopUpMenu.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BtTableModel.cpp"
#endif

BtTableModelRecipeObserver::BtTableModelRecipeObserver(QTableView * parent,
                                                       bool editable) :
   BtTableModel{parent, editable/*, columnInfos*/},
   recObs{nullptr} {
   return;
}

BtTableModelRecipeObserver::~BtTableModelRecipeObserver() = default;

BtTableModel::BtTableModel(QTableView * parent,
                           bool editable) :
   QAbstractTableModel{parent},
   m_parentTableWidget{parent},
   m_editable{editable} {

   QHeaderView * rowHeaderView = this->m_parentTableWidget->verticalHeader();
   rowHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
   QHeaderView * columnHeaderView = this->m_parentTableWidget->horizontalHeader();
   columnHeaderView->setContextMenuPolicy(Qt::CustomContextMenu);
   this->m_parentTableWidget->setWordWrap(false);
   // We use QHeaderView::Interactive here because it's the only option that allows the user to resize the columns
   // (In theory, QHeaderView::ResizeToContents automatically sets a fixed size that's right for all the data, but, in
   // practice it doesn't always do what you want, so it's better to give the user some control).
   columnHeaderView->setSectionResizeMode(QHeaderView::Interactive);
   // Have the last column take up any spare width, so we don't leave an empty strip at the right of the table
   columnHeaderView->setStretchLastSection(true);
   // Allow the user to rearrange the columns by dragging their headers.  (The order is saved in saveUiState.)
   columnHeaderView->setSectionsMovable(true);

   return;
}

BtTableModel::~BtTableModel() = default;

QVariant BtTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   //
   // For horizontal headers, the section number corresponds to the column number.  Similarly, for vertical headers, the
   // section number corresponds to the row number.
   //
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      // For reasons I didn't get to the bottom of, we sometimes get calls from the Qt framework requesting header data
      // for a column number that's one beyond the maximum, so we have to validate that here.
      if (section < this->columnCount()) {
         return this->columnLabel(section);
      }

      qWarning() <<
         Q_FUNC_INFO << "Request for invalid column number " << section << ", this->columnCount():" <<
         this->columnCount();
//      qWarning().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      return QVariant();
   }

   return QVariant();
}

namespace {
   //! Name of the setting where we store the number of columns alongside the header state
   QString numColumnsSettingName(BtStringConst const & property) {
      return QString{*property} + QStringLiteral("_numColumns");
   }
}

void BtTableModel::saveUiState(BtStringConst const & property, BtStringConst const & section) const {
   PersistentSettings::saveUiState(property, *this->m_parentTableWidget->horizontalHeader(), section);
   PersistentSettings::insert(numColumnsSettingName(property), this->columnCount(), *section);
   return;
}

void BtTableModel::restoreUiState(BtStringConst const & property, BtStringConst const & section) {
   QHeaderView & columnHeaderView = *this->m_parentTableWidget->horizontalHeader();
   bool const restored = PersistentSettings::restoreUiState(property, columnHeaderView, section);
   // Saved header state includes the "stretch last section" and "sections movable" settings, and older versions of the
   // software did not set them, so we need to (re)set them after restoring.
   columnHeaderView.setStretchLastSection(true);
   columnHeaderView.setSectionsMovable(true);

   //
   // Columns that are hidden by default need hiding if the saved state does not know about them -- ie if there is no
   // saved state, or it was saved before the column was added.  (Older versions of the software did not save the
   // number of columns, but they also did not have any columns that are hidden by default, so we know any such column
   // is new.)
   //
   int const numSavedColumns =
      restored ? PersistentSettings::value(numColumnsSettingName(property), 0, *section).toInt() : 0;
   for (int const column : this->m_columnsHiddenByDefault) {
      if (column >= numSavedColumns) {
         columnHeaderView.setSectionHidden(column, true);
      }
   }
   return;
}

void BtTableModel::setColumnsHiddenByDefault(std::initializer_list<int> columns) {
   this->m_columnsHiddenByDefault = columns;
   return;
}

QList<QAction *> BtTableModel::addShowHideColumnsMenu(QMenu & menu, QHeaderView & headerView) const {
   QList<QAction *> columnActions;
   QMenu * columnsMenu = menu.addMenu(tr("Columns"));
   int const numColumns = this->columnCount();
   int numVisible = 0;
   for (int column = 0; column < numColumns; ++column) {
      if (!headerView.isSectionHidden(column)) {
         ++numVisible;
      }
   }

   //
   // List the columns in the order they are currently shown, which may not be the same as their logical order if the
   // user has dragged them around.
   //
   for (int visualIndex = 0; visualIndex < numColumns; ++visualIndex) {
      int const column = headerView.logicalIndex(visualIndex);
      QAction * action = columnsMenu->addAction(this->columnLabel(column).toString());
      action->setCheckable(true);
      bool const isShown = !headerView.isSectionHidden(column);
      action->setChecked(isShown);
      //
      // We don't allow the user to hide the first (name) column, as double-clicking it is how you edit an item.  Nor do
      // we allow hiding the last visible column, as an empty table would be confusing.
      //
      if (column == 0 || (isShown && numVisible <= 1)) {
         action->setEnabled(false);
      }
      connect(action, &QAction::toggled, &headerView, [&headerView, column](bool const checked) {
         headerView.setSectionHidden(column, !checked);
         return;
      });
      columnActions.append(action);
   }

   return columnActions;
}

void BtTableModel::contextMenu(QPoint const & point) {
   qDebug() << Q_FUNC_INFO;
   auto const hView = qobject_cast<QHeaderView *>(this->sender());
   if (!hView) {
      return;
   }
   int const selected = hView->logicalIndexAt(point);

   //
   // If the column holds an amount, the menu starts with options for units, scale and/or decimal places for physical
   // quantities (or decimal places for some non-physical ones, eg percentages).
   //
   // .:TBD:. The logic here is similar to that in SmartLabel::popContextMenu, but I didn't yet figure out how to have
   // more of the code be shared.
   //
   std::unique_ptr<QMenu> menu;
   std::optional<ColumnInfo> columnInfo;
   if (selected >= 0) {
      columnInfo.emplace(this->columnInfo(selected));
      if (UnitAndScalePopUpMenu::isAvailableFor(columnInfo->typeInfo)) {
         //
         // Note that UnitAndScalePopUpMenu::create handles the case where there are two possible physical quantities
         //
         QuantityFieldType const fieldType = *columnInfo->typeInfo.fieldType;
         menu = std::holds_alternative<NonPhysicalQuantity>(fieldType) ?
            UnitAndScalePopUpMenu::create(this->m_parentTableWidget,
                                          std::get<NonPhysicalQuantity>(fieldType),
                                          columnInfo->getForcedDecimalPlaces()) :
            UnitAndScalePopUpMenu::create(this->m_parentTableWidget,
                                          ConvertToPhysicalQuantities(fieldType),
                                          columnInfo->getForcedSystemOfMeasurement(),
                                          columnInfo->getForcedRelativeScale(),
                                          columnInfo->getForcedDecimalPlaces());
      }
   }
   if (!menu) {
      menu = std::make_unique<QMenu>(this->m_parentTableWidget);
   }

   //
   // Every column header also offers the option to show or hide columns
   //
   if (!menu->actions().isEmpty()) {
      menu->addSeparator();
   }
   QList<QAction *> const columnActions = this->addShowHideColumnsMenu(*menu, *hView);

   //
   // This shows the context menu and returns when the user either selects something or dismisses the menu
   //
   QAction const * invoked = menu->exec(hView->mapToGlobal(point));
   if (!invoked || !columnInfo || columnActions.contains(invoked)) {
      // Nothing chosen, or a column shown/hidden (which the action itself has already handled)
      return;
   }

   QVariant const data = invoked->data();
   if (!data.canConvert<UnitAndScalePopUpMenu::Selection>()) {
      return;
   }
   auto const selection = data.value<UnitAndScalePopUpMenu::Selection>();
   qDebug() << Q_FUNC_INFO << "Column" << selected << "updated";
   UnitAndScalePopUpMenu::applySelection(selection, *columnInfo);

   // Make sure the column is redisplayed with the new settings
   emit this->headerDataChanged(Qt::Horizontal, selected, selected);
   if (this->rowCount() > 0) {
      emit this->dataChanged(this->index(0, selected), this->index(this->rowCount() - 1, selected));
   }

   return;
}
