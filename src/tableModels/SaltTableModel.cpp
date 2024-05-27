/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/SaltTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "tableModels/SaltTableModel.h"

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

SaltTableModel::SaltTableModel(QTableView* parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         // NOTE: Need PropertyNames::Salt::amountWithUnits not PropertyNames::Salt::amount below so we
         //       can handle mass-or-volume generically in TableModelBase.
         //
         // Note too that, for the purposes of these columns, the "name" of a Salt is not its "NamedEntity name" but actually its type
         TABLE_MODEL_HEADER(Salt, Name   , tr("Name"    ), PropertyNames::Salt::type           , EnumInfo{Salt::typeStringMapping, Salt::typeDisplayNames}),
///         TABLE_MODEL_HEADER(Salt, Amount , tr("Amount"  ), PropertyNames::Salt::amountWithUnits),
///         TABLE_MODEL_HEADER(Salt, AddTo  , tr("Added To"), PropertyNames::Salt::whenToAdd      , EnumInfo{Salt::whenToAddStringMapping, Salt::whenToAddDisplayNames}),
         TABLE_MODEL_HEADER(Salt, PctAcid, tr("% Acid"  ), PropertyNames::Salt::percentAcid    ),
         TABLE_MODEL_HEADER(Salt, TotalInventory    , tr("Inventory"  ), PropertyNames::Ingredient::totalInventory, PrecisionInfo{1}),
         TABLE_MODEL_HEADER(Salt, TotalInventoryType, tr("Amount Type"), PropertyNames::Ingredient::totalInventory, Salt::validMeasures),
      }
   },
   TableModelBase<SaltTableModel, Salt>{} {
   setObjectName("saltTable");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   headerView->setMinimumSectionSize(parent->width()/this->columnCount());

   connect(headerView, &QWidget::customContextMenuRequested, this, &SaltTableModel::contextMenu);
   return;
}

SaltTableModel::~SaltTableModel() = default;

void SaltTableModel::added  ([[maybe_unused]] std::shared_ptr<Salt> item) { return; }
void SaltTableModel::removed([[maybe_unused]] std::shared_ptr<Salt> item) { return; }
void SaltTableModel::updateTotals()                                       { return; }


///void SaltTableModel::added  ([[maybe_unused]] std::shared_ptr<Salt> item) { return; }
///void SaltTableModel::removed(std::shared_ptr<Salt> item) {
///   // Dead salts do not malinger in the database. This will
///   // delete the thing, not just mark it deleted
///   if (item->key() > 0) {
///      this->recObs->remove(item);
///      ObjectStoreWrapper::hardDelete(*item);
///   }
///
///   emit newTotals();
///   return;
///}
///void SaltTableModel::updateTotals() { return; }
///
///void SaltTableModel::catchSalt() {
///   // This gets stored in the DB in saveAndClose()
///   auto gaq = std::make_shared<Salt>("");
///   this->add(gaq);
///   return;
///}
///
///double SaltTableModel::multiplier(Salt & salt) const {
///   double ret = 1.0;
///
///   if ( ! this->recObs->mash()->hasSparge() ) {
///      return ret;
///   }
///
///   if (salt.whenToAdd() == Salt::WhenToAdd::EQUAL ) {
///      ret = 2.0;
///   }
///   // If we are adding a proportional amount to both,
///   // this should handle that math.
///   else if (salt.whenToAdd() == Salt::WhenToAdd::RATIO ) {
///      double spargePct = this->recObs->mash()->totalSpargeAmount_l()/this->recObs->mash()->totalInfusionAmount_l();
///      ret = 1.0 + spargePct;
///   }
///
///   return ret;
///}
///
///// total salt in ppm. Not sure this is helping.
///double SaltTableModel::total_Ca() const {
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      double mult = this->multiplier(*salt);
///      ret += mult * salt->Ca();
///   }
///   return ret;
///}
///
///double SaltTableModel::total_Cl() const {
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      double mult  = multiplier(*salt);
///      ret += mult * salt->Cl();
///   }
///   return ret;
///}
///
///double SaltTableModel::total_CO3() const {
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      double mult  = multiplier(*salt);
///      ret += mult * salt->CO3();
///   }
///   return ret;
///}
///
///double SaltTableModel::total_HCO3() const {
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      double mult  = multiplier(*salt);
///      ret += mult * salt->HCO3();
///   }
///   return ret;
///}
///
///double SaltTableModel::total_Mg() const {
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      double mult  = multiplier(*salt);
///      ret += mult * salt->Mg();
///   }
///   return ret;
///}
///
///double SaltTableModel::total_Na() const {
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      double mult  = multiplier(*salt);
///      ret += mult * salt->Na();
///   }
///   return ret;
///}
///
///double SaltTableModel::total_SO4() const {
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      double mult  = multiplier(*salt);
///      ret += mult * salt->SO4();
///   }
///   return ret;
///}
///
///double SaltTableModel::total(Water::Ion ion) const {
///   switch(ion) {
///      case Water::Ion::Ca:   return total_Ca();
///      case Water::Ion::Cl:   return total_Cl();
///      case Water::Ion::HCO3: return total_HCO3();
///      case Water::Ion::Mg:   return total_Mg();
///      case Water::Ion::Na:   return total_Na();
///      case Water::Ion::SO4:  return total_SO4();
///      default: return 0.0;
///   }
///   return 0.0;
///}
///
///double SaltTableModel::total(Salt::Type type) const {
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      if (salt->type() == type && salt->whenToAdd() != Salt::WhenToAdd::NEVER) {
///         double mult  = multiplier(*salt);
///         ret += mult * salt->amount();
///      }
///   }
///   return ret;
///}
///
///double SaltTableModel::totalAcidWeight(Salt::Type type) const {
///   const double H3PO4_density = 1.685;
///   const double lactic_density = 1.2;
///
///   double ret = 0.0;
///   for (auto salt : this->rows) {
///      if ( salt->type() == type && salt->whenToAdd() != Salt::WhenToAdd::NEVER) {
///         double mult  = multiplier(*salt);
///         // Acid malts are easy
///         if ( type == Salt::Type::AcidulatedMalt ) {
///            ret += 1000.0 * salt->amount() * salt->percentAcid();
///         }
///         // Lactic acid isn't quite so easy
///         else if ( type == Salt::Type::LacticAcid ) {
///            double density = salt->percentAcid()/88.0 * (lactic_density - 1.0) + 1.0;
///            double lactic_wgt = 1000.0 * salt->amount() * mult * density;
///            ret += (salt->percentAcid()/100.0) * lactic_wgt;
///         }
///         else if ( type == Salt::Type::H3PO4 ) {
///            double density = salt->percentAcid()/85.0 * (H3PO4_density - 1.0) + 1.0;
///            double H3PO4_wgt = 1000.0 * salt->amount() * density;
///            ret += (salt->percentAcid()/100.0) * H3PO4_wgt;
///         }
///      }
///   }
///   return ret;
///}

QVariant SaltTableModel::data(QModelIndex const & index, int role) const {
   if (!this->isIndexOk(index)) {
      return QVariant();
   }

   auto row = this->rows[index.row()];

   auto const columnIndex = static_cast<SaltTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case SaltTableModel::ColumnIndex::Name:
///      case SaltTableModel::ColumnIndex::Amount:
///      case SaltTableModel::ColumnIndex::AddTo:
      case SaltTableModel::ColumnIndex::PctAcid:
      case SaltTableModel::ColumnIndex::TotalInventory    :
      case SaltTableModel::ColumnIndex::TotalInventoryType:
         return this->readDataFromModel(index, role);

      // No default case as we want the compiler to warn us if we missed one
   }
   // Should be unreachable
   return QVariant();
}

