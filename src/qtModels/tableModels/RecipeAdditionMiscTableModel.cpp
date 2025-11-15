/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/RecipeAdditionMiscTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#include "qtModels/tableModels/RecipeAdditionMiscTableModel.h"

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
   #include "moc_RecipeAdditionMiscTableModel.cpp"
#endif

COLUMN_INFOS(
   RecipeAdditionMiscTableModel,
   //
   // Note that for Name, we want the name of the contained Misc, not the name of the RecipeAdditionMisc
   //
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Name          , PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,         // "Name"
                                                                        PropertyNames::NamedEntity::name      }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Type          , PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,         // "Type"
                                                                        PropertyNames::Misc::type             }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Amount        , PropertyNames::IngredientAmount::amount             ),         // "Amount"
   TABLE_MODEL_HEADER(RecipeAdditionMisc, AmountType    , PropertyNames::IngredientAmount::amount, Misc::validMeasures), // "Amount Type"
   // Total inventory is read-only, so there is intentionally no TotalInventoryType column
   TABLE_MODEL_HEADER(RecipeAdditionMisc, TotalInventory, PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,         // "Inventory"
                                                                        PropertyNames::Ingredient::totalInventory}, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Stage         , PropertyNames::RecipeAddition::stage                     ),    // "Stage"
   TABLE_MODEL_HEADER(RecipeAdditionMisc, Time          , PropertyNames::RecipeAddition::addAtTime_mins            ),    // "Time"
)

RecipeAdditionMiscTableModel::RecipeAdditionMiscTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{parent, editable},
   TableModelBase<RecipeAdditionMiscTableModel, RecipeAdditionMisc>{},
   showIBUs(false) {
   this->m_rows.clear();

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdditionMiscTableModel::contextMenu);
   connect(&ObjectStoreTyped<StockPurchaseMisc>::getInstance(), &ObjectStoreTyped<StockPurchaseMisc>::signalPropertyChanged, this,
           &RecipeAdditionMiscTableModel::changedInventory);
   return;
}

RecipeAdditionMiscTableModel::~RecipeAdditionMiscTableModel() = default;

void RecipeAdditionMiscTableModel::added  ([[maybe_unused]] std::shared_ptr<RecipeAdditionMisc> item) { return; }
void RecipeAdditionMiscTableModel::removed([[maybe_unused]] std::shared_ptr<RecipeAdditionMisc> item) { return; }
void RecipeAdditionMiscTableModel::updateTotals()                                                    { return; }

void RecipeAdditionMiscTableModel::setShowIBUs(bool var) {
   showIBUs = var;
   return;
}

QVariant RecipeAdditionMiscTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

QVariant RecipeAdditionMiscTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return ColumnOwnerTraits<RecipeAdditionMiscTableModel>::getColumnLabel(section);
   }
   if (showIBUs && recObs && orientation == Qt::Vertical && role == Qt::DisplayRole) {
      QList<double> ibus = recObs->IBUs();

      if (ibus.size() > section) {
         return QVariant(QString("%L1 IBU").arg(ibus.at(section), 0, 'f', 1));
      }
   }
   return QVariant();
}

bool RecipeAdditionMiscTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdditionMisc, recipeAdditionMisc, PropertyNames::Recipe::miscAdditions)
//=============================================== CLASS RecipeAdditionMiscItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionMisc)
