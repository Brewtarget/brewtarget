/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/RecipeAdditionFermentableTableModel.cpp is part of Brewtarget, and is copyright the following authors
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
#include "tableModels/RecipeAdditionFermentableTableModel.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QModelIndex>
#include <QString>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "Localization.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"

RecipeAdditionFermentableTableModel::RecipeAdditionFermentableTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{
      parent,
      editable,
      {
         //
         // Note that for Name, we want the name of the contained Fermentable, not the name of the RecipeAdditionFermentable
         //
         // Note that we have to use PropertyNames::NamedEntityWithInventory::inventoryWithUnits because
         // PropertyNames::NamedEntityWithInventory::inventory is not implemented
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, Name          , tr("Name"       ), PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,
                                                                                                        PropertyNames::NamedEntity::name                 }}),
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, Type          , tr("Type"       ), PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,
                                                                                                        PropertyNames::Fermentable::type                 }}, EnumInfo{Fermentable::typeStringMapping, Fermentable::typeDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, Yield         , tr("Yield"      ), PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,
                                                                                                        PropertyNames::Fermentable::fineGrindYield_pct   }}, PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, Color         , tr("Color"      ), PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,
                                                                                                        PropertyNames::Fermentable::color_srm            }}),
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, Amount        , tr("Amount"     ), PropertyNames::IngredientAmount::amount                  , PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, AmountType    , tr("Amount Type"), PropertyNames::IngredientAmount::amount                  , Fermentable::validMeasures),
         // In this table, inventory is read-only, so there is intentionally no TotalInventoryType column
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, TotalInventory, tr("Inventory"  ), PropertyPath{{PropertyNames::RecipeAdditionFermentable::fermentable,
                                                                                                PropertyNames::Ingredient::totalInventory}}),
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, Stage         , tr("Stage"      ), PropertyNames::RecipeAddition::stage                     , EnumInfo{RecipeAddition::stageStringMapping, RecipeAddition::stageDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionFermentable, Time          , tr("Time"       ), PropertyNames::RecipeAddition::addAtTime_mins            , PrecisionInfo{1}),
      }
   },
   TableModelBase<RecipeAdditionFermentableTableModel, RecipeAdditionFermentable>{},
   displayPercentages(false),
   totalFermMass_kg(0) {
   this->rows.clear();
   this->setObjectName("fermentableAdditionTable");

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdditionFermentableTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryFermentable>::getInstance(), &ObjectStoreTyped<InventoryFermentable>::signalPropertyChanged, this,
           &RecipeAdditionFermentableTableModel::changedInventory);
   return;
}

RecipeAdditionFermentableTableModel::~RecipeAdditionFermentableTableModel() = default;

// .:TODO:.:JSON:.  Now that fermentables can also be measured by volume, we might need to rethink this
void RecipeAdditionFermentableTableModel::added  (std::shared_ptr<RecipeAdditionFermentable> item) { if (item->amount().unit == &Measurement::Units::kilograms) { this->totalFermMass_kg += item->amount().quantity; } return; }
void RecipeAdditionFermentableTableModel::removed(std::shared_ptr<RecipeAdditionFermentable> item) { if (item->amount().unit == &Measurement::Units::kilograms) { this->totalFermMass_kg -= item->amount().quantity; } return; }
void RecipeAdditionFermentableTableModel::updateTotals() {
   this->totalFermMass_kg = 0;
   for (auto const & ferm : this->rows) {
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

QVariant RecipeAdditionFermentableTableModel::data(const QModelIndex & index, int role) const {
   if (!this->isIndexOk(index)) {
      return QVariant();
   }

   auto const columnIndex = static_cast<RecipeAdditionFermentableTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case RecipeAdditionFermentableTableModel::ColumnIndex::Name          :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Type          :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Yield         :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Color         :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Amount        :
      case RecipeAdditionFermentableTableModel::ColumnIndex::AmountType    :
      case RecipeAdditionFermentableTableModel::ColumnIndex::TotalInventory:
      case RecipeAdditionFermentableTableModel::ColumnIndex::Stage         :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Time          :
         return this->readDataFromModel(index, role);

      // No default case as we want the compiler to warn us if we missed one
   }
   return QVariant();
}

QVariant RecipeAdditionFermentableTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumnLabel(section);
   }
   if (displayPercentages && orientation == Qt::Vertical && role == Qt::DisplayRole) {
      double perMass = 0.0;
      if (totalFermMass_kg > 0.0 ) {
         // .:TODO:. Work out what to do for amounts that are volumes
         if (this->rows[section]->amount().unit->getPhysicalQuantity() == Measurement::PhysicalQuantity::Mass) {
            perMass = this->rows[section]->amount().quantity/totalFermMass_kg;
         } else {
//            qWarning() << Q_FUNC_INFO << "Unhandled branch for liquid fermentables";
         }
      }
      return QVariant( QString("%1%").arg( static_cast<double>(100.0) * perMass, 0, 'f', 0 ) );
   }

   return QVariant();
}

Qt::ItemFlags RecipeAdditionFermentableTableModel::flags(const QModelIndex & index) const {
   auto const columnIndex = static_cast<RecipeAdditionFermentableTableModel::ColumnIndex>(index.column());
   if (columnIndex == RecipeAdditionFermentableTableModel::ColumnIndex::Name) {
      return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }
   if (columnIndex == RecipeAdditionFermentableTableModel::ColumnIndex::TotalInventory) {
      return Qt::ItemIsEnabled | Qt::NoItemFlags;
   }
   return Qt::ItemIsSelectable |
          (this->m_editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool RecipeAdditionFermentableTableModel::setData(const QModelIndex & index, const QVariant & value, int role) {
   if (!this->isIndexOk(index)) {
      return false;
   }

   bool retVal = false;

   auto row = this->rows[index.row()];
///   Measurement::PhysicalQuantity physicalQuantity = row->getMeasure();

   auto const columnIndex = static_cast<RecipeAdditionFermentableTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case RecipeAdditionFermentableTableModel::ColumnIndex::Name          :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Type          :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Yield         :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Color         :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Amount        :
      case RecipeAdditionFermentableTableModel::ColumnIndex::AmountType    :
      case RecipeAdditionFermentableTableModel::ColumnIndex::TotalInventory:
      case RecipeAdditionFermentableTableModel::ColumnIndex::Stage         :
      case RecipeAdditionFermentableTableModel::ColumnIndex::Time          :
         retVal = this->writeDataToModel(index, value, role);
         break;

      // We don't need to pass in a PhysicalQuantity for any of the columns

      // No default case as we want the compiler to warn us if we missed one
   }

   if (retVal) {
      headerDataChanged(Qt::Vertical, index.row(), index.row());   // Need to re-show header (IBUs).
   }

   return retVal;
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdditionFermentable, recipeAdditionFermentable, PropertyNames::Recipe::fermentableAdditions)
//=============================================== CLASS RecipeAdditionFermentableItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionFermentable)
