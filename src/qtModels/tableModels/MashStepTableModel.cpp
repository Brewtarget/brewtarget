/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/MashStepTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MashStepTableModel.cpp"
#endif

COLUMN_INFOS(
   MashStepTableModel,
   TABLE_MODEL_HEADER(MashStep, Name        , PropertyNames::NamedEntity::name               ), // "Name"
   TABLE_MODEL_HEADER(MashStep, Type        , PropertyNames::MashStep::type                  ), // "Type"
   TABLE_MODEL_HEADER(MashStep, Amount      , PropertyNames::MashStep::amount_l              ), // "Amount"
   TABLE_MODEL_HEADER(MashStep, Temp        , PropertyNames::MashStep::infuseTemp_c          ), // "Infusion Temp"
   TABLE_MODEL_HEADER(MashStep, TargetTemp  , PropertyNames::StepBase::startTemp_c           ), // "Target Temp"
   TABLE_MODEL_HEADER(MashStep, EndTemp     , PropertyNames::Step::endTemp_c                 ), // "End Temp"
   TABLE_MODEL_HEADER(MashStep, RampTime    , PropertyNames::StepBase::rampTime_mins         ), // "Ramp Time"
   TABLE_MODEL_HEADER(MashStep, Time        , PropertyNames::StepBase::stepTime_mins         ), // "Time"
   TABLE_MODEL_HEADER(MashStep, StartAcidity, PropertyNames::Step::startAcidity_pH           ), // "Start pH"
   TABLE_MODEL_HEADER(MashStep,   EndAcidity, PropertyNames::Step::  endAcidity_pH           ), // "End pH"
   TABLE_MODEL_HEADER(MashStep, WaterToGrain, PropertyNames::MashStep::liquorToGristRatio_lKg), // "Water:Grain"
)

MashStepTableModel::MashStepTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<MashStepTableModel, MashStep>{},
   EnumeratedItemTableModelBase<MashStepTableModel, MashStep, Mash>{} {
   this->setObjectName("mashStepTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &MashStepTableModel::contextMenu);
   //
   // See comment in qtModels/tableModels/BoilStepTableModel.cpp for why we don't listen directly to signals from
   // ObjectStore.
   //
   return;
}

MashStepTableModel::~MashStepTableModel() = default;

void MashStepTableModel::added  ([[maybe_unused]] std::shared_ptr<MashStep> item) { return; }
void MashStepTableModel::removed([[maybe_unused]] std::shared_ptr<MashStep> item) { return; }
void MashStepTableModel::updateTotals()                                      { return; }

QVariant MashStepTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_itemOwnerObs || !this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
   if (MashStepTableModel::ColumnIndex::Temp == columnIndex) {
      auto row = this->m_rows[index.row()];
      if (row->type() == MashStep::Type::Decoction) {
         return QVariant("---");
      }
   }

   // No other special handling required for any of our other columns
   return this->readDataFromModel(index, role);
}

bool MashStepTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_itemOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(MashStep, mashStep, PropertyNames::Recipe::mashId)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
ENUMERATED_ITEM_TABLE_MODEL_COMMON_CODE(MashStep, Mash)
//=============================================== CLASS MashStepItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(MashStep)