QVariant SaltTableModel::headerData( int section, Qt::Orientation orientation, int role ) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumnLabel(section);
   }
   return QVariant();
}

Qt::ItemFlags SaltTableModel::flags(const QModelIndex& index) const {
   // Q_UNUSED(index)
   if (index.row() >= this->rows.size() ) {
      return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }

   auto const row = this->rows[index.row()];
   if (!row->isAcid() && index.column() == static_cast<int>(SaltTableModel::ColumnIndex::PctAcid))  {
      return Qt::NoItemFlags;
   }
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool SaltTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->isIndexOk(index)) {
      return false;
   }

   if (role != Qt::EditRole) {
      return false;
   }

   bool retval = false;

///   auto row = this->rows[index.row()];

///   Measurement::PhysicalQuantity physicalQuantity =
///      row->amountIsWeight() ? Measurement::PhysicalQuantity::Mass: Measurement::PhysicalQuantity::Volume;

   auto const columnIndex = static_cast<SaltTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case SaltTableModel::ColumnIndex::Name:
///      case SaltTableModel::ColumnIndex::AddTo:
      case SaltTableModel::ColumnIndex::PctAcid:
      case SaltTableModel::ColumnIndex::TotalInventory    :
      case SaltTableModel::ColumnIndex::TotalInventoryType:
         retval = this->writeDataToModel(index, value, role);
         break;

///      case SaltTableModel::ColumnIndex::Amount:
///         retval = this->writeDataToModel(index, value, role, physicalQuantity);
///         break;

      // No default case as we want the compiler to warn us if we missed one
   }

///   if ( retval && row->whenToAdd() != Salt::WhenToAdd::NEVER ) {
///      emit newTotals();
///   }
   emit dataChanged(index,index);
   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   headerView->resizeSections(QHeaderView::ResizeToContents);

   return retval;
}

///void SaltTableModel::saveAndClose() {
///   // all of the writes should have been instantaneous unless
///   // we've added a new salt. Wonder if this will work?
///   for (auto salt : this->rows) {
///      if (salt->key() < 0 &&
///         salt->whenToAdd() != Salt::WhenToAdd::NEVER) {
///         ObjectStoreWrapper::insert(salt);
///         this->recObs->add(salt);
///      }
///   }
///   return;
///}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Salt, salt, PropertyNames::None::none)
//=============================================== CLASS SaltItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Salt)

// .:TBD:. We don't currently replicate the enabled/disabled logic from the fragment of old commented-out code below.
//         AFAICT it's graying out the SPARGE and RATIO options if the observed recipe mash has no sparge.

///   if (columnIndex == SaltTableModel::ColumnIndex::AddTo) {
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
