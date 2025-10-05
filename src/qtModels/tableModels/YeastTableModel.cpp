/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/YeastTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Tim Payne <swstim@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "qtModels/tableModels/YeastTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_YeastTableModel.cpp"
#endif

COLUMN_INFOS(
   YeastTableModel,
   // NOTE: Need PropertyNames::Yeast::amountWithUnits not PropertyNames::Yeast::amount below so we
   //       can handle mass-or-volume generically in TableModelBase.  Same for inventoryWithUnits.
   TABLE_MODEL_HEADER(Yeast, Name              , tr("Name"       ), PropertyNames::NamedEntity::name            ),
   TABLE_MODEL_HEADER(Yeast, Laboratory        , tr("Laboratory" ), PropertyNames::Yeast::laboratory            ),
   TABLE_MODEL_HEADER(Yeast, ProductId         , tr("Product ID" ), PropertyNames::Yeast::productId             ),
   TABLE_MODEL_HEADER(Yeast, Type              , tr("Type"       ), PropertyNames::Yeast::type                  ),
   TABLE_MODEL_HEADER(Yeast, Form              , tr("Form"       ), PropertyNames::Yeast::form                  ),
   TABLE_MODEL_HEADER(Yeast, TotalInventory    , tr("Inventory"  ), PropertyNames::Ingredient::totalInventory   ),
   TABLE_MODEL_HEADER(Yeast, TotalInventoryType, tr("Amount Type"), PropertyNames::Ingredient::totalInventory   , Yeast::validMeasures),
   TABLE_MODEL_HEADER(Yeast, NumRecipesUsedIn  , tr("N° Recipes" ), PropertyNames::NamedEntity::numRecipesUsedIn),
)

YeastTableModel::YeastTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<YeastTableModel, Yeast>{} {

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &YeastTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryYeast>::getInstance(),
           &ObjectStoreTyped<InventoryYeast>::signalPropertyChanged,
           this,
           &YeastTableModel::changedInventory);
   return;
}

YeastTableModel::~YeastTableModel() = default;

void YeastTableModel::added  ([[maybe_unused]] std::shared_ptr<Yeast> item) { return; }
void YeastTableModel::removed([[maybe_unused]] std::shared_ptr<Yeast> item) { return; }
void YeastTableModel::updateTotals()                                        { return; }

QVariant YeastTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

Qt::ItemFlags YeastTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<YeastTableModel>(
      index,
      this->m_editable,
      {{YeastTableModel::ColumnIndex::TotalInventory, Qt::ItemIsEditable}}
   );
}

bool YeastTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Yeast, yeast, PropertyNames::None::none)
//============================================== CLASS YeastItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Yeast)
