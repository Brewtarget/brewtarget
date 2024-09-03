/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/StyleTableModel.cpp is part of Brewtarget, and is copyright the following authors 2023:
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
#include "tableModels/StyleTableModel.h"

#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>

#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"
#include "widgets/BtComboBox.h"

StyleTableModel::StyleTableModel(QTableView* parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(Style, Name          , tr("Name"           ), PropertyNames::NamedEntity::name    ),
         TABLE_MODEL_HEADER(Style, Type          , tr("Type"           ), PropertyNames::Style::type          , EnumInfo{Style::typeStringMapping, Style::typeDisplayNames}),
         TABLE_MODEL_HEADER(Style, Category      , tr("Category"       ), PropertyNames::Style::category      ),
         TABLE_MODEL_HEADER(Style, CategoryNumber, tr("Category Number"), PropertyNames::Style::categoryNumber),
         TABLE_MODEL_HEADER(Style, StyleLetter   , tr("Style Letter"   ), PropertyNames::Style::styleLetter   ),
         TABLE_MODEL_HEADER(Style, StyleGuide    , tr("Style Guide"    ), PropertyNames::Style::styleGuide    ),
      }
   },
   TableModelBase<StyleTableModel, Style>{} {
   this->rows.clear();
   setObjectName("styleTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &StyleTableModel::contextMenu);
   return;
}

StyleTableModel::~StyleTableModel() = default;

void StyleTableModel::added  ([[maybe_unused]] std::shared_ptr<Style> item) { return; }
void StyleTableModel::removed([[maybe_unused]] std::shared_ptr<Style> item) { return; }
void StyleTableModel::updateTotals()                                       { return; }

QVariant StyleTableModel::data(QModelIndex const & index, int role) const {
   if (!this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   // No special handling required for any of our columns
   return this->readDataFromModel(index, role);
}

Qt::ItemFlags StyleTableModel::flags(QModelIndex const & index) const {
   Qt::ItemFlags const defaults = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
   auto const columnIndex = static_cast<StyleTableModel::ColumnIndex>(index.column());
   if (columnIndex == StyleTableModel::ColumnIndex::Name) {
      return defaults;
   }
   return defaults | (this->m_editable ? Qt::ItemIsEditable : Qt::NoItemFlags);
}

bool StyleTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->indexAndRoleOk(index, role)) {
      return false;
   }

   // No special handling required for any of our columns
   return this->writeDataToModel(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Style, style, PropertyNames::Recipe::styleId)
//=============================================== CLASS StyleItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Style)
