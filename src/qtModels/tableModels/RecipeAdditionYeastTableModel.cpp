/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/RecipeAdditionYeastTableModel.cpp is part of Brewtarget, and is copyright the following authors
 * 2009-2024:
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
#include "model/Inventory.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdditionYeastTableModel.cpp"
#endif

RecipeAdditionYeastTableModel::RecipeAdditionYeastTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{
      parent,
      editable,
      {
         //
         // Note that for Name, we want the name of the contained Yeast, not the name of the RecipeAdditionYeast
         //
         // Note that we have to use PropertyNames::NamedEntityWithInventory::inventoryWithUnits because
         // PropertyNames::NamedEntityWithInventory::inventory is not implemented
         TABLE_MODEL_HEADER(RecipeAdditionYeast, Name          , tr("Name"          ), PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,
                                                                                                     PropertyNames::NamedEntity::name      }}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, Laboratory    , tr("Laboratory"    ), PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,
                                                                                                     PropertyNames::Yeast::laboratory      }}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, ProductId     , tr("Product ID"    ), PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,
                                                                                                     PropertyNames::Yeast::productId       }}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, Type          , tr("Type"          ), PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,
                                                                                                     PropertyNames::Yeast::type             }}, EnumInfo{Yeast::typeStringMapping, Yeast::typeDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, Form          , tr("Form"          ), PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,
                                                                                                     PropertyNames::Yeast::form             }}, EnumInfo{Yeast::formStringMapping, Yeast::formDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, Amount        , tr("Amount"        ), PropertyNames::IngredientAmount::amount                , PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, AmountType    , tr("Amount Type"   ), PropertyNames::IngredientAmount::amount                , Yeast::validMeasures),
         // In this table, inventory is read-only, so there is intentionally no TotalInventoryType column
         TABLE_MODEL_HEADER(RecipeAdditionYeast, TotalInventory, tr("Inventory"     ), PropertyPath{{PropertyNames::RecipeAdditionYeast::yeast,
                                                                                                   PropertyNames::Ingredient::totalInventory}}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, Stage         , tr("Stage"         ), PropertyNames::RecipeAddition::stage                     , EnumInfo{RecipeAddition::stageStringMapping, RecipeAddition::stageDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, Step          , tr("Step"          ), PropertyNames::RecipeAddition::step                    ),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, Attenuation   , tr("Attenuation"   ), PropertyNames::RecipeAdditionYeast::attenuation_pct    , PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdditionYeast, TimesCultured , tr("Times Cultured"), PropertyNames::RecipeAdditionYeast::timesCultured      ),
      }
   },
   TableModelBase<RecipeAdditionYeastTableModel, RecipeAdditionYeast>{} {
   this->rows.clear();

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdditionYeastTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryYeast>::getInstance(), &ObjectStoreTyped<InventoryYeast>::signalPropertyChanged, this,
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

Qt::ItemFlags RecipeAdditionYeastTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<RecipeAdditionYeastTableModel>(
      index,
      this->m_editable,
      {{RecipeAdditionYeastTableModel::ColumnIndex::TotalInventory, Qt::ItemIsEnabled}}
   );
}

bool RecipeAdditionYeastTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdditionYeast, recipeAdditionYeast, PropertyNames::Recipe::yeastAdditions)
//=============================================== CLASS RecipeAdditionYeastItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionYeast)
