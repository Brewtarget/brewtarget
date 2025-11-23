/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/RecipeAdditionFermentableTableModel.cpp is part of Brewtarget, and is copyright the following authors
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
#include "qtModels/tableModels/RecipeAdditionFermentableTableModel.h"

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
   #include "moc_RecipeAdditionFermentableTableModel.cpp"
#endif

COLUMN_INFOS(
   RecipeAdditionFermentableTableModel,
   //
   // Note that for Name, we want the name of the contained Fermentable, not the name of the RecipeAdditionFermentable
   //
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, Name          , PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,   // "Name"
                                                                               PropertyNames::NamedEntity::name                 }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, Type          , PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,   // "Type"
                                                                               PropertyNames::Fermentable::type                 }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, Yield         , PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,   // "Yield"
                                                                               PropertyNames::Fermentable::fineGrindYield_pct   }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, Color         , PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,   // "Color"
                                                                               PropertyNames::Fermentable::color_srm            }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, Amount        , PropertyNames::IngredientAmount::amount),                              // "Amount"
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, AmountType    , PropertyNames::IngredientAmount::amount, Fermentable::validMeasures),  // "Amount Type"
   // Total inventory is read-only, so there is intentionally no TotalInventoryType column
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, TotalInventory, PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable, // "Inventory"
                                                                               PropertyNames::Ingredient::totalInventory       }, 1}),
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, Stage         , PropertyNames::RecipeAddition::stage         ), // "Stage"
   TABLE_MODEL_HEADER(RecipeAdditionFermentable, Time          , PropertyNames::RecipeAddition::addAtTime_mins), // "Time"
)

RecipeAdditionFermentableTableModel::RecipeAdditionFermentableTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{parent, editable},
   TableModelBase<RecipeAdditionFermentableTableModel, RecipeAdditionFermentable>{},
   displayPercentages(false),
   totalFermMass_kg(0) {
   this->m_rows.clear();

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdditionFermentableTableModel::contextMenu);
   connect(&ObjectStoreTyped<StockPurchaseFermentable>::getInstance(), &ObjectStoreTyped<StockPurchaseFermentable>::signalPropertyChanged, this,
           &RecipeAdditionFermentableTableModel::changedInventory);
   return;
}

RecipeAdditionFermentableTableModel::~RecipeAdditionFermentableTableModel() = default;

// .:TODO:.:JSON:.  Now that fermentables can also be measured by volume, we might need to rethink this
void RecipeAdditionFermentableTableModel::added  (std::shared_ptr<RecipeAdditionFermentable> item) { if (item->amount().unit == &Measurement::Units::kilograms) { this->totalFermMass_kg += item->amount().quantity; } return; }
void RecipeAdditionFermentableTableModel::removed(std::shared_ptr<RecipeAdditionFermentable> item) { if (item->amount().unit == &Measurement::Units::kilograms) { this->totalFermMass_kg -= item->amount().quantity; } return; }
void RecipeAdditionFermentableTableModel::updateTotals() {
   this->totalFermMass_kg = 0;
   for (auto const & ferm : this->m_rows) {
      if (ferm->amount().unit->getPhysicalQuantity() == Measurement::PhysicalQuantity::Mass) {
         totalFermMass_kg += ferm->amount().quantity;
      }
   }
   if (this->displayPercentages && this->rowCount() > 0) {
      emit headerDataChanged(Qt::Vertical, 0, this->rowCount() - 1);
   }
   return;
}

void RecipeAdditionFermentableTableModel::setDisplayPercentages(bool var) {
   this->displayPercentages = var;
   return;
}

QVariant RecipeAdditionFermentableTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

QVariant RecipeAdditionFermentableTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return ColumnOwnerTraits<RecipeAdditionFermentableTableModel>::getColumnLabel(section);
   }
   if (displayPercentages && orientation == Qt::Vertical && role == Qt::DisplayRole) {
      double perMass = 0.0;
      if (totalFermMass_kg > 0.0 ) {
         // .:TODO:. Work out what to do for amounts that are volumes
         if (this->m_rows[section]->amount().unit->getPhysicalQuantity() == Measurement::PhysicalQuantity::Mass) {
            perMass = this->m_rows[section]->amount().quantity/totalFermMass_kg;
         } else {
//            qWarning() << Q_FUNC_INFO << "Unhandled branch for liquid fermentables";
         }
      }
      return QVariant( QString("%1%").arg( static_cast<double>(100.0) * perMass, 0, 'f', 0 ) );
   }

   return QVariant();
}

bool RecipeAdditionFermentableTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   // Template parameter is true as we might need to re-show header percentages
   return this->doSetDataDefault<true>(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdditionFermentable, recipeAdditionFermentable, PropertyNames::Recipe::fermentableAdditions)
//=============================================== CLASS RecipeAdditionFermentableItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionFermentable)
