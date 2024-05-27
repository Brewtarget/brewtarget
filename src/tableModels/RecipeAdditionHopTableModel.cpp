/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/RecipeAdditionHopTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2023:
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
#include "tableModels/RecipeAdditionHopTableModel.h"

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

RecipeAdditionHopTableModel::RecipeAdditionHopTableModel(QTableView * parent, bool editable) :
   BtTableModelRecipeObserver{
      parent,
      editable,
      {
         //
         // Note that for Name, we want the name of the contained Hop, not the name of the RecipeAdditionHop
         //
         // Note that we have to use PropertyNames::NamedEntityWithInventory::inventoryWithUnits because
         // PropertyNames::NamedEntityWithInventory::inventory is not implemented
         TABLE_MODEL_HEADER(RecipeAdditionHop, Name          , tr("Name"       ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                                PropertyNames::NamedEntity::name         }}),
         TABLE_MODEL_HEADER(RecipeAdditionHop, Form          , tr("Form"       ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                                PropertyNames::Hop::form                 }}, EnumInfo{Hop::formStringMapping, Hop::formDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionHop, Alpha         , tr("Alpha %"    ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                                PropertyNames::Hop::alpha_pct            }}, PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdditionHop, Year          , tr("Year"       ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                                PropertyNames::Hop::year                 }}),
         TABLE_MODEL_HEADER(RecipeAdditionHop, Amount        , tr("Amount"     ), PropertyNames::IngredientAmount::amount                  , PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdditionHop, AmountType    , tr("Amount Type"), PropertyNames::IngredientAmount::amount                  , Hop::validMeasures),
         // In this table, inventory is read-only, so there is intentionally no TotalInventoryType column
         TABLE_MODEL_HEADER(RecipeAdditionHop, TotalInventory, tr("Inventory"  ), PropertyPath{{PropertyNames::RecipeAdditionHop::hop,
                                                                                                PropertyNames::Ingredient::totalInventory}}),
         TABLE_MODEL_HEADER(RecipeAdditionHop, Stage         , tr("Stage"      ), PropertyNames::RecipeAddition::stage                     , EnumInfo{RecipeAddition::stageStringMapping, RecipeAddition::stageDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdditionHop, Time          , tr("Time"       ), PropertyNames::RecipeAddition::addAtTime_mins            , PrecisionInfo{1}),
      }
   },
   TableModelBase<RecipeAdditionHopTableModel, RecipeAdditionHop>{},
   showIBUs(false) {
   this->rows.clear();
   this->setObjectName("hopAdditionTable");

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

QVariant RecipeAdditionHopTableModel::data(const QModelIndex & index, int role) const {
   if (!this->isIndexOk(index)) {
      return QVariant();
   }

   auto const columnIndex = static_cast<RecipeAdditionHopTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case RecipeAdditionHopTableModel::ColumnIndex::Name          :
      case RecipeAdditionHopTableModel::ColumnIndex::Form          :
      case RecipeAdditionHopTableModel::ColumnIndex::Alpha         :
      case RecipeAdditionHopTableModel::ColumnIndex::Year          :
      case RecipeAdditionHopTableModel::ColumnIndex::Amount        :
      case RecipeAdditionHopTableModel::ColumnIndex::AmountType    :
      case RecipeAdditionHopTableModel::ColumnIndex::TotalInventory:
      case RecipeAdditionHopTableModel::ColumnIndex::Stage         :
      case RecipeAdditionHopTableModel::ColumnIndex::Time          :
         return this->readDataFromModel(index, role);

      // No default case as we want the compiler to warn us if we missed one
   }
   return QVariant();
}

QVariant RecipeAdditionHopTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

Qt::ItemFlags RecipeAdditionHopTableModel::flags(const QModelIndex & index) const {
   auto const columnIndex = static_cast<RecipeAdditionHopTableModel::ColumnIndex>(index.column());
   if (columnIndex == RecipeAdditionHopTableModel::ColumnIndex::Name) {
      return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }
   if (columnIndex == RecipeAdditionHopTableModel::ColumnIndex::TotalInventory) {
      return Qt::ItemIsEnabled | Qt::NoItemFlags;
   }
   return Qt::ItemIsSelectable |
          (this->m_editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool RecipeAdditionHopTableModel::setData(const QModelIndex & index, const QVariant & value, int role) {
   if (!this->isIndexOk(index)) {
      return false;
   }

   bool retVal = false;

   auto row = this->rows[index.row()];
///   Measurement::PhysicalQuantity physicalQuantity = row->getMeasure();

   auto const columnIndex = static_cast<RecipeAdditionHopTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case RecipeAdditionHopTableModel::ColumnIndex::Name          :
      case RecipeAdditionHopTableModel::ColumnIndex::Form          :
      case RecipeAdditionHopTableModel::ColumnIndex::Alpha         :
      case RecipeAdditionHopTableModel::ColumnIndex::Year          :
      case RecipeAdditionHopTableModel::ColumnIndex::Amount        :
      case RecipeAdditionHopTableModel::ColumnIndex::AmountType    :
      case RecipeAdditionHopTableModel::ColumnIndex::TotalInventory:
      case RecipeAdditionHopTableModel::ColumnIndex::Stage         :
      case RecipeAdditionHopTableModel::ColumnIndex::Time          :
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
TABLE_MODEL_COMMON_CODE(RecipeAdditionHop, recipeAdditionHop, PropertyNames::Recipe::hopAdditionIds)
//=============================================== CLASS RecipeAdditionHopItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdditionHop)
