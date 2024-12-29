/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/RecipeAdditionMiscTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "tableModels/RecipeAdditionMiscTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Recipe.h"

RecipeAdditionMiscTableModel::RecipeAdditionMiscTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{
      parent,
      editable,
      {
         //
         // Note that for Name, we want the name of the contained Misc, not the name of the RecipeAdditionMisc
         //
         // Note that we have to use PropertyNames::NamedEntityWithInventory::inventoryWithUnits because
         // PropertyNames::NamedEntityWithInventory::inventory is not implemented
         TABLE_MODEL_HEADER(RecipeAdditionMisc, Name          , tr("Name"       ), PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,
                                                                                                 PropertyNames::NamedEntity::name         }}),
         TABLE_MODEL_HEADER(RecipeAdditionMisc, Type          , tr("Type"       ), PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,
                                                                                                 PropertyNames::Misc::type                 }}, EnumInfo{Misc::typeStringMapping, Misc::typeDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionMisc, Amount        , tr("Amount"     ), PropertyNames::IngredientAmount::amount                  , PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdditionMisc, AmountType    , tr("Amount Type"), PropertyNames::IngredientAmount::amount                  , Misc::validMeasures),
         // In this table, inventory is read-only, so there is intentionally no TotalInventoryType column
         TABLE_MODEL_HEADER(RecipeAdditionMisc, TotalInventory, tr("Inventory"  ), PropertyPath{{PropertyNames::RecipeAdditionMisc::misc,
                                                                                                 PropertyNames::Ingredient::totalInventory}}),
         TABLE_MODEL_HEADER(RecipeAdditionMisc, Stage         , tr("Stage"      ), PropertyNames::RecipeAddition::stage                     , EnumInfo{RecipeAddition::stageStringMapping, RecipeAddition::stageDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionMisc, Time          , tr("Time"       ), PropertyNames::RecipeAddition::addAtTime_mins            , PrecisionInfo{1}),
      }
   },
   TableModelBase<RecipeAdditionMiscTableModel, RecipeAdditionMisc>{},
   showIBUs(false) {
   this->rows.clear();

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdditionMiscTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryMisc>::getInstance(), &ObjectStoreTyped<InventoryMisc>::signalPropertyChanged, this,
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
      return this->getColumnLabel(section);
   }
   if (showIBUs && recObs && orientation == Qt::Vertical && role == Qt::DisplayRole) {
      QList<double> ibus = recObs->IBUs();

      if (ibus.size() > section) {
         return QVariant(QString("%L1 IBU").arg(ibus.at(section), 0, 'f', 1));
      }
   }
   return QVariant();
}

Qt::ItemFlags RecipeAdditionMiscTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<RecipeAdditionMiscTableModel>(
      index,
      this->m_editable,
      {{RecipeAdditionMiscTableModel::ColumnIndex::TotalInventory, Qt::ItemIsEnabled}}
   );
}

bool RecipeAdditionMiscTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdditionMisc, recipeAdditionMisc, PropertyNames::Recipe::miscAdditions)
//=============================================== CLASS RecipeAdditionMiscItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionMisc)
