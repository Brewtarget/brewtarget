/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/MashStepTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "qtModels/tableModels/MashStepTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QTableView>
#include <QVariant>
#include <QWidget>

#include "model/MashStep.h"
#include "qtModels/tableModels/BtTableModel.h"

MashStepTableModel::MashStepTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(MashStep, Name      , tr("Name"         ), PropertyNames::NamedEntity::name      ),
         TABLE_MODEL_HEADER(MashStep, Type      , tr("Type"         ), PropertyNames::MashStep::type         , EnumInfo{MashStep::typeStringMapping, MashStep::typeDisplayNames}),
         TABLE_MODEL_HEADER(MashStep, Amount    , tr("Amount"       ), PropertyNames::MashStep::amount_l     ),
         TABLE_MODEL_HEADER(MashStep, Temp      , tr("Infusion Temp"), PropertyNames::MashStep::infuseTemp_c ),
         TABLE_MODEL_HEADER(MashStep, TargetTemp, tr("Target Temp"  ), PropertyNames::StepBase::startTemp_c  ),
         TABLE_MODEL_HEADER(MashStep, Time      , tr("Time"         ), PropertyNames::StepBase::stepTime_mins),
      }
   },
   TableModelBase<MashStepTableModel, MashStep>{},
   StepTableModelBase<MashStepTableModel, MashStep, Mash>{} {
   this->setObjectName("mashStepTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &MashStepTableModel::contextMenu);
   //
   // Whilst, in principle, we could connect to ObjectStoreTyped<MashStep>::getInstance() to listen for signals
   // &ObjectStoreTyped<MashStep>::signalObjectInserted and &ObjectStoreTyped<MashStep>::signalObjectDeleted, this is
   // less useful in practice because (a) we get updates about MashSteps in Mashes other than the one we are watching
   // (so we have to filter them out) and (b) when a new MashStep is created, it doesn't have a Mash, so it's not useful
   // for us to receive a signal about it until after it has been added to a Mash.  Fortunately, all we have to do is
   // connect to the Mash we are watching and listen for Mash::mashStepsChanged, which we'll get whenever a MashStep is
   // added to, or removed from, the Mash, as well as when the MashStep order changes.  We then just reread all the
   // MashSteps from the Mash which gives us simplicity for a miniscule overhead (because the number of MashSteps in a
   // Mash is never going to be enormous).
   //
   return;
}

MashStepTableModel::~MashStepTableModel() = default;

void MashStepTableModel::added  ([[maybe_unused]] std::shared_ptr<MashStep> item) { return; }
void MashStepTableModel::removed([[maybe_unused]] std::shared_ptr<MashStep> item) { return; }
void MashStepTableModel::updateTotals()                                      { return; }

QVariant MashStepTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_stepOwnerObs || !this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
   if (MashStepTableModel::ColumnIndex::Temp == columnIndex) {
      auto row = this->rows[index.row()];
      if (row->type() == MashStep::Type::Decoction) {
         return QVariant("---");
      }
   }

   // No other special handling required for any of our other columns
   return this->readDataFromModel(index, role);
}

Qt::ItemFlags MashStepTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<MashStepTableModel>(index, this->m_editable);
}

bool MashStepTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_stepOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(MashStep, mashStep, PropertyNames::Recipe::mashId)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
STEP_TABLE_MODEL_COMMON_CODE(Mash)
//=============================================== CLASS MashStepItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(MashStep)
