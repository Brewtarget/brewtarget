/*======================================================================================================================
 * qtModels/tableModels/RecipeTableModel.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
 =====================================================================================================================*/
#include "qtModels/tableModels/RecipeTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeTableModel.cpp"
#endif

RecipeTableModel::RecipeTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(Recipe, Name          , tr("Name"           ), PropertyNames::NamedEntity::name    ),
      }
   },
   TableModelBase<RecipeTableModel, Recipe>{} {
   this->m_rows.clear();

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeTableModel::contextMenu);
   return;
}

RecipeTableModel::~RecipeTableModel() = default;

void RecipeTableModel::added  ([[maybe_unused]] std::shared_ptr<Recipe> item) { return; }
void RecipeTableModel::removed([[maybe_unused]] std::shared_ptr<Recipe> item) { return; }
void RecipeTableModel::updateTotals()                                         { return; }

QVariant RecipeTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

Qt::ItemFlags RecipeTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<RecipeTableModel>(index, this->m_editable);
}

bool RecipeTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Recipe, recipe, PropertyNames::None::none)
//=============================================== CLASS RecipeItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Recipe)
