/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/RecipeAdditionYeastTableModel.cpp is part of Brewtarget, and is copyright the following authors
 * 2009-2025:
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
#include "qtModels/tableModels/RecipeAdditionYeastTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/StockPurchase.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdditionYeastTableModel.cpp"
#endif

COLUMN_INFOS(
   RecipeAdditionYeastTableModel,
   //
   // Note that for Name, we want the name of the contained Yeast, not the name of the RecipeAdditionYeast
   //
   TABLE_MODEL_HEADER(RecipeAdditionYeast, Name          , PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,        // "Name"
                                                                         PropertyNames::NamedEntity::name      }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionYeast, Laboratory    , PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,        // "Laboratory"
                                                                         PropertyNames::Yeast::laboratory      }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionYeast, ProductId     , PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,        // "Product ID"
                                                                         PropertyNames::Yeast::productId       }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionYeast, Type          , PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,        // "Type"
                                                                         PropertyNames::Yeast::type             }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionYeast, Form          , PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,        // "Form"
                                                                         PropertyNames::Yeast::form             }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionYeast, Amount        , PropertyNames::IngredientAmount::amount                ),       // "Amount"
   TABLE_MODEL_HEADER(RecipeAdditionYeast, AmountType    , PropertyNames::IngredientAmount::amount, Yeast::validMeasures), // "Amount Type"
   // Total inventory is read-only, so there is intentionally no TotalInventoryType column
   TABLE_MODEL_HEADER(RecipeAdditionYeast, TotalInventory, PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,        // "Inventory"
                                                                         PropertyNames::Ingredient::totalInventory}, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionYeast, Stage         , PropertyNames::RecipeAddition::stage                   ),       // "Stage"
   TABLE_MODEL_HEADER(RecipeAdditionYeast, Step          , PropertyNames::RecipeAddition::step                    ),       // "Step"
   TABLE_MODEL_HEADER(RecipeAdditionYeast, Attenuation   , PropertyNames::RecipeAdditionYeast::attenuation_pct    ),       // "Attenuation"
   TABLE_MODEL_HEADER(RecipeAdditionYeast, TimesCultured , PropertyNames::RecipeAdditionYeast::timesCultured      ),       // "Times Cultured"
)

RecipeAdditionYeastTableModel::RecipeAdditionYeastTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{parent, editable},
   TableModelBase<RecipeAdditionYeastTableModel, RecipeAdditionYeast>{} {
   this->m_rows.clear();

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdditionYeastTableModel::contextMenu);
   connect(&ObjectStoreTyped<StockPurchaseYeast>::getInstance(), &ObjectStoreTyped<StockPurchaseYeast>::signalPropertyChanged, this,
           &RecipeAdditionYeastTableModel::changedInventory);
   return;
}

RecipeAdditionYeastTableModel::~RecipeAdditionYeastTableModel() = default;

void RecipeAdditionYeastTableModel::added  ([[maybe_unused]] std::shared_ptr<RecipeAdditionYeast> item) { return; }
void RecipeAdditionYeastTableModel::removed([[maybe_unused]] std::shared_ptr<RecipeAdditionYeast> item) { return; }
void RecipeAdditionYeastTableModel::updateTotals()                                                    { return; }

QVariant RecipeAdditionYeastTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

bool RecipeAdditionYeastTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdditionYeast, recipeAdditionYeast, PropertyNames::Recipe::yeastAdditions)
//=============================================== CLASS RecipeAdditionYeastItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionYeast)
