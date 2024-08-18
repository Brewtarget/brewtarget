/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/RecipeAdjustmentSaltTableModel.cpp is part of Brewtarget, and is copyright the following authors
 * 2009-2024:
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
#include "tableModels/RecipeAdjustmentSaltTableModel.h"

#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QComboBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QStandardItemModel>
#include <QString>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "WaterDialog.h"

RecipeAdjustmentSaltTableModel::RecipeAdjustmentSaltTableModel(QTableView* parent, bool editable) :
   BtTableModelRecipeObserver{
      parent,
      editable,
      {
         // NOTE: Need PropertyNames::RecipeAdjustmentSalt::amountWithUnits not PropertyNames::RecipeAdjustmentSalt::amount below so we
         //       can handle mass-or-volume generically in TableModelBase.
         //
         // Note too that, for the purposes of these columns, the "name" of a RecipeAdjustmentSalt is not its "NamedEntity name" but actually its type
         TABLE_MODEL_HEADER(RecipeAdjustmentSalt, Name          , tr("Name"       ), PropertyPath{{PropertyNames::RecipeAdjustmentSalt::salt,
                                                                                                   PropertyNames::NamedEntity::name         }}),
         TABLE_MODEL_HEADER(RecipeAdjustmentSalt, Type          , tr("Type"       ), PropertyPath{{PropertyNames::RecipeAdjustmentSalt::salt,
                                                                                                   PropertyNames::Salt::type                 }},
                            EnumInfo{Salt::typeStringMapping,
                                     Salt::typeDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdjustmentSalt, Amount        , tr("Amount"     ), PropertyNames::IngredientAmount::amount                  , PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdjustmentSalt, AmountType    , tr("Amount Type"), PropertyNames::IngredientAmount::amount                  , Salt::validMeasures),
         // In this table, inventory is read-only, so there is intentionally no TotalInventoryType column
         TABLE_MODEL_HEADER(RecipeAdjustmentSalt, TotalInventory, tr("Inventory"  ), PropertyPath{{PropertyNames::RecipeAdjustmentSalt::salt,
                                                                                                   PropertyNames::Ingredient::totalInventory}}, PrecisionInfo{1}),
         TABLE_MODEL_HEADER(RecipeAdjustmentSalt, AddTo         , tr("Added To"   ), PropertyNames::RecipeAdjustmentSalt::whenToAdd      ,
                            EnumInfo{RecipeAdjustmentSalt::whenToAddStringMapping,
                                     RecipeAdjustmentSalt::whenToAddDisplayNames}),
         TABLE_MODEL_HEADER(RecipeAdjustmentSalt, PctAcid       , tr("% Acid"     ), PropertyPath{{PropertyNames::RecipeAdjustmentSalt::salt,
                                                                                                   PropertyNames::Salt::percentAcid         }}),
      }
   },
   TableModelBase<RecipeAdjustmentSaltTableModel, RecipeAdjustmentSalt>{} {
   setObjectName("saltTable");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   headerView->setMinimumSectionSize(parent->width()/this->columnCount());

   connect(headerView, &QWidget::customContextMenuRequested, this, &RecipeAdjustmentSaltTableModel::contextMenu);
   return;
}

RecipeAdjustmentSaltTableModel::~RecipeAdjustmentSaltTableModel() = default;

void RecipeAdjustmentSaltTableModel::added  ([[maybe_unused]] std::shared_ptr<RecipeAdjustmentSalt> item) { return; }
void RecipeAdjustmentSaltTableModel::removed(std::shared_ptr<RecipeAdjustmentSalt> item) {
   // Dead salts do not malinger in the database. This will
   // delete the thing, not just mark it deleted
   if (item->key() > 0) {
      this->recObs->removeAddition(item);
      ObjectStoreWrapper::hardDelete(*item);
   }

   emit newTotals();
   return;
}
void RecipeAdjustmentSaltTableModel::updateTotals() { return; }

void RecipeAdjustmentSaltTableModel::catchSalt() {
   // This gets stored in the DB in saveAndClose()
   auto gaq = std::make_shared<RecipeAdjustmentSalt>("");
   this->add(gaq);
   return;
}

double RecipeAdjustmentSaltTableModel::multiplier(RecipeAdjustmentSalt & salt) const {
   double ret = 1.0;

   if (this->recObs->mash()->hasSparge() ) {
      if (salt.whenToAdd() == RecipeAdjustmentSalt::WhenToAdd::Equal) {
         ret = 2.0;
      } else if (salt.whenToAdd() == RecipeAdjustmentSalt::WhenToAdd::Ratio) {
         // If we are adding a proportional amount to both,
         // this should handle that math.
         double spargePct = this->recObs->mash()->totalSpargeAmount_l()/this->recObs->mash()->totalInfusionAmount_l();
         ret = 1.0 + spargePct;
      }
   }

   // Per comment in model/Salt.cpp we need to multiply by 1000.0 to convert kilograms to grams
   return ret * 1000.0;
}

// total salt in ppm. Not sure this is helping.
double RecipeAdjustmentSaltTableModel::total_Ca() const {
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      ret += this->multiplier(*saltAdjustment) * saltAdjustment->salt()->massConcPpm_Ca_perGramPerLiter();
   }
   return ret;
}

