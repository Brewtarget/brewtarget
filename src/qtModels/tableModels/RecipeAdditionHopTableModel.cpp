/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/RecipeAdditionHopTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Luke Vincent <luke.r.vincent@gmail.com>
 *   • Markus Mårtensson <mackan.90@gmail.com>
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
#include "qtModels/tableModels/RecipeAdditionHopTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdditionHopTableModel.cpp"
#endif

template<> std::vector<ColumnInfo> const ColumnOwnerTraits<RecipeAdditionHopTableModel>::columnInfos {
   //
   // Note that for Name, we want the name of the contained Hop, not the name of the RecipeAdditionHop
   //
   // Note that we have to use PropertyNames::NamedEntityWithInventory::inventoryWithUnits because
   // PropertyNames::NamedEntityWithInventory::inventory is not implemented
   TABLE_MODEL_HEADER(RecipeAdditionHop, Name          , tr("Name"       ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                          PropertyNames::NamedEntity::name         }}),
   TABLE_MODEL_HEADER(RecipeAdditionHop, Form          , tr("Form"       ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                          PropertyNames::Hop::form                 }}),
   TABLE_MODEL_HEADER(RecipeAdditionHop, Alpha         , tr("Alpha %"    ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                          PropertyNames::Hop::alpha_pct            }}),
   TABLE_MODEL_HEADER(RecipeAdditionHop, Year          , tr("Year"       ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                          PropertyNames::Hop::year                 }}),
   TABLE_MODEL_HEADER(RecipeAdditionHop, Amount        , tr("Amount"     ), PropertyNames::IngredientAmount::amount                  ),
   TABLE_MODEL_HEADER(RecipeAdditionHop, AmountType    , tr("Amount Type"), PropertyNames::IngredientAmount::amount                  , Hop::validMeasures),
   // In this table, inventory is read-only, so there is intentionally no TotalInventoryType column
   TABLE_MODEL_HEADER(RecipeAdditionHop, TotalInventory, tr("Inventory"  ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                          PropertyNames::Ingredient::totalInventory}}),
   TABLE_MODEL_HEADER(RecipeAdditionHop, Stage         , tr("Stage"      ), PropertyNames::RecipeAddition::stage                     ),
   TABLE_MODEL_HEADER(RecipeAdditionHop, Time          , tr("Time"       ), PropertyNames::RecipeAddition::addAtTime_mins            ),
};

RecipeAdditionHopTableModel::RecipeAdditionHopTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{parent, editable},
   TableModelBase<RecipeAdditionHopTableModel, RecipeAdditionHop>{},
   showIBUs(false) {
   this->m_rows.clear();

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdditionHopTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryHop>::getInstance(), &ObjectStoreTyped<InventoryHop>::signalPropertyChanged, this,
           &RecipeAdditionHopTableModel::changedInventory);
   return;
}

RecipeAdditionHopTableModel::~RecipeAdditionHopTableModel() = default;

void RecipeAdditionHopTableModel::added  ([[maybe_unused]] std::shared_ptr<RecipeAdditionHop> item) { return; }
void RecipeAdditionHopTableModel::removed([[maybe_unused]] std::shared_ptr<RecipeAdditionHop> item) { return; }
void RecipeAdditionHopTableModel::updateTotals()                                                    { return; }

void RecipeAdditionHopTableModel::setShowIBUs(bool var) {
   showIBUs = var;
   return;
}

QVariant RecipeAdditionHopTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

QVariant RecipeAdditionHopTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return ColumnOwnerTraits<RecipeAdditionHopTableModel>::getColumnLabel(section);
   }
   if (showIBUs && recObs && orientation == Qt::Vertical && role == Qt::DisplayRole) {
      QList<double> ibus = recObs->IBUs();

      if (ibus.size() > section) {
         return QVariant(QString("%L1 IBU").arg(ibus.at(section), 0, 'f', 1));
      }
   }
   return QVariant();
}

Qt::ItemFlags RecipeAdditionHopTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<RecipeAdditionHopTableModel>(
      index,
      this->m_editable,
      {{RecipeAdditionHopTableModel::ColumnIndex::TotalInventory, Qt::ItemIsEnabled}}
   );
}

bool RecipeAdditionHopTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   // Template parameter is true as we might need to re-show header IBUs
   return this->doSetDataDefault<true>(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdditionHop, recipeAdditionHop, PropertyNames::Recipe::hopAdditions)
//=============================================== CLASS RecipeAdditionHopItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionHop)
