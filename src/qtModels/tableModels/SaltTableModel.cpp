/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/SaltTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Tim Payne <swstim@gmail.com>
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
#include "qtModels/tableModels/SaltTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_SaltTableModel.cpp"
#endif

template<> std::vector<ColumnInfo> const ColumnOwnerTraits<SaltTableModel>::columnInfos {
   // NOTE: Need PropertyNames::Salt::amountWithUnits not PropertyNames::Salt::amount below so we
   //       can handle mass-or-volume generically in TableModelBase.
   //
   TABLE_MODEL_HEADER(Salt, Name              , tr("Name"       ), PropertyNames::NamedEntity::name            ),
   TABLE_MODEL_HEADER(Salt, Type              , tr("Type"       ), PropertyNames::Salt::type                   ),
   TABLE_MODEL_HEADER(Salt, PctAcid           , tr("% Acid"     ), PropertyNames::Salt::percentAcid            ),
   TABLE_MODEL_HEADER(Salt, TotalInventory    , tr("Inventory"  ), PropertyNames::Ingredient::totalInventory   ),
   TABLE_MODEL_HEADER(Salt, TotalInventoryType, tr("Amount Type"), PropertyNames::Ingredient::totalInventory   , Salt::validMeasures),
   TABLE_MODEL_HEADER(Salt, NumRecipesUsedIn  , tr("N° Recipes" ), PropertyNames::NamedEntity::numRecipesUsedIn),
};

SaltTableModel::SaltTableModel(QTableView* parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<SaltTableModel, Salt>{} {

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   headerView->setMinimumSectionSize(parent->width()/this->columnCount());

   connect(headerView, &QWidget::customContextMenuRequested, this, &SaltTableModel::contextMenu);
   return;
}

SaltTableModel::~SaltTableModel() = default;

void SaltTableModel::added  ([[maybe_unused]] std::shared_ptr<Salt> item) { return; }
void SaltTableModel::removed([[maybe_unused]] std::shared_ptr<Salt> item) { return; }
void SaltTableModel::updateTotals()                                       { return; }

QVariant SaltTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

Qt::ItemFlags SaltTableModel::flags(const QModelIndex& index) const {
   // Q_UNUSED(index)
   if (index.row() >= this->m_rows.size() ) {
      return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }

   auto const row = this->m_rows[index.row()];
   if (!row->isAcid() && index.column() == static_cast<int>(SaltTableModel::ColumnIndex::PctAcid))  {
      return Qt::NoItemFlags;
   }
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool SaltTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->indexAndRoleOk(index, role)) {
      return false;
   }

   // No special handling required for any of our columns...
   bool const retVal = this->writeDataToModel(index, value, role);

   // ...but some other post-modification things we check
   emit dataChanged(index,index);
   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   headerView->resizeSections(QHeaderView::ResizeToContents);

   return retVal;
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Salt, salt, PropertyNames::None::none)
//=============================================== CLASS SaltItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Salt)