double RecipeAdjustmentSaltTableModel::total_Cl() const {
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      ret += this->multiplier(*saltAdjustment) * saltAdjustment->salt()->massConcPpm_Cl_perGramPerLiter();
   }
   return ret;
}

double RecipeAdjustmentSaltTableModel::total_CO3() const {
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      ret += this->multiplier(*saltAdjustment) * saltAdjustment->salt()->massConcPpm_CO3_perGramPerLiter();
   }
   return ret;
}

double RecipeAdjustmentSaltTableModel::total_HCO3() const {
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      ret += this->multiplier(*saltAdjustment) * saltAdjustment->salt()->massConcPpm_HCO3_perGramPerLiter();
   }
   return ret;
}

double RecipeAdjustmentSaltTableModel::total_Mg() const {
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      ret += this->multiplier(*saltAdjustment) * saltAdjustment->salt()->massConcPpm_Mg_perGramPerLiter();
   }
   return ret;
}

double RecipeAdjustmentSaltTableModel::total_Na() const {
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      ret += this->multiplier(*saltAdjustment) * saltAdjustment->salt()->massConcPpm_Na_perGramPerLiter();
   }
   return ret;
}

double RecipeAdjustmentSaltTableModel::total_SO4() const {
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      ret += this->multiplier(*saltAdjustment) * saltAdjustment->salt()->massConcPpm_SO4_perGramPerLiter();
   }
   return ret;
}

double RecipeAdjustmentSaltTableModel::total(Water::Ion ion) const {
   switch(ion) {
      case Water::Ion::Ca:   return total_Ca();
      case Water::Ion::Cl:   return total_Cl();
      case Water::Ion::HCO3: return total_HCO3();
      case Water::Ion::Mg:   return total_Mg();
      case Water::Ion::Na:   return total_Na();
      case Water::Ion::SO4:  return total_SO4();
      default: return 0.0;
   }
   return 0.0;
}

double RecipeAdjustmentSaltTableModel::total(Salt::Type type) const {
   // .:TBD:. Some assumptions in here that mass and volume are interchangeable... :-/
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      if (saltAdjustment->salt()->type() == type) {
         ret += this->multiplier(*saltAdjustment) * saltAdjustment->amount().quantity;
      }
   }
   return ret;
}

double RecipeAdjustmentSaltTableModel::totalAcidWeight(Salt::Type type) const {
   constexpr double H3PO4_density  = 1.685;
   constexpr double lactic_density = 1.2;

   // .:TODO:. There are assumptions in here about measurement being by weight or by volume.  We should check or assert
   //          these.
   double ret = 0.0;
   for (auto saltAdjustment : this->rows) {
      auto salt = saltAdjustment->salt();
      if (salt->type() == type) {
         double mult  = multiplier(*saltAdjustment);
         // Acid malts are easy
         if ( type == Salt::Type::AcidulatedMalt ) {
            ret += 1000.0 * saltAdjustment->amount().quantity * salt->percentAcid();
         }
         // Lactic acid isn't quite so easy
         else if ( type == Salt::Type::LacticAcid ) {
            double density = salt->percentAcid()/88.0 * (lactic_density - 1.0) + 1.0;
            double lactic_wgt = 1000.0 * saltAdjustment->amount().quantity * mult * density;
            ret += (salt->percentAcid()/100.0) * lactic_wgt;
         }
         else if ( type == Salt::Type::H3PO4 ) {
            double density = salt->percentAcid()/85.0 * (H3PO4_density - 1.0) + 1.0;
            double H3PO4_wgt = 1000.0 * saltAdjustment->amount().quantity * density;
            ret += (salt->percentAcid()/100.0) * H3PO4_wgt;
         }
      }
   }
   return ret;
}

