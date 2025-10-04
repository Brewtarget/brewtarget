/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/StyleTableModel.cpp is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#include "qtModels/tableModels/StyleTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StyleTableModel.cpp"
#endif

COLUMN_INFOS(
   StyleTableModel,
   TABLE_MODEL_HEADER(Style, Name            , tr("Name"           ), PropertyNames::NamedEntity::name            ),
   TABLE_MODEL_HEADER(Style, Type            , tr("Type"           ), PropertyNames::Style::type                  ),
   TABLE_MODEL_HEADER(Style, Category        , tr("Category"       ), PropertyNames::Style::category              ),
   TABLE_MODEL_HEADER(Style, CategoryNumber  , tr("Category Number"), PropertyNames::Style::categoryNumber        ),
   TABLE_MODEL_HEADER(Style, StyleLetter     , tr("Style Letter"   ), PropertyNames::Style::styleLetter           ),
   TABLE_MODEL_HEADER(Style, StyleGuide      , tr("Style Guide"    ), PropertyNames::Style::styleGuide            ),
   TABLE_MODEL_HEADER(Style, NumRecipesUsedIn, tr("N° Recipes"     ), PropertyNames::NamedEntity::numRecipesUsedIn),
)

StyleTableModel::StyleTableModel(QTableView* parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<StyleTableModel, Style>{} {
   this->m_rows.clear();

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &StyleTableModel::contextMenu);
   return;
}

StyleTableModel::~StyleTableModel() = default;

void StyleTableModel::added  ([[maybe_unused]] std::shared_ptr<Style> item) { return; }
void StyleTableModel::removed([[maybe_unused]] std::shared_ptr<Style> item) { return; }
void StyleTableModel::updateTotals()                                       { return; }

QVariant StyleTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

Qt::ItemFlags StyleTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<StyleTableModel>(index, this->m_editable);
}

bool StyleTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Style, style, PropertyNames::Recipe::styleId)
//=============================================== CLASS StyleItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Style)