QVariant RecipeAdjustmentSaltTableModel::data(QModelIndex const & index, int role) const {
   if (!this->isIndexOk(index)) {
      return QVariant();
   }

   auto row = this->rows[index.row()];

   auto const columnIndex = static_cast<RecipeAdjustmentSaltTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case RecipeAdjustmentSaltTableModel::ColumnIndex::Name          :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::Type          :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::Amount        :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::AmountType    :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::TotalInventory:
      case RecipeAdjustmentSaltTableModel::ColumnIndex::AddTo         :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::PctAcid       :
         return this->readDataFromModel(index, role);

      // No default case as we want the compiler to warn us if we missed one
   }
   // Should be unreachable
   return QVariant();
}

QVariant RecipeAdjustmentSaltTableModel::headerData( int section, Qt::Orientation orientation, int role ) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumnLabel(section);
   }
   return QVariant();
}

Qt::ItemFlags RecipeAdjustmentSaltTableModel::flags(const QModelIndex& index) const {
   // Q_UNUSED(index)
   if (index.row() >= this->rows.size() ) {
      return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }

   auto const row = this->rows[index.row()];
   if (!row->salt()->isAcid() && index.column() == static_cast<int>(RecipeAdjustmentSaltTableModel::ColumnIndex::PctAcid))  {
      return Qt::NoItemFlags;
   }
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool RecipeAdjustmentSaltTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->isIndexOk(index)) {
      return false;
   }

   if (role != Qt::EditRole) {
      return false;
   }

   bool retval = false;

   auto const columnIndex = static_cast<RecipeAdjustmentSaltTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case RecipeAdjustmentSaltTableModel::ColumnIndex::Name          :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::Type          :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::Amount        :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::AmountType    :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::TotalInventory:
      case RecipeAdjustmentSaltTableModel::ColumnIndex::AddTo         :
      case RecipeAdjustmentSaltTableModel::ColumnIndex::PctAcid       :
         retval = this->writeDataToModel(index, value, role);
         break;

      // No default case as we want the compiler to warn us if we missed one
   }

   if (retval) {
      emit newTotals();
   }
   emit dataChanged(index,index);
   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   headerView->resizeSections(QHeaderView::ResizeToContents);

   return retval;
}

void RecipeAdjustmentSaltTableModel::saveAndClose() {
   // all of the writes should have been instantaneous unless
   // we've added a new salt. Wonder if this will work?
   for (auto saltAddition : this->rows) {
      if (saltAddition->key() < 0) {
         ObjectStoreWrapper::insert(saltAddition);
         this->recObs->addAddition(saltAddition);
      }
   }
   return;
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(RecipeAdjustmentSalt, salt, PropertyNames::Recipe::saltAdjustments)
//=============================================== CLASS SaltItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(RecipeAdjustmentSalt)

// .:TBD:. We don't currently replicate the enabled/disabled logic from the fragment of old commented-out code below.
//         AFAICT it's graying out the SPARGE and RATIO options if the observed recipe mash has no sparge.

///   if (columnIndex == RecipeAdjustmentSaltTableModel::ColumnIndex::AddTo) {
///      QComboBox *box = new QComboBox(parent);
///
///      ...
///
///      if ( m_mash != nullptr ) {
///         QStandardItemModel* i_model = qobject_cast<QStandardItemModel*>(box->model());
///         if ( ! m_mash->hasSparge() ) {
///            for( int i = 2; i < 5; ++i ) {
///               QStandardItem* entry = i_model->item(i);
///               if ( entry )
///                  entry->setEnabled(false);
///            }
///         }
///      }
///
///      return box;
///   }
